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

#include "audio/esp_sr_afe_api.h"
#include "esp_err.h"
#include <cstring>

// 语音合成（esp-sr）：仅在 cpp 中包含；头文件需已修补 C++ 兼容性（见 esp_tts.h / esp_tts_player.h）
extern "C" {
#include "audio/esp-sr/esp-tts/esp_tts_chinese/include/esp_tts.h"
#include "audio/esp-sr/esp-tts/esp_tts_chinese/include/esp_tts_voice_template.h"
}

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

constexpr EventBits_t kSrFeedDone = (1 << 0);
constexpr EventBits_t kSrDetectDone = (1 << 1);
constexpr EventBits_t kSrAllDone = kSrFeedDone | kSrDetectDone;

// AFE interface layout compatible with ESP-SR v1.x binaries bundled in Arduino ESP32 libs.
// We intentionally use this layout to avoid function-pointer offset mismatch when headers and
// linked binary are from different esp-sr releases.
typedef struct {
  esp_afe_sr_data_t *(*create_from_config)(afe_config_t *afe_config);
  int (*feed)(esp_afe_sr_data_t *afe, const int16_t *in);
  afe_fetch_result_t *(*fetch)(esp_afe_sr_data_t *afe);
  int (*reset_buffer)(esp_afe_sr_data_t *afe);
  int (*get_feed_chunksize)(esp_afe_sr_data_t *afe);
  int (*get_fetch_chunksize)(esp_afe_sr_data_t *afe);
  int (*get_total_channel_num)(esp_afe_sr_data_t *afe);
  int (*get_channel_num)(esp_afe_sr_data_t *afe);
  int (*get_samp_rate)(esp_afe_sr_data_t *afe);
  int (*set_wakenet)(esp_afe_sr_data_t *afe, char *model_name);
  int (*disable_wakenet)(esp_afe_sr_data_t *afe);
  int (*enable_wakenet)(esp_afe_sr_data_t *afe);
  int (*disable_aec)(esp_afe_sr_data_t *afe);
  int (*enable_aec)(esp_afe_sr_data_t *afe);
  int (*disable_se)(esp_afe_sr_data_t *afe);
  int (*enable_se)(esp_afe_sr_data_t *afe);
  void (*destroy)(esp_afe_sr_data_t *afe);
} afe_iface_legacy_t;

static inline afe_iface_legacy_t *asLegacyAfeIface(void *p) {
  return reinterpret_cast<afe_iface_legacy_t *>(p);
}

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
    ctx->count = ctx->count + 1;
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
    ctx->count = ctx->count - 1;
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
      _playCodecOpened(false),
      _recCodecOpened(false),
      _playSampleRate(0),
      _playBitsPerSample(0),
      _playChannels(0),
      _recSampleRate(0),
      _recBitsPerSample(0),
      _recChannels(0),
      _volume(DEFAULT_VOLUME),
      _micGain(35.0f),
      _recordFs(nullptr),
      _recordDataBytes(0),
      _recordSampleRate(16000),
      _recordBitsPerSample(16),
      _recordChannels(1),
      _ttsHandle(nullptr),
      _ttsInitialized(false),
      _ttsSemaphore(nullptr),
      _srAfeIface(nullptr),
      _srAfeData(nullptr),
      _srModels(nullptr),
      _srTaskFlag(0),
      _srLatestCommandId(0),
      _srWakeupFlag(0),
      _srMultinetReady(false),
      _srMultinet(nullptr),
      _srMnModel(nullptr),
      _srMnTimeoutMs(6000),
      _srLoadFromSdkconfig(false),
      _srDoneEvent(nullptr),
      _srFeedTaskHandle(nullptr),
      _srDetectTaskHandle(nullptr),
      _srCommandsAllocated(false) {}

Audio::~Audio() { end(); }

