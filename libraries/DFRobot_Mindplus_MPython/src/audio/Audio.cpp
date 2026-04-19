#include "audio/Audio.h"

#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <cstring>
#include <memory>

#include "driver/i2s_std.h"
#include "audio/esp_codec_dev/device/include/es8388_codec.h"
#include "audio/esp_codec_dev/include/esp_codec_dev.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_defaults.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_reg.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_m4a_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_ts_dec.h"

// Align HTTP streaming behavior with Espressif esp-gmf `esp_gmf_io_http.c`:
// timeout 30s, follow redirects, accept 200/206; gzip body is handled there via
// gzip_miniz 鈥?here we request identity encoding and reject gzip if still present.
#define AUDIO_HTTP_TIMEOUT_MS   (30 * 1000)
#define AUDIO_HTTP_MAX_REDIRECT 5

#define AUDIO_DEBUG 1

#if AUDIO_DEBUG
#define AUDIO_LOG(...) Serial.printf("[Audio] " __VA_ARGS__)
#define AUDIO_LOGLN(msg) Serial.println("[Audio] " msg)
#else
#define AUDIO_LOG(...)
#define AUDIO_LOGLN(msg)
#endif

extern "C" {
int audio_codec_delete_codec_if(const audio_codec_if_t *codec_if);
int audio_codec_delete_ctrl_if(const audio_codec_ctrl_if_t *h);
int audio_codec_delete_data_if(const audio_codec_data_if_t *h);
int audio_codec_delete_gpio_if(const audio_codec_gpio_if_t *gpio_if);
void esp_codec_dev_delete(esp_codec_dev_handle_t handle);
}

namespace {
constexpr gpio_num_t SDA_PIN = GPIO_NUM_44;
constexpr gpio_num_t SCL_PIN = GPIO_NUM_43;
constexpr gpio_num_t MCLK_PIN = GPIO_NUM_39;
constexpr gpio_num_t BCLK_PIN = GPIO_NUM_41;
constexpr gpio_num_t WS_PIN = GPIO_NUM_42;
constexpr gpio_num_t DOUT_PIN = GPIO_NUM_38;
constexpr gpio_num_t DIN_PIN = GPIO_NUM_40;
constexpr size_t RECORD_BUFFER_BYTES = 2048;
constexpr size_t RECORD_RING_BUFFER_BYTES = 64 * 1024;
// Larger buffer reduces decoder starvation on network jitter.
constexpr size_t PLAY_INPUT_BUFFER_BYTES = 8192;
constexpr size_t PLAY_OUTPUT_BUFFER_BYTES = 8192;
constexpr size_t NET_RX_CACHE_BYTES = 64 * 1024;
constexpr uint32_t NET_RX_WAIT_MS = 2000;
constexpr uint32_t NET_PREFILL_BYTES = 16 * 1024;
constexpr uint32_t PLAY_STOP_WAIT_MS = 3000;

bool wr16(File &file, uint16_t value) {
  uint8_t bytes[2] = {uint8_t(value), uint8_t(value >> 8)};
  return file.write(bytes, 2) == 2;
}

bool wr32(File &file, uint32_t value) {
  uint8_t bytes[4] = {
      uint8_t(value),
      uint8_t(value >> 8),
      uint8_t(value >> 16),
      uint8_t(value >> 24),
  };
  return file.write(bytes, 4) == 4;
}

esp_audio_simple_dec_type_t getDecoderTypeFromPath(const char *path) {
  if (!path) return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;

  const char *extension = strrchr(path, '.');
  if (!extension) return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
  ++extension;

  if (!strcasecmp(extension, "aac")) return ESP_AUDIO_SIMPLE_DEC_TYPE_AAC;
  if (!strcasecmp(extension, "mp3")) return ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
  if (!strcasecmp(extension, "flac")) return ESP_AUDIO_SIMPLE_DEC_TYPE_FLAC;
  if (!strcasecmp(extension, "wav")) return ESP_AUDIO_SIMPLE_DEC_TYPE_WAV;
  if (!strcasecmp(extension, "mp4") || !strcasecmp(extension, "m4a")) {
    return ESP_AUDIO_SIMPLE_DEC_TYPE_M4A;
  }
  if (!strcasecmp(extension, "ts")) return ESP_AUDIO_SIMPLE_DEC_TYPE_TS;
  return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
}

esp_audio_simple_dec_type_t getDecoderTypeFromHttpContentType(
    const String &contentType) {
  String lowered = contentType;
  lowered.toLowerCase();

  if (lowered.indexOf("audio/mpeg") >= 0) return ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
  if (lowered.indexOf("audio/aac") >= 0 ||
      lowered.indexOf("audio/x-aac") >= 0 ||
      lowered.indexOf("audio/aacp") >= 0) {
    return ESP_AUDIO_SIMPLE_DEC_TYPE_AAC;
  }
  if (lowered.indexOf("audio/mp4") >= 0 ||
      lowered.indexOf("audio/m4a") >= 0 ||
      lowered.indexOf("audio/x-m4a") >= 0) {
    return ESP_AUDIO_SIMPLE_DEC_TYPE_M4A;
  }
  if (lowered.indexOf("audio/flac") >= 0 ||
      lowered.indexOf("audio/x-flac") >= 0) {
    return ESP_AUDIO_SIMPLE_DEC_TYPE_FLAC;
  }
  if (lowered.indexOf("audio/wav") >= 0 ||
      lowered.indexOf("audio/wave") >= 0 ||
      lowered.indexOf("audio/x-wav") >= 0) {
    return ESP_AUDIO_SIMPLE_DEC_TYPE_WAV;
  }
  if (lowered.indexOf("video/mp2t") >= 0) return ESP_AUDIO_SIMPLE_DEC_TYPE_TS;
  return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
}
}  // namespace

