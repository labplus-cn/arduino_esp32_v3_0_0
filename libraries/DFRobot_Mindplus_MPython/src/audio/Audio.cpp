#include "audio/Audio.h"

#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
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

size_t expandMonoToStereo(const uint8_t *src,
                          size_t srcLen,
                          uint8_t bitsPerSample,
                          uint8_t *dst,
                          size_t dstCap) {
  if (!src || !dst || srcLen == 0) return 0;
  size_t bytesPerSample = bitsPerSample / 8;
  if (bytesPerSample == 0 || (srcLen % bytesPerSample) != 0) return 0;
  size_t sampleCount = srcLen / bytesPerSample;
  size_t outLen = sampleCount * bytesPerSample * 2;
  if (outLen > dstCap) return 0;

  const uint8_t *in = src;
  uint8_t *out = dst;
  for (size_t i = 0; i < sampleCount; ++i) {
    memcpy(out, in, bytesPerSample);
    memcpy(out + bytesPerSample, in, bytesPerSample);
    in += bytesPerSample;
    out += bytesPerSample * 2;
  }
  return outLen;

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
  std::unique_ptr<NetworkClient> client;
  uint8_t *tmp = (uint8_t *)malloc(1460);

  if (!tmp) {
    AUDIO_LOGLN("PlayUrl failed: read task malloc failed");
    goto done;
  }
  if (ctx && ctx->source.length()) {
    AUDIO_LOG("PlayUrl(url=%s)\n", ctx->source.c_str());
    if (!strncmp(ctx->source.c_str(), "https://", 8)) {
      auto *secureClient = new WiFiClientSecure();
      secureClient->setInsecure();
      client.reset(secureClient);
    } else {
      client.reset(new WiFiClient());
    }

    if (!client || !http.begin(*client, ctx->source)) {
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
      if (statusCode != HTTP_CODE_OK && statusCode != HTTP_CODE_PARTIAL_CONTENT) {
        AUDIO_LOG("PlayUrl failed: http status=%d\n", statusCode);
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
      _dataIf(nullptr),
      _codecIf(nullptr),
      _codecDev(nullptr),
      _begun(false),
      _recording(false),
      _decoderRegistered(false),
      _simpleDecoderRegistered(false),
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
      _recordBuffer(nullptr),
      _recordBufferSize(0),
      _recordBufferHead(0),
      _recordBufferTail(0),
      _recordBufferCount(0),
      _recordBufferLock(nullptr),
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

bool Audio::initI2C() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  return true;
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

  audio_codec_i2s_cfg_t dataConfig = {
      .port = I2S_PORT,
      .rx_handle = rx,
      .tx_handle = tx,
      .clk_src = I2S_CLK_SRC_DEFAULT,
  };
  _dataIf = audio_codec_new_i2s_data(&dataConfig);
  return _dataIf != nullptr;
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

  esp_codec_dev_cfg_t devConfig = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
      .codec_if = _codecIf,
      .data_if = _dataIf,
  };
  _codecDev = esp_codec_dev_new(&devConfig);
  return _codecDev != nullptr;
}

void Audio::deinitCodec() {
  closeCodec();

  if (_codecDev) {
    esp_codec_dev_delete(_codecDev);
    _codecDev = nullptr;
  }
  if (_codecIf) {
    audio_codec_delete_codec_if(_codecIf);
    _codecIf = nullptr;
  }
  if (_ctrlIf) {
    audio_codec_delete_ctrl_if(_ctrlIf);
    _ctrlIf = nullptr;
  }
  if (_dataIf) {
    audio_codec_delete_data_if(_dataIf);
    _dataIf = nullptr;
  }
  if (_gpioIf) {
    audio_codec_delete_gpio_if(_gpioIf);
    _gpioIf = nullptr;
  }
}

bool Audio::begin() {
  if (_begun) return true;

  AUDIO_LOGLN("begin()");

  if (!initI2C() || !initI2S() || !initCodec()) {
    AUDIO_LOGLN("begin failed during init");
    deinitCodec();
    return false;
  }

  _decoderRegistered = esp_audio_dec_register_default() == ESP_AUDIO_ERR_OK;
  _simpleDecoderRegistered =
      esp_audio_simple_dec_register_default() == ESP_AUDIO_ERR_OK;

  AUDIO_LOG("decoder registered=%d, simple decoder registered=%d\n",
            _decoderRegistered, _simpleDecoderRegistered);

  _begun = true;
  setVolume(_volume);
  setMicGain(_micGain);
  return true;
}

void Audio::end() {
  stop();
  stopRecord();

  if (_simpleDecoderRegistered) {
    esp_audio_simple_dec_unregister_default();
    _simpleDecoderRegistered = false;
  }
  if (_decoderRegistered) {
    esp_audio_dec_unregister_default();
    _decoderRegistered = false;
  }

  deinitCodec();
  _begun = false;
}

bool Audio::openCodec(uint32_t sr, uint8_t bits, uint8_t ch, bool in, bool out) {
  if (!_codecDev) {
    AUDIO_LOGLN("openCodec failed: _codecDev is null");
    return false;
  }

  if (out && ch == 1) {
    ch = 2;
  }
  if (in && ch == 1) {
    ch = 2;
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

  AUDIO_LOG(
      "openCodec(sr=%lu, bits=%u, ch=%u, in=%d, out=%d) => hw_ch=%u, mask=0x%X, mclk=%d\n",
      (unsigned long)sr,
      bits,
      ch,
      in,
      out,
      sampleInfo.channel,
      sampleInfo.channel_mask,
      sampleInfo.mclk_multiple);

  int status = esp_codec_dev_open(_codecDev, &sampleInfo);
  if (status != ESP_CODEC_DEV_OK) {
    AUDIO_LOG("openCodec failed: status=%d\n", status);
    return false;
  }
  if (out) esp_codec_dev_set_out_vol(_codecDev, _volume);
  if (in) esp_codec_dev_set_in_gain(_codecDev, _micGain);
  return true;
}

void Audio::closeCodec() {
  if (_codecDev) esp_codec_dev_close(_codecDev);
}

bool Audio::setVolume(uint8_t volume) {
  _volume = volume > 100 ? 100 : volume;
  return !_codecDev ||
         esp_codec_dev_set_out_vol(_codecDev, _volume) == ESP_CODEC_DEV_OK;
}

bool Audio::setMicGain(float gain) {
  _micGain = gain;
  return !_codecDev ||
         esp_codec_dev_set_in_gain(_codecDev, _micGain) == ESP_CODEC_DEV_OK;
}

bool Audio::initRecordBuffer(size_t bufferSize) {
  deinitRecordBuffer();
  _recordBuffer = (uint8_t *)malloc(bufferSize);
  if (!_recordBuffer) return false;
  _recordBufferLock = xSemaphoreCreateMutex();
  if (!_recordBufferLock) {
    free(_recordBuffer);
    _recordBuffer = nullptr;
    return false;
  }
  _recordBufferSize = bufferSize;
  _recordBufferHead = 0;
  _recordBufferTail = 0;
  _recordBufferCount = 0;
  return true;
}

void Audio::deinitRecordBuffer() {
  if (_recordBufferLock) {
    vSemaphoreDelete(_recordBufferLock);
    _recordBufferLock = nullptr;
  }
  free(_recordBuffer);
  _recordBuffer = nullptr;
  _recordBufferSize = 0;
  _recordBufferHead = 0;
  _recordBufferTail = 0;
  _recordBufferCount = 0;
}

size_t Audio::recordBufferWrite(const uint8_t *src, size_t len) {
  size_t written = 0;
  if (!_recordBuffer || !_recordBufferLock || !src || !len) return 0;
  if (xSemaphoreTake(_recordBufferLock, pdMS_TO_TICKS(10)) != pdTRUE) return 0;
  while (written < len && _recordBufferCount < _recordBufferSize) {
    _recordBuffer[_recordBufferHead] = src[written++];
    _recordBufferHead = (_recordBufferHead + 1) % _recordBufferSize;
    _recordBufferCount++;
  }
  xSemaphoreGive(_recordBufferLock);
  return written;
}

size_t Audio::recordBufferRead(uint8_t *dst, size_t len) {
  size_t read = 0;
  if (!_recordBuffer || !_recordBufferLock || !dst || !len) return 0;
  if (xSemaphoreTake(_recordBufferLock, pdMS_TO_TICKS(10)) != pdTRUE) return 0;
  while (read < len && _recordBufferCount > 0) {
    dst[read++] = _recordBuffer[_recordBufferTail];
    _recordBufferTail = (_recordBufferTail + 1) % _recordBufferSize;
    _recordBufferCount--;
  }
  xSemaphoreGive(_recordBufferLock);
  return read;
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
  if (!openCodec(sr, bits, ch, true, false)) {
    AUDIO_LOGLN("startRecord failed: openCodec failed");
    return false;
  }

  _recordFile = fs->open(normalizedPath.c_str(), FILE_WRITE, true);
  if (!_recordFile) {
    AUDIO_LOGLN("startRecord failed: open file failed");
    closeCodec();
    return false;
  }

  if (!initRecordBuffer(RECORD_RING_BUFFER_BYTES)) {
    AUDIO_LOGLN("startRecord failed: init record buffer failed");
    _recordFile.close();
    closeCodec();
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
    closeCodec();
    return false;
  }

  _recordStopRequested = false;
  _recordCaptureDone = false;
  _recordWriterDone = false;
  if (xTaskCreatePinnedToCore(recordCaptureTaskEntry,
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
    closeCodec();
    return false;
  }
  if (xTaskCreatePinnedToCore(recordWriterTaskEntry,
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
    closeCodec();
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
    int status = esp_codec_dev_read(_codecDev, buffer, (int)RECORD_BUFFER_BYTES);
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

  while (!_recordStopRequested || _recordBufferCount > 0 || !_recordCaptureDone) {
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
    closeCodec();
    _recording = false;
  }
}

bool Audio::recording() const { return _recording; }

esp_audio_simple_dec_type_t Audio::getDecoderType(const char *path) {
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

esp_audio_simple_dec_type_t Audio::getDecoderTypeFromContentType(
    const String &contentType) const {
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

void Audio::playDecodeTask() {
  AudioPlayContext *ctx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (!ctx) {
    _playDecodeTaskHandle = nullptr;
    setPlayState(PLAY_STATE_ERROR);
    return;
  }

  while (!ctx->sourceReady && !ctx->stop && !_playStopRequested) {
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  if (!ctx->initOk || ctx->type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) {
    _playDecodeTaskHandle = nullptr;
    setPlayState(PLAY_STATE_ERROR);
    return;
  }

  uint8_t decoderConfigStorage[sizeof(esp_m4a_dec_cfg_t)] = {};
  esp_audio_simple_dec_cfg_t decoderConfig = {
      .dec_type = ctx->type,
      .dec_cfg = nullptr,
      .cfg_size = 0,
  };
  configureSimpleDecoder(decoderConfig, decoderConfigStorage,
                         sizeof(decoderConfigStorage));

  esp_audio_simple_dec_handle_t decoder = nullptr;
  if (esp_audio_simple_dec_open(&decoderConfig, &decoder) != ESP_AUDIO_ERR_OK) {
    AUDIO_LOGLN("PlayDecodeTask failed: decoder open failed");
    _playStopRequested = true;
    _playDecodeTaskHandle = nullptr;
    return;
  }

  uint8_t *inputBuffer = (uint8_t *)malloc(PLAY_INPUT_BUFFER_BYTES);
  uint8_t *outputBuffer = (uint8_t *)malloc(PLAY_OUTPUT_BUFFER_BYTES);
  uint8_t *stereoBuffer = (uint8_t *)calloc(1, PLAY_OUTPUT_BUFFER_BYTES * 2);
  if (!inputBuffer || !outputBuffer || !stereoBuffer) {
    AUDIO_LOGLN("PlayDecodeTask failed: malloc failed");
    free(inputBuffer);
    free(outputBuffer);
    free(stereoBuffer);
    esp_audio_simple_dec_close(decoder);
    _playStopRequested = true;
    _playDecodeTaskHandle = nullptr;
    return;
  }

  bool ok = true;
  bool codecOpened = false;
  bool firstWriteLogged = false;
  esp_audio_simple_dec_info_t simpleInfo = {};
  setPlayState(PLAY_STATE_PLAYING);

  while (!_playStopRequested) {
    if (!waitWhilePaused()) break;
    size_t bytesRead = playBufferRead(ctx, inputBuffer, PLAY_INPUT_BUFFER_BYTES);
    if (!bytesRead) {
      if (ctx->readDone && ctx->count == 0) break;
      delay(2);
      continue;
    }

    esp_audio_simple_dec_raw_t raw = {
        .buffer = inputBuffer,
        .len = (uint32_t)bytesRead,
        .eos = ctx->readDone && ctx->count == 0,
        .consumed = 0,
        .frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE,
    };

    while (raw.len && !_playStopRequested) {
      if (!waitWhilePaused()) break;
      esp_audio_simple_dec_out_t frame = {
          .buffer = outputBuffer,
          .len = PLAY_OUTPUT_BUFFER_BYTES,
          .needed_size = 0,
          .decoded_size = 0,
      };

      esp_audio_err_t err = esp_audio_simple_dec_process(decoder, &raw, &frame);
      if (err == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
        uint8_t *resizedBuffer = (uint8_t *)realloc(outputBuffer, frame.needed_size);
        if (!resizedBuffer) {
          AUDIO_LOGLN("PlayDecodeTask failed: realloc failed");
          ok = false;
          _playStopRequested = true;
          break;
        }
        outputBuffer = resizedBuffer;
        continue;
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

      raw.buffer += raw.consumed;
      raw.len -= raw.consumed;

      if (frame.decoded_size > 0) {
        size_t writeSize = frame.decoded_size;
        uint8_t *writeBuffer = outputBuffer;
        uint8_t bitsPerSample = simpleInfo.bits_per_sample;
        uint8_t channels = simpleInfo.channel ? simpleInfo.channel : 2;

        if (!codecOpened) {
          if (esp_audio_simple_dec_get_info(decoder, &simpleInfo) != ESP_AUDIO_ERR_OK) {
            AUDIO_LOGLN("PlayDecodeTask waiting decoder info");
            continue;
          }
          bitsPerSample = simpleInfo.bits_per_sample;
          channels = simpleInfo.channel ? simpleInfo.channel : 2;
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
          if (!openCodec(simpleInfo.sample_rate, bitsPerSample, channels, false, true)) {
            AUDIO_LOGLN("PlayDecodeTask openCodec failed");
            ok = false;
            _playStopRequested = true;
            break;
          }
          simpleInfo.bits_per_sample = bitsPerSample;
          simpleInfo.channel = channels;
          codecOpened = true;
        }

        bitsPerSample = simpleInfo.bits_per_sample;
        channels = simpleInfo.channel ? simpleInfo.channel : 2;
        if (channels == 1) {
          memset(stereoBuffer, 0, PLAY_OUTPUT_BUFFER_BYTES * 2);
          writeSize = expandMonoToStereo(outputBuffer,
                                         frame.decoded_size,
                                         bitsPerSample,
                                         stereoBuffer,
                                         PLAY_OUTPUT_BUFFER_BYTES * 2);
          if (!writeSize) {
            AUDIO_LOG("PlayDecodeTask mono expand failed: decoded_size=%lu bits=%u\n",
                      (unsigned long)frame.decoded_size,
                      bitsPerSample);
            ok = false;
            _playStopRequested = true;
            break;
          }
          writeBuffer = stereoBuffer;
        }

        if (!firstWriteLogged) {
          AUDIO_LOG("PlayDecodeTask first write: decoded_size=%lu write_size=%lu src_ch=%u codec=%p buf=%p\n",
                    (unsigned long)frame.decoded_size,
                    (unsigned long)writeSize,
                    channels,
                    _codecDev,
                    writeBuffer);
          firstWriteLogged = true;
        }

        if (esp_codec_dev_write(_codecDev, writeBuffer, writeSize) != ESP_CODEC_DEV_OK) {
          AUDIO_LOG("PlayDecodeTask write failed: decoded_size=%lu write_size=%lu\n",
                    (unsigned long)frame.decoded_size,
                    (unsigned long)writeSize);
          ok = false;
          _playStopRequested = true;
          break;
        }
      }
    }
  }

  free(inputBuffer);
  free(outputBuffer);
  free(stereoBuffer);
  esp_audio_simple_dec_close(decoder);
  closeCodec();

  AudioPlayContext *doneCtx = reinterpret_cast<AudioPlayContext *>(_playContext);
  if (doneCtx == ctx) {
    _playContext = nullptr;
    if (doneCtx->lock) vSemaphoreDelete(doneCtx->lock);
    free(doneCtx->buf);
    delete doneCtx;
  }
  if (_playStopRequested) setPlayState(PLAY_STATE_STOPPED);
  else if (ok) setPlayState(PLAY_STATE_IDLE);
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
  if (xTaskCreatePinnedToCore(readTask,
                              readTaskName,
                              4096,
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

  if (xTaskCreatePinnedToCore(playDecodeTaskEntry,
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
  closeCodec();
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