// 语音合成初始化
bool Audio::ttsInit() {
  if (_ttsInitialized) {
    return true;
  }
  
  // 查找 voice_data 分区
  const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "voice_data");
  if (!part) {
    AUDIO_LOGLN("ttsInit failed: voice_data partition not found");
    return false;
  }
  
  // 映射语音数据
  const void *voicedata;
  esp_partition_mmap_handle_t mmap;
  esp_err_t err = esp_partition_mmap(part, 0, part->size, ESP_PARTITION_MMAP_DATA, (const void **)&voicedata, &mmap);
  if (err != ESP_OK) {
    AUDIO_LOGLN("ttsInit failed: mmap failed");
    return false;
  }
  
  // 初始化语音
  esp_tts_voice_t *voice = esp_tts_voice_set_init(&esp_tts_voice_template, (int16_t *)voicedata);
  if (!voice) {
    AUDIO_LOGLN("ttsInit failed: voice init failed");
    return false;
  }
  
  // 创建 TTS 句柄
  _ttsHandle = esp_tts_create(voice);
  if (!_ttsHandle) {
    AUDIO_LOGLN("ttsInit failed: esp_tts_create failed");
    return false;
  }
  
  // 创建信号量
  _ttsSemaphore = xSemaphoreCreateBinary();
  if (!_ttsSemaphore) {
    AUDIO_LOGLN("ttsInit failed: semaphore create failed");
    esp_tts_destroy(_ttsHandle);
    _ttsHandle = nullptr;
    return false;
  }
  
  xSemaphoreGive(_ttsSemaphore);
  _ttsInitialized = true;
  return true;
}

// 文本转语音
bool Audio::textToSpeech(const char *text) {
  if (!_ttsInitialized) {
    AUDIO_LOGLN("textToSpeech failed: TTS not initialized");
    return false;
  }
  
  if (!text || strlen(text) == 0) {
    AUDIO_LOGLN("textToSpeech failed: empty text");
    return false;
  }
  
  // 创建参数结构体
  struct TtsTaskParams {
    Audio *audio;
    char text[256];
  };
  
  TtsTaskParams *params = (TtsTaskParams *)malloc(sizeof(TtsTaskParams));
  if (!params) {
    AUDIO_LOGLN("textToSpeech failed: malloc failed");
    return false;
  }
  
  params->audio = this;
  strncpy(params->text, text, sizeof(params->text) - 1);
  params->text[sizeof(params->text) - 1] = '\0';
  
  // 创建 TTS 任务
  if (xTaskCreatePinnedToCore(ttsTask, "tts_task", 4 * 1024, (void *)params, 5, nullptr, 0) != pdPASS) {
    AUDIO_LOGLN("textToSpeech failed: task create failed");
    free(params);
    return false;
  }
  
  return true;
}

// 语音合成任务
void Audio::ttsTask(void *arg) {
  struct TtsTaskParams {
    Audio *audio;
    char text[256];
  };
  
  TtsTaskParams *params = (TtsTaskParams *)arg;
  if (!params || !params->audio) {
    vTaskDelete(nullptr);
    return;
  }
  
  Audio *audio = params->audio;
  const char *text = params->text;
  
  // 等待信号量
  if (xSemaphoreTake(audio->_ttsSemaphore, portMAX_DELAY) != pdTRUE) {
    free(params);
    vTaskDelete(nullptr);
    return;
  }
  
  if (!audio->openPlayCodec(16000, 16, 1)) {
    AUDIO_LOGLN("ttsTask failed: openPlayCodec failed");
    xSemaphoreGive(audio->_ttsSemaphore);
    free(params);
    vTaskDelete(nullptr);
    return;
  }
  
  if (esp_tts_parse_chinese(audio->_ttsHandle, text)) {
      int len;
      do {
          short *pcm = esp_tts_stream_play(audio->_ttsHandle, &len, 0);
          if(len > 0){
              esp_codec_dev_write(audio->_playDev, (int8_t *)pcm, len * 2);
          }
      } while (len > 0);
  }
  
  // 清理资源
  esp_tts_stream_reset(audio->_ttsHandle);
  // audio->closePlayCodec();
  xSemaphoreGive(audio->_ttsSemaphore);
  free(params);
  vTaskDelete(nullptr);
}

bool Audio::mountFS(fs::FS *fs, const char *label) {
  return fs && (fs != &LittleFS || LittleFS.begin(true, "/littlefs", 5, label));
}