namespace {
struct AudioPlayContext {
  uint8_t *buf = nullptr;
  size_t cap = 0;
  volatile size_t head = 0;
  volatile size_t tail = 0;
  volatile size_t count = 0;
  volatile bool stop = false;
  volatile bool readDone = false;
  volatile bool sourceReady = false;
  volatile bool initOk = false;
  SemaphoreHandle_t lock = nullptr;
  Stream *stream = nullptr;
  File file;
  String source;
  bool useFile = false;
  esp_audio_simple_dec_type_t type = ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
  TaskHandle_t readTaskHandle = nullptr;
  TaskHandle_t decodeTaskHandle = nullptr;
};

size_t playBufferWrite(AudioPlayContext *ctx, const uint8_t *src, size_t len) {
  size_t written = 0;
  if (!ctx || !src || !len) return 0;
  if (xSemaphoreTake(ctx->lock, pdMS_TO_TICKS(10)) != pdTRUE) return 0;
  while (written < len && ctx->count < ctx->cap) {
    ctx->buf[ctx->head] = src[written++];
    ctx->head = (ctx->head + 1) % ctx->cap;
    ctx->count++;
  }
  xSemaphoreGive(ctx->lock);
  return written;
}

size_t playBufferRead(AudioPlayContext *ctx, uint8_t *dst, size_t len) {
  size_t read = 0;
  if (!ctx || !dst || !len) return 0;
  if (xSemaphoreTake(ctx->lock, pdMS_TO_TICKS(10)) != pdTRUE) return 0;
  while (read < len && ctx->count > 0) {
    dst[read++] = ctx->buf[ctx->tail];
    ctx->tail = (ctx->tail + 1) % ctx->cap;
    ctx->count--;
  }
  xSemaphoreGive(ctx->lock);
  return read;
}

void networkReadTask(void *arg) {
  AudioPlayContext *ctx = reinterpret_cast<AudioPlayContext *>(arg);
  HTTPClient http;
  WiFiClientSecure *secureClient = nullptr;
  WiFiClient *plainClient = nullptr;
  uint8_t *tmp = (uint8_t *)malloc(1460);

  if (!tmp) {
    AUDIO_LOGLN("PlayUrl failed: read task malloc failed");
    goto done;
  }
  if (ctx && ctx->source.length()) {
    AUDIO_LOG("PlayUrl(url=%s)\n", ctx->source.c_str());

    bool isHttps = !strncmp(ctx->source.c_str(), "https://", 8);
    bool beginOk = false;
    if (isHttps) {
      secureClient = new WiFiClientSecure();
      secureClient->setInsecure();
      secureClient->setHandshakeTimeout(30);
      beginOk = http.begin(*secureClient, ctx->source);
    } else {
      plainClient = new WiFiClient();
      beginOk = http.begin(*plainClient, ctx->source);
    }

    if (!beginOk) {
      AUDIO_LOGLN("PlayUrl failed: http begin failed");
    } else {
      http.setTimeout(AUDIO_HTTP_TIMEOUT_MS);
      http.setConnectTimeout(AUDIO_HTTP_TIMEOUT_MS);
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      http.setRedirectLimit(AUDIO_HTTP_MAX_REDIRECT);
      const char *responseHeaders[] = {"Content-Type", "Content-Encoding"};
      http.collectHeaders(responseHeaders, 2);
      http.addHeader("Accept", "audio/*,*/*;q=0.8");
      http.addHeader("Accept-Encoding", "identity");

      int statusCode = http.GET();
      AUDIO_LOG("PlayUrl: http.GET() returned %d\n", statusCode);
      if (statusCode != HTTP_CODE_OK && statusCode != HTTP_CODE_PARTIAL_CONTENT) {
        AUDIO_LOG("PlayUrl failed: http status=%d (%s)\n",
                  statusCode,
                  HTTPClient::errorToString(statusCode).c_str());
      } else {
        String contentEncoding = http.header("Content-Encoding");
        contentEncoding.toLowerCase();
        if (contentEncoding.indexOf("gzip") >= 0 || contentEncoding.indexOf("deflate") >= 0) {
          AUDIO_LOG(
              "PlayUrl failed: compressed Content-Encoding=%s (esp-gmf uses gzip "
              "decompress; use a URL that serves raw audio or add decompression)\n",
              contentEncoding.c_str());
        } else {
          String urlPath = ctx->source;
          int queryPos = urlPath.indexOf('?');
          if (queryPos >= 0) urlPath.remove(queryPos);
          int fragmentPos = urlPath.indexOf('#');
          if (fragmentPos >= 0) urlPath.remove(fragmentPos);
          ctx->type = getDecoderTypeFromPath(urlPath.c_str());
          if (ctx->type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) {
            ctx->type = getDecoderTypeFromHttpContentType(http.header("Content-Type"));
          }

          ctx->stream = http.getStreamPtr();
          if (!ctx->stream || ctx->type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) {
            AUDIO_LOGLN("PlayUrl failed: context init failed");
          } else {
            ctx->initOk = true;
            ctx->sourceReady = true;
            while (!ctx->stop) {
              auto *streamClient = static_cast<NetworkClient *>(ctx->stream);
              if (!streamClient->connected()) break;
              int avail = streamClient->available();
              if (avail <= 0) {
                vTaskDelay(pdMS_TO_TICKS(2));
                continue;
              }
              int toRead = avail;
              if (toRead > 1460) toRead = 1460;
              int n = streamClient->read(tmp, toRead);
              if (n <= 0) {
                vTaskDelay(pdMS_TO_TICKS(2));
                continue;
              }
              size_t off = 0;
              while (!ctx->stop && off < (size_t)n) {
                size_t w = playBufferWrite(ctx, tmp + off, (size_t)n - off);
                off += w;
                if (off < (size_t)n) vTaskDelay(pdMS_TO_TICKS(2));
              }
            }
          }
        }
      }
    }
  }

done:
  free(tmp);
  if (ctx && !ctx->sourceReady) {
    ctx->initOk = false;
    ctx->sourceReady = true;
  }
  if (ctx) {
    ctx->readDone = true;
    ctx->readTaskHandle = nullptr;
  }
  http.end();
  delete secureClient;
  delete plainClient;
  vTaskDelete(nullptr);
}

void localReadTask(void *arg) {
  AudioPlayContext *ctx = reinterpret_cast<AudioPlayContext *>(arg);
  uint8_t *tmp = (uint8_t *)malloc(PLAY_INPUT_BUFFER_BYTES);

  if (!tmp) {
    AUDIO_LOGLN("PlayLocal failed: read task malloc failed");
    goto done;
  }
  if (!ctx || !ctx->source.length()) goto done;

  AUDIO_LOG("PlayLocal(path=%s)\n", ctx->source.c_str());
  ctx->type = getDecoderTypeFromPath(ctx->source.c_str());
  ctx->file = LittleFS.open(ctx->source.c_str(), FILE_READ);
  if (ctx->type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE || !ctx->file) {
    AUDIO_LOGLN("PlayLocal failed: context init failed");
    goto done;
  }

  AUDIO_LOG("PlayLocal size=%lu type=%lu\n", (unsigned long)ctx->file.size(),
            (unsigned long)ctx->type);
  ctx->initOk = true;
  ctx->sourceReady = true;

  while (!ctx->stop) {
    size_t n = ctx->file.read(tmp, PLAY_INPUT_BUFFER_BYTES);
    if (!n) break;
    size_t off = 0;
    while (!ctx->stop && off < n) {
      size_t w = playBufferWrite(ctx, tmp + off, n - off);
      off += w;
      if (off < n) vTaskDelay(pdMS_TO_TICKS(2));
    }
  }

done:
  free(tmp);
  if (ctx && !ctx->sourceReady) {
    ctx->initOk = false;
    ctx->sourceReady = true;
  }
  if (ctx && ctx->file) ctx->file.close();
  if (ctx) {
    ctx->readDone = true;
    ctx->readTaskHandle = nullptr;
  }
  vTaskDelete(nullptr);
}

}  // namespace