bool Audio::codecCreate() {
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
  if (!_dataIf) {
    i2s_del_channel(tx);
    i2s_del_channel(rx);
    return false;
  }

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
      .data_if = _dataIf,
  };
  _playDev = esp_codec_dev_new(&playDevConfig);
  if (!_playDev) return false;

  esp_codec_dev_cfg_t recDevConfig = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = _codecIf,
      .data_if = _dataIf,
  };
  _recDev = esp_codec_dev_new(&recDevConfig);
  return _recDev != nullptr;
}
void Audio::codecDestroy() {
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

  if (!codecCreate()) {
    AUDIO_LOGLN("begin failed during init");
    codecDestroy();
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
  srEnd();

  // 清理语音合成资源
  if (_ttsHandle) {
    esp_tts_destroy(_ttsHandle);
    _ttsHandle = nullptr;
  }
  if (_ttsSemaphore) {
    vSemaphoreDelete(_ttsSemaphore);
    _ttsSemaphore = nullptr;
  }
  _ttsInitialized = false;
  
  codecDestroy();
  _begun = false;
}

bool Audio::openPlayCodec(uint32_t sr, uint8_t bits, uint8_t ch) {
  if (!_playDev) {
    AUDIO_LOGLN("openPlayCodec failed: _playDev is null");
    return false;
  }

  esp_codec_dev_sample_info_t sampleInfo = {
      .bits_per_sample = bits,
      .channel = ch,
      .channel_mask = 0,
      .sample_rate = sr,
      .mclk_multiple = 256,
  };

  AUDIO_LOG("openPlayCodec(sr=%lu, bits=%u, ch=%u) mclk=%d\n",
      (unsigned long)sr, bits, ch, sampleInfo.mclk_multiple);

  int status = esp_codec_dev_open(_playDev, &sampleInfo);
  if (status != ESP_CODEC_DEV_OK) {
    AUDIO_LOG("openPlayCodec failed: status=%d\n", status);
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
  // RINGBUF_TYPE_BYTEBUF：无头部开销，字节流语义，天然线程安全，无需额外 mutex
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
  // 等待最多 10ms；缓冲区满时丢弃本次数据（采集任务不应长期阻塞）
  if (xRingbufferSend(_recordRingbuf, src, len, pdMS_TO_TICKS(10)) == pdTRUE) {
    return len;
  }
  // 缓冲区已满：写入尽可能多的字节（逐块尝试，直到无可用空间）
  size_t written = 0;
  while (written < len) {
    size_t free = xRingbufferGetCurFreeSize(_recordRingbuf);
    if (free == 0) break;
    size_t chunk = (len - written) < free ? (len - written) : free;
    if (xRingbufferSend(_recordRingbuf, src + written, chunk, 0) != pdTRUE) break;
    written += chunk;
  }
  return written;
}

size_t Audio::recordBufferRead(uint8_t *dst, size_t len) {
  if (!_recordRingbuf || !dst || !len) return 0;
  size_t read = 0;
  while (read < len) {
    size_t itemSize = 0;
    // xRingbufferReceiveUpTo 最多取 (len-read) 字节，立即返回（timeout=0）
    void *item = xRingbufferReceiveUpTo(_recordRingbuf, &itemSize,
                                        pdMS_TO_TICKS(10), len - read);
    if (!item) break;
    memcpy(dst + read, item, itemSize);
    vRingbufferReturnItem(_recordRingbuf, item);
    read += itemSize;
  }
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

  if (_srTaskFlag) {
    AUDIO_LOGLN("startRecord failed: speech recognition is active");
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
         xRingbufferGetCurFreeSize(_recordRingbuf) < RECORD_RING_BUFFER_BYTES) {
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
  bool codecOpened = false;
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
            AUDIO_LOGLN("PlayDecodeTask openPlayCodec failed");
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
  uint32_t readTaskStackSize = sourceIsUrl ? 10240 : 4096;  // 网络任务需要更大栈空间（HTTPClient + WiFiClientSecure）
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

// ---------------------------------------------------------------------------
// 离线语音识别（esp-sr：WakeNet + MultiNet），逻辑参考 temp/audio/src/sr/sc.c
// ---------------------------------------------------------------------------

void Audio::srFeedTaskEntry(void *arg) {
  static_cast<Audio *>(arg)->srFeedTask();
  vTaskDelete(nullptr);
}

void Audio::srDetectTaskEntry(void *arg) {
  static_cast<Audio *>(arg)->srDetectTask();
  vTaskDelete(nullptr);
}

void Audio::srFeedTask() {
  afe_iface_legacy_t *iface = asLegacyAfeIface(_srAfeIface);
  esp_afe_sr_data_t *afe = reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData);
  int chunk = iface->get_feed_chunksize(afe);
  int nch = iface->get_total_channel_num(afe);
  // Compatibility guard for mixed esp-sr header/lib versions.
  if (chunk < 64 || chunk > 4096) chunk = 512;
  if (nch <= 0 || nch > 4) nch = 1;
  size_t bytes = (size_t)chunk * (size_t)nch * sizeof(int16_t);
  int16_t *buf = reinterpret_cast<int16_t *>(malloc(bytes));
  if (!buf) {
    AUDIO_LOGLN("SR feed: malloc failed");
    if (_srDoneEvent) xEventGroupSetBits(_srDoneEvent, kSrFeedDone);
    return;
  }
  while (_srTaskFlag) {
    if (esp_codec_dev_read(_recDev, buf, (int)bytes) != ESP_CODEC_DEV_OK) {
      vTaskDelay(pdMS_TO_TICKS(5));
      continue;
    }
    iface->feed(afe, buf);
  }
  free(buf);
  if (_srDoneEvent) xEventGroupSetBits(_srDoneEvent, kSrFeedDone);
}

void Audio::srDetectTask() {
  afe_iface_legacy_t *afe = asLegacyAfeIface(_srAfeIface);
  esp_afe_sr_data_t *afe_data = reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData);
  srmodel_list_t *models = reinterpret_cast<srmodel_list_t *>(_srModels);
  esp_mn_iface_t *multinet = nullptr;
  model_iface_data_t *model_data = nullptr;

  int afe_chunksize = afe->get_fetch_chunksize(afe_data);
  char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_CHINESE);
  if (!mn_name) {
    AUDIO_LOGLN("SR detect: no Chinese multinet model");
    goto finish;
  }
  multinet = esp_mn_handle_from_name(mn_name);
  if (!multinet) {
    AUDIO_LOGLN("SR detect: esp_mn_handle_from_name failed");
    goto finish;
  }
  model_data = multinet->create(mn_name, (int)_srMnTimeoutMs);
  if (!model_data) {
    AUDIO_LOGLN("SR detect: multinet->create failed");
    goto finish;
  }
  if (esp_mn_commands_alloc(multinet, model_data) != ESP_OK) {
    AUDIO_LOGLN("SR detect: esp_mn_commands_alloc failed");
    multinet->destroy(model_data);
    model_data = nullptr;
    goto finish;
  }
  _srCommandsAllocated = true;

  if (_srLoadFromSdkconfig) {
    esp_mn_commands_update_from_sdkconfig(multinet, model_data);
    esp_mn_commands_clear();
    esp_mn_commands_update();
  }

  {
    int mu_chunksize = multinet->get_samp_chunksize(model_data);
    if (mu_chunksize != afe_chunksize) {
      AUDIO_LOG("SR detect: chunk mismatch afe=%d mn=%d\n", afe_chunksize, mu_chunksize);
      // Compatibility path: some esp-sr binary/header combinations report an
      // inconsistent fetch chunk size but still work with fetch()->data.
      // Keep running instead of aborting SR startup.
    }
  }

  _srMultinet = multinet;
  _srMnModel = model_data;
  _srMultinetReady = true;

  AUDIO_LOGLN("SR detect: started");

  while (_srTaskFlag) {
    afe_fetch_result_t *res = afe->fetch(afe_data);
    if (!res || res->ret_value == ESP_FAIL) {
      AUDIO_LOGLN("SR detect: fetch error");
      break;
    }

    if (res->wakeup_state == WAKENET_DETECTED) {
      if (_srWakeupFlag == 0) {
        multinet->clean(model_data);
        afe->disable_wakenet(afe_data);
        if (!_ttsInitialized && !ttsInit()) {
          AUDIO_LOGLN("SR wake detected: ttsInit failed");
        } else {
          textToSpeech(_srWakeupReplyTts.c_str());
        }
      }
      _srWakeupFlag = 1;
    } else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED) {
      _srWakeupFlag = 1;
    }

    if (_srWakeupFlag == 1) {
      esp_mn_state_t mn_state = multinet->detect(model_data, res->data);
      if (mn_state == ESP_MN_STATE_DETECTING) {
        continue;
      }
      if (mn_state == ESP_MN_STATE_DETECTED) {
        esp_mn_results_t *mn_result = multinet->get_results(model_data);
        if (mn_result && mn_result->num > 0) {
          _srLatestCommandId = mn_result->command_id[0];
          afe->enable_wakenet(afe_data);
          _srWakeupFlag = 0;
        }
        continue;
      }
      if (mn_state == ESP_MN_STATE_TIMEOUT) {
        afe->enable_wakenet(afe_data);
        _srWakeupFlag = 0;
        continue;
      }
    }
  }

  if (model_data && multinet) {
    multinet->destroy(model_data);
    model_data = nullptr;
  }
  if (_srCommandsAllocated) {
    esp_mn_commands_free();
    _srCommandsAllocated = false;
  }
  _srMultinet = nullptr;
  _srMnModel = nullptr;
  _srMultinetReady = false;

finish:
  if (_srDoneEvent) xEventGroupSetBits(_srDoneEvent, kSrDetectDone);
}