Audio::Audio()
    : _gpioIf(nullptr),
      _ctrlIf(nullptr),
      _playDataIf(nullptr),
      _recDataIf(nullptr),
      _codecIf(nullptr),
      _playDev(nullptr),
      _recDev(nullptr),
      _begun(false),
      _recording(false),
      _playState(PLAY_STATE_IDLE),
      _playStopRequested(false),
      _playPauseRequested(false),
      _playReadTaskHandle(nullptr),
      _playDecodeTaskHandle(nullptr),
      _playContext(nullptr),
      _playSource(),
      _playSourceIsUrl(false),
      _recordStopRequested(false),
      _recordCaptureDone(true),
      _recordWriterDone(true),
      _recordCaptureTaskHandle(nullptr),
      _recordWriterTaskHandle(nullptr),
      _recordRingbuf(nullptr),
      _volume(DEFAULT_VOLUME),
      _micGain(35.0f),
      _recordFs(nullptr),
      _recordDataBytes(0),
      _recordSampleRate(16000),
      _recordBitsPerSample(16),
      _recordChannels(1) {}

Audio::~Audio() { end(); }

bool Audio::mountFS(fs::FS *fs, const char *label) {
  return fs && (fs != &LittleFS || LittleFS.begin(true, "/littlefs", 5, label));
}

bool Audio::initI2S() {
  i2s_chan_config_t channelConfig =
      I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)I2S_PORT, I2S_ROLE_MASTER);
  channelConfig.auto_clear = true;

  i2s_chan_handle_t tx = nullptr;
  i2s_chan_handle_t rx = nullptr;
  if (i2s_new_channel(&channelConfig, &tx, &rx) != ESP_OK) return false;

  i2s_std_config_t stdConfig = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
          I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
      .gpio_cfg = {
          .mclk = MCLK_PIN,
          .bclk = BCLK_PIN,
          .ws = WS_PIN,
          .dout = DOUT_PIN,
          .din = DIN_PIN,
          .invert_flags = {false, false, false},
      },
  };

  if (i2s_channel_init_std_mode(tx, &stdConfig) != ESP_OK ||
      i2s_channel_init_std_mode(rx, &stdConfig) != ESP_OK) {
    i2s_del_channel(tx);
    i2s_del_channel(rx);
    return false;
  }

  // 播放用 data_if：只含 TX handle
  // 官方参考（audio_codec_data_i2s.c）：get_paired() 通过 i2s_data_list 查找同 port
  // 的配对 data_if，TX 和 RX 各自持有对方 handle 可使框架正确协调时钟。
  audio_codec_i2s_cfg_t playDataConfig = {
      .port = I2S_PORT,
      .rx_handle = nullptr,
      .tx_handle = tx,
      .clk_src = I2S_CLK_SRC_DEFAULT,
  };
  _playDataIf = audio_codec_new_i2s_data(&playDataConfig);
  if (!_playDataIf) {
    i2s_del_channel(tx);
    i2s_del_channel(rx);
    return false;
  }

  // 录音用 data_if：只含 RX handle，不携带 TX handle。
  // get_paired() 通过相同 port 将 _playDataIf（有 out_handle=TX）和 _recDataIf（有 in_handle=RX）配对。
  // 当 RX 需要重配时钟时，框架会通过 get_paired() 找到 _playDataIf 并操作其 TX channel，
  // 不需要在 _recDataIf 中重复注册同一个 TX handle（否则同一 TX handle 被两个条目引用，
  // 导致 _i2s_drv_enable 对同一 channel 双重操作，产生噪音）。
  audio_codec_i2s_cfg_t recDataConfig = {
      .port = I2S_PORT,
      .rx_handle = rx,
      .tx_handle = nullptr,
      .clk_src = I2S_CLK_SRC_DEFAULT,
  };
  _recDataIf = audio_codec_new_i2s_data(&recDataConfig);
  if (!_recDataIf) {
    audio_codec_delete_data_if(_playDataIf);
    _playDataIf = nullptr;
    i2s_del_channel(rx);
    return false;
  }
  return true;
}

bool Audio::initCodec() {
  audio_codec_i2c_cfg_t i2cConfig = {
      .port = I2C_PORT,
      .addr = ES8388_CODEC_DEFAULT_ADDR,
      .bus_handle = nullptr,
  };
  _ctrlIf = audio_codec_new_i2c_ctrl(&i2cConfig);
  if (!_ctrlIf) return false;

  _gpioIf = audio_codec_new_gpio();
  if (!_gpioIf) return false;

  es8388_codec_cfg_t codecConfig = {
      .ctrl_if = _ctrlIf,
      .gpio_if = _gpioIf,
      .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
      .master_mode = false,
      .pa_pin = -1,
      .pa_reverted = false,
      .hw_gain = {},
  };
  _codecIf = es8388_codec_new(&codecConfig);
  if (!_codecIf) return false;

  esp_codec_dev_cfg_t playDevConfig = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = _codecIf,
      .data_if = _playDataIf,
  };
  _playDev = esp_codec_dev_new(&playDevConfig);
  if (!_playDev) return false;

  esp_codec_dev_cfg_t recDevConfig = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = _codecIf,
      .data_if = _recDataIf,
  };
  _recDev = esp_codec_dev_new(&recDevConfig);
  return _recDev != nullptr;
}

void Audio::deinitCodec() {
  closePlayCodec();
  closeRecCodec();

  if (_playDev) {
    esp_codec_dev_delete(_playDev);
    _playDev = nullptr;
  }
  if (_recDev) {
    esp_codec_dev_delete(_recDev);
    _recDev = nullptr;
  }
  if (_codecIf) {
    audio_codec_delete_codec_if(_codecIf);
    _codecIf = nullptr;
  }
  if (_ctrlIf) {
    audio_codec_delete_ctrl_if(_ctrlIf);
    _ctrlIf = nullptr;
  }
  if (_recDataIf) {
    audio_codec_delete_data_if(_recDataIf);
    _recDataIf = nullptr;
  }
  if (_playDataIf) {
    audio_codec_delete_data_if(_playDataIf);
    _playDataIf = nullptr;
  }
  if (_gpioIf) {
    audio_codec_delete_gpio_if(_gpioIf);
    _gpioIf = nullptr;
  }
}

bool Audio::begin() {
  if (_begun) return true;

  AUDIO_LOGLN("begin()");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  if (!initI2S() || !initCodec()) {
    AUDIO_LOGLN("begin failed during init");
    deinitCodec();
    return false;
  }

  _begun = true;
  setVolume(_volume);
  setMicGain(_micGain);

  return true;
}

void Audio::end() {
  stop();
  stopRecord();
  deinitCodec();
  _begun = false;
}

bool Audio::openPlayCodec(uint32_t sr, uint8_t bits, uint8_t ch) {
  if (!_playDev) {
    AUDIO_LOGLN("OpenPlayCodec failed: _playDev is null");
    return false;
  }

  int mclkMultiple = 256;

  esp_codec_dev_sample_info_t sampleInfo = {
      .bits_per_sample = bits,
      .channel = ch,
      .channel_mask = 0,
      .sample_rate = sr,
      .mclk_multiple = mclkMultiple,
  };

  AUDIO_LOG("OpenPlayCodec(sr=%lu, bits=%u, ch=%u) mclk=%d\n",
      (unsigned long)sr, bits, ch, sampleInfo.mclk_multiple);

  int status = esp_codec_dev_open(_playDev, &sampleInfo);
  if (status != ESP_CODEC_DEV_OK) {
    AUDIO_LOG("OpenPlayCodec failed: status=%d\n", status);
    return false;
  }
  esp_codec_dev_set_out_vol(_playDev, _volume);
  return true;
}

void Audio::closePlayCodec() {
  if (_playDev) esp_codec_dev_close(_playDev);
}

bool Audio::openRecCodec(uint32_t sr, uint8_t bits, uint8_t ch) {
  if (!_recDev) {
    AUDIO_LOGLN("openRecCodec failed: _recDev is null");
    return false;
  }

  int mclkMultiple = 256;
  if (sr == 11025 || sr == 22050 || sr == 44100) {
    mclkMultiple = 384;
  }

  esp_codec_dev_sample_info_t sampleInfo = {
      .bits_per_sample = bits,
      .channel = ch,
      .channel_mask = 0,
      .sample_rate = sr,
      .mclk_multiple = mclkMultiple,
  };

  AUDIO_LOG("openRecCodec(sr=%lu, bits=%u, ch=%u) mclk=%d\n",
      (unsigned long)sr, bits, ch, sampleInfo.mclk_multiple);

  int status = esp_codec_dev_open(_recDev, &sampleInfo);
  if (status != ESP_CODEC_DEV_OK) {
    AUDIO_LOG("openRecCodec failed: status=%d\n", status);
    return false;
  }
  esp_codec_dev_set_in_gain(_recDev, _micGain);
  return true;
}

void Audio::closeRecCodec() {
  if (_recDev) esp_codec_dev_close(_recDev);
}

bool Audio::setVolume(uint8_t volume) {
  _volume = volume > 100 ? 100 : volume;
  return !_playDev ||
         esp_codec_dev_set_out_vol(_playDev, _volume) == ESP_CODEC_DEV_OK;
}

bool Audio::setMicGain(float gain) {
  _micGain = gain;
  return !_recDev ||
         esp_codec_dev_set_in_gain(_recDev, _micGain) == ESP_CODEC_DEV_OK;
}

bool Audio::initRecordBuffer(size_t bufferSize) {
  deinitRecordBuffer();
  _recordRingbuf = xRingbufferCreate(bufferSize, RINGBUF_TYPE_BYTEBUF);
  return _recordRingbuf != nullptr;
}

void Audio::deinitRecordBuffer() {
  if (_recordRingbuf) {
    vRingbufferDelete(_recordRingbuf);
    _recordRingbuf = nullptr;
  }
}

size_t Audio::recordBufferWrite(const uint8_t *src, size_t len) {
  if (!_recordRingbuf || !src || !len) return 0;
  if (xRingbufferSend(_recordRingbuf, src, len, 0) == pdTRUE) return len;
  return 0;
}

size_t Audio::recordBufferRead(uint8_t *dst, size_t len) {
  if (!_recordRingbuf || !dst || !len) return 0;
  size_t received = 0;
  void *item = xRingbufferReceiveUpTo(_recordRingbuf, &received, 0, len);
  if (!item) return 0;
  memcpy(dst, item, received);
  vRingbufferReturnItem(_recordRingbuf, item);
  return received;
}