bool Audio::speechRecognitionDeinitInternal() {
  if (_srAfeIface && _srAfeData) {
    afe_iface_legacy_t *iface = asLegacyAfeIface(_srAfeIface);
    esp_afe_sr_data_t *data = reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData);
    iface->destroy(data);
  }
  _srAfeIface = nullptr;
  _srAfeData = nullptr;

  if (_srCommandsAllocated) {
    esp_mn_commands_free();
    _srCommandsAllocated = false;
  }
  if (_srModels) {
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
  }
  _srMultinet = nullptr;
  _srMnModel = nullptr;
  closePlayCodec();
  closeRecCodec();
  if (_srDoneEvent) {
    vEventGroupDelete(_srDoneEvent);
    _srDoneEvent = nullptr;
  }
  _srWakeupReplyTts = "";
  return true;
}

bool Audio::srBegin(const char *wakeupReplyTts,
                                    uint16_t multinetTimeoutMs,
                                    bool loadCommandsFromSdkconfig) {
  if (_srAfeIface) return true;
  if (_recording) {
    AUDIO_LOGLN("SR begin: file recording active");
    return false;
  }
  if ((!_begun && !begin()) || !_recDev) return false;

  _srMnTimeoutMs = multinetTimeoutMs;
  _srLoadFromSdkconfig = loadCommandsFromSdkconfig;
  _srWakeupReplyTts = wakeupReplyTts ? wakeupReplyTts : "我在";
  _srLatestCommandId = 0;
  _srWakeupFlag = 0;
  _srMultinetReady = false;
  _srMultinet = nullptr;
  _srMnModel = nullptr;
  _srCommandsAllocated = false;

  _srModels = esp_srmodel_init("model");
  if (!_srModels) {
    AUDIO_LOGLN("SR begin: esp_srmodel_init failed");
    return false;
  }
  if (!esp_srmodel_filter(reinterpret_cast<srmodel_list_t *>(_srModels), ESP_WN_PREFIX,
                          nullptr)) {
    AUDIO_LOGLN("SR begin: no wakenet model");
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }
  if (!esp_srmodel_filter(reinterpret_cast<srmodel_list_t *>(_srModels), ESP_MN_PREFIX,
                          ESP_MN_CHINESE)) {
    AUDIO_LOGLN("SR begin: no Chinese multinet");
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }

  afe_config_t *afe_cfg =
      afe_config_init("M", reinterpret_cast<srmodel_list_t *>(_srModels), AFE_TYPE_SR,
                      AFE_MODE_HIGH_PERF);
  if (!afe_cfg) {
    AUDIO_LOGLN("SR begin: afe_config_init failed");
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }
  _srAfeIface = esp_afe_handle_from_config(afe_cfg);
  if (!_srAfeIface) {
    AUDIO_LOGLN("SR begin: esp_afe_handle_from_config failed");
    afe_config_free(afe_cfg);
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }
  afe_iface_legacy_t *iface = asLegacyAfeIface(_srAfeIface);
  _srAfeData = iface->create_from_config(afe_cfg);
  afe_config_free(afe_cfg);
  if (!_srAfeData) {
    AUDIO_LOGLN("SR begin: create_from_config failed");
    _srAfeIface = nullptr;
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }
  iface->disable_aec(reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData));

  _srDoneEvent = xEventGroupCreate();
  if (!_srDoneEvent) {
    AUDIO_LOGLN("SR begin: event group failed");
    iface->destroy(reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData));
    _srAfeData = nullptr;
    _srAfeIface = nullptr;
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }

  if (!openRecCodec(16000, 16, 1)) {
    AUDIO_LOGLN("SR begin: openRecCodec failed");
    vEventGroupDelete(_srDoneEvent);
    _srDoneEvent = nullptr;
    iface->destroy(reinterpret_cast<esp_afe_sr_data_t *>(_srAfeData));
    _srAfeData = nullptr;
    _srAfeIface = nullptr;
    esp_srmodel_deinit(reinterpret_cast<srmodel_list_t *>(_srModels));
    _srModels = nullptr;
    return false;
  }

  xEventGroupClearBits(_srDoneEvent, kSrAllDone);
  _srTaskFlag = 1;

  if (xTaskCreatePinnedToCore(srDetectTaskEntry, "audio_sr_det", 8192, this, 5,
                              &_srDetectTaskHandle, 1) != pdPASS) {
    AUDIO_LOGLN("SR begin: detect task failed");
    _srTaskFlag = 0;
    speechRecognitionDeinitInternal();
    return false;
  }
  if (xTaskCreatePinnedToCore(srFeedTaskEntry, "audio_sr_feed", 8192, this, 5,
                              &_srFeedTaskHandle, 0) != pdPASS) {
    AUDIO_LOGLN("SR begin: feed task failed");
    _srTaskFlag = 0;
    (void)xEventGroupWaitBits(_srDoneEvent, kSrDetectDone, pdTRUE, pdTRUE,
                             pdMS_TO_TICKS(8000));
    _srDetectTaskHandle = nullptr;
    speechRecognitionDeinitInternal();
    return false;
  }
  return true;
}