bool Audio::writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate,
                           uint8_t bitsPerSample, uint8_t channels) {
  if (!file.seek(0)) return false;

  uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
  uint16_t blockAlign = channels * (bitsPerSample / 8);

  file.write((const uint8_t *)"RIFF", 4);
  if (!wr32(file, 36 + dataSize)) return false;

  file.write((const uint8_t *)"WAVEfmt ", 8);
  if (!wr32(file, 16) || !wr16(file, 1) || !wr16(file, channels) ||
      !wr32(file, sampleRate) || !wr32(file, byteRate) ||
      !wr16(file, blockAlign) || !wr16(file, bitsPerSample)) {
    return false;
  }

  file.write((const uint8_t *)"data", 4);
  return wr32(file, dataSize);
}

bool Audio::finalizeWavHeader() {
  return _recordFile &&
         writeWavHeader(_recordFile, _recordDataBytes, _recordSampleRate,
                        _recordBitsPerSample, _recordChannels);
}

String Audio::normalizeFsPath(const char *path) const {
  if (!path || !path[0]) return String();
  String normalized(path);
  normalized.trim();
  if (!normalized.length()) return String();
  if (isHttpSource(normalized.c_str())) return normalized;
  if (!normalized.startsWith("/")) normalized = "/" + normalized;
  return normalized;
}

bool Audio::isHttpSource(const char *path) const {
  return path && (!strncmp(path, "http://", 7) || !strncmp(path, "https://", 8));
}

void Audio::setPlayState(PlayState state) {
  _playState = state;
}