void Audio::srEnd() {
  if (!_srAfeIface && !_srModels) return;
  _srMultinetReady = false;
  _srTaskFlag = 0;
  if (_srDoneEvent) {
    EventBits_t bits = xEventGroupWaitBits(_srDoneEvent, kSrAllDone, pdTRUE, pdTRUE,
                                           pdMS_TO_TICKS(8000));
    if ((bits & kSrAllDone) != kSrAllDone) {
      AUDIO_LOGLN("SR end: task join timeout");
    }
  }
  _srFeedTaskHandle = nullptr;
  _srDetectTaskHandle = nullptr;
  speechRecognitionDeinitInternal();
}

bool Audio::srRunning() const { return _srAfeIface != nullptr; }

bool Audio::srWaitReady(uint32_t timeoutMs) {
  uint32_t start = millis();
  while (!_srMultinetReady && millis() - start < timeoutMs) {
    delay(10);
  }
  return _srMultinetReady;
}

bool Audio::srAddCommand(int commandId, const char *phraseUtf8) {
  if (!_srMultinetReady && !srWaitReady()) return false;
  if (!phraseUtf8 || !phraseUtf8[0]) return false;
  char buf[ESP_MN_MAX_PHRASE_LEN + 1];
  strncpy(buf, phraseUtf8, ESP_MN_MAX_PHRASE_LEN);
  buf[ESP_MN_MAX_PHRASE_LEN] = '\0';
  char *dup = strdup(buf);
  if (!dup) return false;
  esp_err_t e = esp_mn_commands_add(commandId, dup);
  free(dup);
  return e == ESP_OK;
}

bool Audio::srClearCommands() {
  if (!_srMultinetReady && !srWaitReady()) return false;
  return esp_mn_commands_clear() == ESP_OK;
}

bool Audio::srApplyCommands() {
  if (!_srMultinetReady && !srWaitReady()) return false;
  esp_mn_error_t *err = esp_mn_commands_update();
  if (err) {
    AUDIO_LOG("SR apply: multinet rejected some phrases (num=%d)\n", err->num);
    return false;
  }
  return true;
}

int Audio::srGetCommandId() {
  int commandId = _srLatestCommandId;
  _srLatestCommandId = 0;
  return commandId;
}