bool Audio::waitWhilePaused() {
  while (_playPauseRequested && !_playStopRequested) {
    setPlayState(PLAY_STATE_PAUSED);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  if (_playStopRequested) return false;
  setPlayState(PLAY_STATE_PLAYING);
  return true;
}

bool Audio::startRecord(const char *path, uint32_t sr, uint8_t bits, uint8_t ch) {
  String normalizedPath = normalizeFsPath(path);
  AUDIO_LOG("startRecord(path=%s, sr=%lu, bits=%u, ch=%u)\n",
            normalizedPath.c_str(),
            (unsigned long)sr,
            bits,
            ch);

  fs::FS *fs = &LittleFS;
  const char *label = "lfs2";
  if ((!_begun && !begin()) || !mountFS(fs, label)) {
    AUDIO_LOGLN("startRecord failed: begin or mountFS failed");
    return false;
  }

  if (!normalizedPath.length() || isHttpSource(normalizedPath.c_str())) {
    AUDIO_LOGLN("startRecord failed: invalid record path");
    return false;
  }

  stopRecord();
  if (!openRecCodec(sr, bits, ch)) {
    AUDIO_LOGLN("startRecord failed: openRecCodec failed");
    return false;
  }

  _recordFile = fs->open(normalizedPath.c_str(), FILE_WRITE, true);
  if (!_recordFile) {
    AUDIO_LOGLN("startRecord failed: open file failed");
    closeRecCodec();
    return false;
  }

  if (!initRecordBuffer(RECORD_RING_BUFFER_BYTES)) {
    AUDIO_LOGLN("startRecord failed: init record buffer failed");
    _recordFile.close();
    closeRecCodec();
    return false;
  }

  _recordFs = fs;
  _recordSampleRate = sr;
  _recordBitsPerSample = bits;
  _recordChannels = ch;
  _recordDataBytes = 0;
  if (!writeWavHeader(_recordFile, 0, sr, bits, ch)) {
    AUDIO_LOGLN("startRecord failed: write header failed");
    _recordFile.close();
    deinitRecordBuffer();
    closeRecCodec();
    return false;
  }

  _recordStopRequested = false;
  _recordCaptureDone = false;
  _recordWriterDone = false;
  if (xTaskCreatePinnedToCore([](void *a){ static_cast<Audio*>(a)->recordCaptureTask(); vTaskDelete(nullptr); },
                              "audio_rec_cap",
                              4096,
                              this,
                              3,
                              &_recordCaptureTaskHandle,
                              0) != pdPASS) {
    AUDIO_LOGLN("startRecord failed: create capture task failed");
    _recordCaptureTaskHandle = nullptr;
    _recordCaptureDone = true;
    _recordWriterDone = true;
    _recordFile.close();
    deinitRecordBuffer();
    closeRecCodec();
    return false;
  }
  if (xTaskCreatePinnedToCore([](void *a){ static_cast<Audio*>(a)->recordWriterTask(); vTaskDelete(nullptr); },
                              "audio_rec_wr",
                              4096,
                              this,
                              2,
                              &_recordWriterTaskHandle,
                              1) != pdPASS) {
    AUDIO_LOGLN("startRecord failed: create writer task failed");
    _recordStopRequested = true;
    _recordWriterTaskHandle = nullptr;
    _recordCaptureDone = true;
    _recordWriterDone = true;
    _recordFile.close();
    deinitRecordBuffer();
    closeRecCodec();
    return false;
  }

  _recording = true;
  AUDIO_LOG("startRecord header write=1, ringBuffer=%u\n",
            (unsigned)RECORD_RING_BUFFER_BYTES);
  return true;
}

bool Audio::startRecord(const char *path, uint32_t sampleRate,
                        uint8_t bitsPerSample, uint8_t channels,
                        uint32_t durationMs) {
  AUDIO_LOG(
      "startRecord(path=%s, sr=%lu, bits=%u, ch=%u, durationMs=%lu)\n",
      path ? path : "<null>",
      (unsigned long)sampleRate,
      bitsPerSample,
      channels,
      (unsigned long)durationMs);

  if (!startRecord(path, sampleRate, bitsPerSample, channels)) {
    return false;
  }

  delay(durationMs);
  stopRecord();
  return true;
}

void Audio::recordCaptureTask() {
  uint8_t *buffer = (uint8_t *)malloc(RECORD_BUFFER_BYTES);
  if (!buffer) {
    _recording = false;
    _recordStopRequested = true;
    _recordCaptureDone = true;
    _recordCaptureTaskHandle = nullptr;
    return;
  }

  while (!_recordStopRequested) {
    int status = esp_codec_dev_read(_recDev, buffer, (int)RECORD_BUFFER_BYTES);
    if (status != ESP_CODEC_DEV_OK) {
      AUDIO_LOG("recordCaptureTask read failed: status=%d req=%u\n",
                status,
                (unsigned)RECORD_BUFFER_BYTES);
      _recording = false;
      _recordStopRequested = true;
      break;
    }

    size_t offset = 0;
    while (!_recordStopRequested && offset < RECORD_BUFFER_BYTES) {
      size_t written = recordBufferWrite(buffer + offset,
                                        RECORD_BUFFER_BYTES - offset);
      if (!written) {
        vTaskDelay(pdMS_TO_TICKS(2));
        continue;
      }
      offset += written;
    }
  }

  free(buffer);
  _recordCaptureDone = true;
  _recordCaptureTaskHandle = nullptr;
}

void Audio::recordWriterTask() {
  uint8_t *buffer = (uint8_t *)malloc(RECORD_BUFFER_BYTES);
  if (!buffer) {
    _recording = false;
    _recordWriterDone = true;
    _recordWriterTaskHandle = nullptr;
    return;
  }

  while (!_recordStopRequested || !_recordCaptureDone ||
         (_recordRingbuf && xRingbufferGetCurFreeSize(_recordRingbuf) < RECORD_RING_BUFFER_BYTES)) {
    size_t bytesRead = recordBufferRead(buffer, RECORD_BUFFER_BYTES);
    if (!bytesRead) {
      vTaskDelay(pdMS_TO_TICKS(2));
      continue;
    }

    size_t written = _recordFile.write(buffer, bytesRead);
    if (written != bytesRead) {
      AUDIO_LOG("recordWriterTask write failed: want=%u actual=%u\n",
                (unsigned)bytesRead,
                (unsigned)written);
      _recording = false;
      _recordStopRequested = true;
      break;
    }
    _recordDataBytes += (uint32_t)written;
  }

  free(buffer);
  _recordWriterDone = true;
  _recordWriterTaskHandle = nullptr;
}

void Audio::stopRecord() {
  if (_recording) {
    AUDIO_LOG("stopRecord dataBytes=%lu\n", (unsigned long)_recordDataBytes);
    _recordStopRequested = true;
    uint32_t startMs = millis();
    while ((!_recordCaptureDone || !_recordWriterDone) &&
           millis() - startMs < PLAY_STOP_WAIT_MS) {
      delay(2);
    }
    finalizeWavHeader();
    _recordFile.flush();
    _recordFile.close();
    deinitRecordBuffer();
    closeRecCodec();
    _recording = false;
  }
}

bool Audio::recording() const { return _recording; }

void Audio::configureSimpleDecoder(esp_audio_simple_dec_cfg_t &cfg,
                                   uint8_t *storage,
                                   size_t storageSize) const {
  if (!storage || storageSize == 0) return;

  memset(storage, 0, storageSize);
  switch (cfg.dec_type) {
    case ESP_AUDIO_SIMPLE_DEC_TYPE_AAC: {
      auto *aacCfg = reinterpret_cast<esp_aac_dec_cfg_t *>(storage);
      aacCfg->aac_plus_enable = true;
      cfg.dec_cfg = aacCfg;
      cfg.cfg_size = sizeof(esp_aac_dec_cfg_t);
      break;
    }
    case ESP_AUDIO_SIMPLE_DEC_TYPE_M4A: {
      auto *m4aCfg = reinterpret_cast<esp_m4a_dec_cfg_t *>(storage);
      m4aCfg->aac_plus_enable = true;
      cfg.dec_cfg = m4aCfg;
      cfg.cfg_size = sizeof(esp_m4a_dec_cfg_t);
      break;
    }
    case ESP_AUDIO_SIMPLE_DEC_TYPE_TS: {
      auto *tsCfg = reinterpret_cast<esp_ts_dec_cfg_t *>(storage);
      tsCfg->aac_plus_enable = true;
      cfg.dec_cfg = tsCfg;
      cfg.cfg_size = sizeof(esp_ts_dec_cfg_t);
      break;
    }
    default:
      break;
  }
}

// 解码任务：从环形缓冲区读取压缩数据，解码后写入 codec DAC 输出。
// 与 localReadTask / networkReadTask 并行运行，通过 AudioPlayContext
// 中的环形缓冲区（生产者-消费者）解耦 IO 与解码。
void Audio::playDecodeTask() {
  // --- 阶段1：等待读取任务完成数据源初始化 ---
  AudioPlayContext *ctx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (!ctx) {
    _playDecodeTaskHandle = nullptr;
    setPlayState(PLAY_STATE_ERROR);
    return;
  }

  // 读取任务异步初始化（打开文件/建立 HTTP 连接），等待其完成或失败
  while (!ctx->sourceReady && !ctx->stop && !_playStopRequested) {
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  if (!ctx->initOk || ctx->type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) {
    _playDecodeTaskHandle = nullptr;
    setPlayState(PLAY_STATE_ERROR);
    return;
  }

  // --- 阶段2：注册并创建 simple decoder ---
  // 解码器注册表是全局的，但在任务上下文中注册/注销更安全，避免 begin()/end() 的生命周期问题
  esp_audio_dec_register_default();
  esp_audio_simple_dec_register_default();

  // decoderConfigStorage 作为 union 对齐存储，覆盖最大的子类型配置（esp_m4a_dec_cfg_t）
  uint8_t decoderConfigStorage[sizeof(esp_m4a_dec_cfg_t)] = {};
  esp_audio_simple_dec_cfg_t decoderConfig = {
      .dec_type = ctx->type,
      .dec_cfg = nullptr,
      .cfg_size = 0,
  };
  // 根据格式填充特定配置（如 AAC/M4A/TS 启用 HE-AAC/aacPlus）
  configureSimpleDecoder(decoderConfig, decoderConfigStorage,
                         sizeof(decoderConfigStorage));

  esp_audio_simple_dec_handle_t decoder = nullptr;
  if (esp_audio_simple_dec_open(&decoderConfig, &decoder) != ESP_AUDIO_ERR_OK) {
    AUDIO_LOGLN("PlayDecodeTask failed: decoder open failed");
    _playStopRequested = true;
    _playDecodeTaskHandle = nullptr;
    return;
  }

  // --- 阶段3：分配 IO 缓冲区 ---
  uint8_t *inputBuffer = (uint8_t *)malloc(PLAY_INPUT_BUFFER_BYTES);   // 从环形缓冲区读入的压缩数据
  size_t outputBufferSize = PLAY_OUTPUT_BUFFER_BYTES;
  uint8_t *outputBuffer = (uint8_t *)malloc(outputBufferSize);          // 解码器输出的 PCM 数据
  if (!inputBuffer || !outputBuffer) {
    AUDIO_LOGLN("PlayDecodeTask failed: malloc failed");
    free(inputBuffer);
    free(outputBuffer);
    esp_audio_simple_dec_close(decoder);
    _playStopRequested = true;
    _playDecodeTaskHandle = nullptr;
    return;
  }

  bool ok = true;
  bool codecOpened = false;     // codec（I2S+ES8388）是否已按音频参数打开
  bool firstWriteLogged = false;
  esp_audio_simple_dec_info_t simpleInfo = {};
  setPlayState(PLAY_STATE_PLAYING);

  // --- 阶段4：主解码循环 ---
  while (!_playStopRequested) {
    // 支持暂停：阻塞直到 resume() 或 stop()
    if (!waitWhilePaused()) break;

    // 从环形缓冲区读取一批压缩数据；缓冲区空时短暂等待读取任务补充
    size_t bytesRead = playBufferRead(ctx, inputBuffer, PLAY_INPUT_BUFFER_BYTES);
    if (!bytesRead) {
      if (ctx->readDone && ctx->count == 0) break;  // 数据源已耗尽，正常结束
      delay(2);
      continue;
    }

    // raw 描述本批待解码的压缩数据；decoder 每次调用消费其中若干字节（raw.consumed）
    esp_audio_simple_dec_raw_t raw = {
        .buffer = inputBuffer,
        .len = (uint32_t)bytesRead,
        .eos = ctx->readDone && ctx->count == 0,  // 文件/流末尾标志
        .consumed = 0,
        .frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE,
    };

    // 内循环：一批输入数据可能包含多个压缩帧，循环直到全部消费完
    while (raw.len && !_playStopRequested) {
      esp_audio_simple_dec_out_t frame = {
          .buffer = outputBuffer,
          .len = (uint32_t)outputBufferSize,
          .needed_size = 0,
          .decoded_size = 0,
      };

      esp_audio_err_t err = esp_audio_simple_dec_process(decoder, &raw, &frame);

      // 输出缓冲区不足：按解码器要求扩容后重试（不移动 raw 指针，重新解码同一帧）
      if (err == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
        uint8_t *resizedOutput = (uint8_t *)realloc(outputBuffer, frame.needed_size);
        if (!resizedOutput) {
          AUDIO_LOGLN("PlayDecodeTask failed: realloc failed");
          ok = false;
          _playStopRequested = true;
          break;
        }
        outputBuffer = resizedOutput;
        outputBufferSize = frame.needed_size;
        continue;  // 用新缓冲区重试，raw 不变
      }
      if (err != ESP_AUDIO_ERR_OK) {
        AUDIO_LOG("PlayDecodeTask decode failed: err=%d raw.len=%lu consumed=%lu\n",
                  (int)err,
                  (unsigned long)raw.len,
                  (unsigned long)raw.consumed);
        ok = false;
        _playStopRequested = true;
        break;
      }

      // 推进输入指针，跳过已消费字节
      raw.buffer += raw.consumed;
      raw.len -= raw.consumed;

      if (frame.decoded_size > 0) {
        // --- 首帧：读取音频参数并打开 codec ---
        // simple decoder 在解码出第一帧后才能提供准确的 sample_rate/bits/channel，
        // 因此 codec（I2S 时钟、ES8388 采样率）延迟到此处才配置。
        if (!codecOpened) {
          if (esp_audio_simple_dec_get_info(decoder, &simpleInfo) != ESP_AUDIO_ERR_OK) {
            // 极少数情况下首帧信息尚未就绪，跳过本帧等待下一帧
            AUDIO_LOGLN("PlayDecodeTask waiting decoder info");
            continue;
          }
          uint8_t bitsPerSample = simpleInfo.bits_per_sample;
          uint8_t channels = simpleInfo.channel ? simpleInfo.channel : 2;
          // 防御：极少数解码器实现中 bits_per_sample 与 channel 字段顺序错误，
          // 通过不合理值组合（bits=1/2 且 channel=8/16/24/32）检测并互换
          if ((bitsPerSample == 1 || bitsPerSample == 2) &&
              (channels == 8 || channels == 16 || channels == 24 || channels == 32)) {
            uint8_t swapped = bitsPerSample;
            bitsPerSample = channels;
            channels = swapped;
          }
          AUDIO_LOG("PlayDecodeTask decoded info: sr=%lu bits=%u ch=%u\n",
                    (unsigned long)simpleInfo.sample_rate,
                    bitsPerSample,
                    channels);
          // 按实际音频参数配置 I2S 时钟和 ES8388 DAC
          // channel=1 时 esp_codec_dev 内部自动设置 I2S slot mask，无需手动扩展为立体声
          if (!openPlayCodec(simpleInfo.sample_rate, bitsPerSample, channels)) {
            AUDIO_LOGLN("PlayDecodeTask OpenPlayCodec failed");
            ok = false;
            _playStopRequested = true;
            break;
          }
          simpleInfo.bits_per_sample = bitsPerSample;
          simpleInfo.channel = channels;
          codecOpened = true;

          // I2S PLL 重新锁定需要几个周期，等待时钟稳定后再写入 PCM，
          // 避免 TX DMA 在 clock 不稳期间输出噪声。
          // 丢弃这一帧（解码已完成，仅跳过写入），下一帧时钟已稳定。
          AUDIO_LOGLN("PlayDecodeTask codec opened, dropping first frame to let I2S PLL settle");
          vTaskDelay(pdMS_TO_TICKS(10));
          continue;
        }

        if (!firstWriteLogged) {
          AUDIO_LOG("PlayDecodeTask first write: decoded_size=%lu ch=%u codec=%p\n",
                    (unsigned long)frame.decoded_size,
                    simpleInfo.channel,
                    _playDev);
          firstWriteLogged = true;
        }

        // 将解码后的 PCM 写入 I2S DMA，阻塞直到 DMA 接受数据（天然背压）
        if (esp_codec_dev_write(_playDev, frame.buffer, (int)frame.decoded_size) != ESP_CODEC_DEV_OK) {
          AUDIO_LOG("PlayDecodeTask write failed: decoded_size=%lu\n",
                    (unsigned long)frame.decoded_size);
          ok = false;
          _playStopRequested = true;
          break;
        }
      }
    }
  }

  // --- 阶段5：清理资源 ---
  free(inputBuffer);
  free(outputBuffer);
  esp_audio_simple_dec_close(decoder);
  closePlayCodec();  // 关闭 I2S + ES8388

  // 注销解码器（与阶段2对应）
  esp_audio_simple_dec_unregister_default();
  esp_audio_dec_unregister_default();

  // 释放播放上下文（环形缓冲区、互斥锁）
  // 检查指针防止 stop() 已抢先释放
  AudioPlayContext *doneCtx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (doneCtx == ctx) {
    _playContext = nullptr;
    if (doneCtx->lock) vSemaphoreDelete(doneCtx->lock);
    free(doneCtx->buf);
    delete doneCtx;
  }
  // 根据退出原因更新播放状态
  if (_playStopRequested) setPlayState(PLAY_STATE_STOPPED);
  else if (ok) setPlayState(PLAY_STATE_IDLE);  // 正常播完
  else setPlayState(PLAY_STATE_ERROR);
  _playReadTaskHandle = nullptr;
  _playDecodeTaskHandle = nullptr;
}

bool Audio::play(const char *path) {
  if (!path || !path[0] || (!_begun && !begin())) return false;

  String source = normalizeFsPath(path);
  bool sourceIsUrl = isHttpSource(source.c_str());
  if (!source.length()) return false;
  if (!sourceIsUrl && !mountFS(&LittleFS, "lfs2")) return false;

  stop();

  AudioPlayContext *ctx = new AudioPlayContext();
  if (!ctx) return false;
  ctx->buf = (uint8_t *)malloc(NET_RX_CACHE_BYTES);
  ctx->cap = NET_RX_CACHE_BYTES;
  ctx->lock = xSemaphoreCreateMutex();
  ctx->useFile = !sourceIsUrl;
  ctx->source = source;
  if (!ctx->buf || !ctx->lock) {
    if (ctx->lock) vSemaphoreDelete(ctx->lock);
    free(ctx->buf);
    delete ctx;
    return false;
  }

  _playSource = source;
  _playSourceIsUrl = sourceIsUrl;
  _playStopRequested = false;
  _playPauseRequested = false;
  _playContext = ctx;
  setPlayState(PLAY_STATE_PLAYING);

  TaskFunction_t readTask = sourceIsUrl ? networkReadTask : localReadTask;
  const char *readTaskName = sourceIsUrl ? "networkReadTask" : "localReadTask";
  uint32_t readTaskStackSize = sourceIsUrl ? 24576 : 4096;  // 网络任务需要更大栈空间（HTTPClient + WiFiClientSecure）
    if (xTaskCreatePinnedToCore(readTask,
                                readTaskName,
                                readTaskStackSize,
                                ctx,
                                2,
                                &_playReadTaskHandle,
                                0) != pdPASS) {
      _playReadTaskHandle = nullptr;
      _playContext = nullptr;
      vSemaphoreDelete(ctx->lock);
      free(ctx->buf);
      delete ctx;
      setPlayState(PLAY_STATE_ERROR);
      return false;
    }
  ctx->readTaskHandle = _playReadTaskHandle;

  if (xTaskCreatePinnedToCore([](void *a){ static_cast<Audio*>(a)->playDecodeTask(); vTaskDelete(nullptr); },
                              "PlayDecodeTask",
                              8192,
                              this,
                              2,
                              &_playDecodeTaskHandle,
                              1) != pdPASS) {
    ctx->stop = true;
    uint32_t joinStart = millis();
    while (ctx->readTaskHandle && millis() - joinStart < 500) delay(2);
    _playReadTaskHandle = nullptr;
    _playDecodeTaskHandle = nullptr;
    _playContext = nullptr;
    vSemaphoreDelete(ctx->lock);
    free(ctx->buf);
    delete ctx;
    setPlayState(PLAY_STATE_ERROR);
    return false;
  }
  ctx->decodeTaskHandle = _playDecodeTaskHandle;
  return true;
}

bool Audio::pause() {
  if (_playState != PLAY_STATE_PLAYING) return false;
  _playPauseRequested = true;
  return true;
}

bool Audio::resume() {
  if (_playState != PLAY_STATE_PAUSED) return false;
  _playPauseRequested = false;
  setPlayState(PLAY_STATE_PLAYING);
  return true;
}

void Audio::stop() {
  if (!_playReadTaskHandle && !_playDecodeTaskHandle &&
      (_playState == PLAY_STATE_IDLE || _playState == PLAY_STATE_STOPPED ||
       _playState == PLAY_STATE_ERROR)) {
    return;
  }
  _playStopRequested = true;
  _playPauseRequested = false;
  AudioPlayContext *ctx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (ctx) ctx->stop = true;
  uint32_t startMs = millis();
  while ((_playReadTaskHandle || _playDecodeTaskHandle) &&
         millis() - startMs < PLAY_STOP_WAIT_MS) {
    delay(10);
  }
  ctx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (ctx) {
    _playContext = nullptr;
    if (ctx->lock) vSemaphoreDelete(ctx->lock);
    free(ctx->buf);
    delete ctx;
  }
  _playReadTaskHandle = nullptr;
  _playDecodeTaskHandle = nullptr;
  closePlayCodec();
  setPlayState(PLAY_STATE_STOPPED);
}

Audio::PlayState Audio::state() const { return _playState; }
const char *Audio::stateName() const {
  switch (_playState) {
    case PLAY_STATE_IDLE: return "idle";
    case PLAY_STATE_PLAYING: return "playing";
    case PLAY_STATE_PAUSED: return "paused";
    case PLAY_STATE_STOPPED: return "stopped";
    case PLAY_STATE_ERROR: return "error";
    default: return "unknown";
  }
}

