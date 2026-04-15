#include "audio/Audio.h"

#include <LittleFS.h>
#include <Wire.h>

#include "driver/i2s_std.h"
#include "audio/esp_codec_dev/device/include/es8388_codec.h"
#include "audio/esp_codec_dev/include/esp_codec_dev.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_defaults.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_aac_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_m4a_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_ts_dec.h"

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
constexpr size_t PLAY_INPUT_BUFFER_BYTES = 2048;
constexpr size_t PLAY_OUTPUT_BUFFER_BYTES = 8192;

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

uint16_t rd16(const uint8_t *data) {
  return uint16_t(data[0]) | (uint16_t(data[1]) << 8);
}

uint32_t rd32(const uint8_t *data) {
  return uint32_t(data[0]) | (uint32_t(data[1]) << 8) |
         (uint32_t(data[2]) << 16) | (uint32_t(data[3]) << 24);
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
      _playing(false),
      _decoderRegistered(false),
      _simpleDecoderRegistered(false),
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
  stopPlay();
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

  esp_codec_dev_sample_info_t sampleInfo = {
      .bits_per_sample = bits,
      .channel = uint8_t(ch == 1 ? 2 : ch),
      .channel_mask = uint16_t(ch == 1 ? ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) : 0),
      .sample_rate = sr,
      .mclk_multiple = 256,
  };

  AUDIO_LOG(
      "openCodec(sr=%lu, bits=%u, ch=%u, in=%d, out=%d) => hw_ch=%u, mask=0x%X\n",
      (unsigned long)sr,
      bits,
      ch,
      in,
      out,
      sampleInfo.channel,
      sampleInfo.channel_mask);

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

bool Audio::startRecord(const char *path, uint32_t sr, uint8_t bits, uint8_t ch) {
  AUDIO_LOG("startRecord(path=%s, sr=%lu, bits=%u, ch=%u)\n",
            path ? path : "<null>",
            (unsigned long)sr,
            bits,
            ch);

  fs::FS *fs = &LittleFS;
  const char *label = "lfs2";
  if ((!_begun && !begin()) || !mountFS(fs, label)) {
    AUDIO_LOGLN("startRecord failed: begin or mountFS failed");
    return false;
  }

  stopRecord();
  if (!openCodec(sr, bits, ch, true, false)) {
    AUDIO_LOGLN("startRecord failed: openCodec failed");
    return false;
  }

  _recordFile = fs->open(path, FILE_WRITE, true);
  if (!_recordFile) {
    AUDIO_LOGLN("startRecord failed: open file failed");
    closeCodec();
    return false;
  }

  _recordFs = fs;
  _recordSampleRate = sr;
  _recordBitsPerSample = bits;
  _recordChannels = ch;
  _recordDataBytes = 0;
  _recording = writeWavHeader(_recordFile, 0, sr, bits, ch);
  AUDIO_LOG("startRecord header write=%d\n", _recording);
  return _recording;
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

  uint32_t startMs = millis();
  while (_recording && millis() - startMs < durationMs) {
    recordChunk();
    delay(10);
  }
  stopRecord();
  return true;
}

size_t Audio::recordChunk(size_t maxBytes) {
  if (!_recording || !_recordFile || !_codecDev) return 0;

  size_t bytesToRead = maxBytes == 0 ? RECORD_BUFFER_BYTES : maxBytes;
  uint8_t *buffer = (uint8_t *)malloc(bytesToRead);
  if (!buffer) {
    AUDIO_LOGLN("recordChunk failed: malloc failed");
    return 0;
  }

  int status = esp_codec_dev_read(_codecDev, buffer, (int)bytesToRead);
  size_t capturedBytes = 0;
  if (status == ESP_CODEC_DEV_OK) {
    bool allZero = true;
    for (size_t i = 0; i < bytesToRead; ++i) {
      if (buffer[i] != 0) {
        allZero = false;
        break;
      }
    }

    size_t written = _recordFile.write(buffer, bytesToRead);
    if (written == bytesToRead) {
      _recordDataBytes += (uint32_t)written;
      capturedBytes = written;
      static uint32_t chunkIndex = 0;
      if (chunkIndex < 8 || allZero) {
        AUDIO_LOG(
            "recordChunk[%lu]: req=%u status=%d written=%u zero=%d first=%02X %02X %02X %02X\n",
            (unsigned long)chunkIndex,
            (unsigned)bytesToRead,
            status,
            (unsigned)written,
            allZero,
            bytesToRead > 0 ? buffer[0] : 0,
            bytesToRead > 1 ? buffer[1] : 0,
            bytesToRead > 2 ? buffer[2] : 0,
            bytesToRead > 3 ? buffer[3] : 0);
      }
      ++chunkIndex;
    } else {
      AUDIO_LOG("recordChunk write failed: want=%u actual=%u\n",
                (unsigned)bytesToRead,
                (unsigned)written);
      _recording = false;
    }
  } else {
    AUDIO_LOG("recordChunk read failed: status=%d req=%u\n",
              status,
              (unsigned)bytesToRead);
  }

  free(buffer);
  return capturedBytes;
}

void Audio::stopRecord() {
  if (_recording) {
    AUDIO_LOG("stopRecord dataBytes=%lu\n", (unsigned long)_recordDataBytes);
    finalizeWavHeader();
    _recordFile.flush();
    _recordFile.close();
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

bool Audio::playStream(Stream &stream, esp_audio_simple_dec_type_t type) {
  if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE || (!_begun && !begin())) {
    AUDIO_LOG("playStream failed before start: type=%d, begun=%d\n", (int)type,
              _begun);
    return false;
  }

  stopPlay();

  uint8_t decoderConfigStorage[sizeof(esp_m4a_dec_cfg_t)] = {};
  esp_audio_simple_dec_cfg_t decoderConfig = {
      .dec_type = type,
      .dec_cfg = nullptr,
      .cfg_size = 0,
  };
  configureSimpleDecoder(decoderConfig, decoderConfigStorage,
                         sizeof(decoderConfigStorage));
  esp_audio_simple_dec_handle_t decoder = nullptr;
  if (esp_audio_simple_dec_open(&decoderConfig, &decoder) != ESP_AUDIO_ERR_OK) {
    AUDIO_LOGLN("playStream failed: decoder open failed");
    return false;
  }

  uint8_t *inputBuffer = (uint8_t *)malloc(PLAY_INPUT_BUFFER_BYTES);
  uint8_t *outputBuffer = (uint8_t *)malloc(PLAY_OUTPUT_BUFFER_BYTES);
  if (!inputBuffer || !outputBuffer) {
    AUDIO_LOGLN("playStream failed: malloc failed");
    free(inputBuffer);
    free(outputBuffer);
    esp_audio_simple_dec_close(decoder);
    return false;
  }

  esp_audio_simple_dec_info_t info = {};
  bool codecOpened = false;
  bool ok = true;
  _playing = true;

  while (_playing) {
    size_t bytesRead = stream.readBytes((char *)inputBuffer, PLAY_INPUT_BUFFER_BYTES);
    if (!bytesRead) break;

    esp_audio_simple_dec_raw_t raw = {
        .buffer = inputBuffer,
        .len = (uint32_t)bytesRead,
        .eos = bytesRead < PLAY_INPUT_BUFFER_BYTES,
        .consumed = 0,
        .frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE,
    };

    while (raw.len && _playing) {
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
          AUDIO_LOGLN("playStream failed: realloc failed");
          ok = false;
          _playing = false;
          break;
        }
        outputBuffer = resizedBuffer;
        continue;
      }
      if (err != ESP_AUDIO_ERR_OK) {
        AUDIO_LOG("playStream decode failed: err=%d raw.len=%lu consumed=%lu\n",
                  (int)err,
                  (unsigned long)raw.len,
                  (unsigned long)raw.consumed);
        ok = false;
        _playing = false;
        break;
      }

      raw.buffer += raw.consumed;
      raw.len -= raw.consumed;

      if (frame.decoded_size > 0) {
        if (!codecOpened) {
          if (esp_audio_simple_dec_get_info(decoder, &info) != ESP_AUDIO_ERR_OK) {
            AUDIO_LOGLN("playStream waiting decoder info");
            continue;
          }

          uint8_t channels = info.channel ? info.channel : 2;
          AUDIO_LOG("playStream decoded info: sr=%lu bits=%u ch=%u\n",
                    (unsigned long)info.sample_rate,
                    info.bits_per_sample,
                    channels);
          if (!openCodec(info.sample_rate, info.bits_per_sample, channels, false,
                         true)) {
            AUDIO_LOGLN("playStream failed: openCodec for playback failed");
            ok = false;
            _playing = false;
            break;
          }
          codecOpened = true;
        }

        if (esp_codec_dev_write(_codecDev, outputBuffer, frame.decoded_size) !=
            ESP_CODEC_DEV_OK) {
          AUDIO_LOG("playStream write failed: decoded_size=%lu\n",
                    (unsigned long)frame.decoded_size);
          ok = false;
          _playing = false;
          break;
        }
      }
    }
  }

  free(inputBuffer);
  free(outputBuffer);
  esp_audio_simple_dec_close(decoder);
  closeCodec();
  _playing = false;
  return ok;
}

bool Audio::playWavFile(File &file) {
  uint8_t header[44] = {0};
  if (file.read(header, sizeof(header)) != sizeof(header)) {
    AUDIO_LOGLN("playWavFile failed: header too short");
    return false;
  }

  if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
    AUDIO_LOGLN("playWavFile failed: invalid RIFF/WAVE header");
    return false;
  }

  if (memcmp(header + 12, "fmt ", 4) != 0 || memcmp(header + 36, "data", 4) != 0) {
    AUDIO_LOGLN("playWavFile failed: unsupported WAV layout");
    return false;
  }

  uint16_t audioFormat = rd16(header + 20);
  uint16_t channels = rd16(header + 22);
  uint32_t sampleRate = rd32(header + 24);
  uint16_t bitsPerSample = rd16(header + 34);
  uint32_t dataSize = rd32(header + 40);

  AUDIO_LOG(
      "playWavFile fmt=%u channels=%u sampleRate=%lu bits=%u dataSize=%lu\n",
      audioFormat,
      channels,
      (unsigned long)sampleRate,
      bitsPerSample,
      (unsigned long)dataSize);

  if (audioFormat != 1 || (bitsPerSample != 16 && bitsPerSample != 8) ||
      channels == 0) {
    AUDIO_LOGLN("playWavFile failed: unsupported PCM format");
    return false;
  }

  if (!openCodec(sampleRate, (uint8_t)bitsPerSample, (uint8_t)channels, false,
                 true)) {
    AUDIO_LOGLN("playWavFile failed: openCodec failed");
    return false;
  }

  _playing = true;
  bool ok = true;
  uint8_t *buffer = (uint8_t *)malloc(PLAY_OUTPUT_BUFFER_BYTES);
  if (!buffer) {
    AUDIO_LOGLN("playWavFile failed: malloc failed");
    closeCodec();
    _playing = false;
    return false;
  }

  while (_playing) {
    size_t bytesRead = file.read(buffer, PLAY_OUTPUT_BUFFER_BYTES);
    if (!bytesRead) break;

    if (esp_codec_dev_write(_codecDev, buffer, (int)bytesRead) !=
        ESP_CODEC_DEV_OK) {
      AUDIO_LOG("playWavFile write failed: bytesRead=%u\n", (unsigned)bytesRead);
      ok = false;
      break;
    }
  }

  free(buffer);
  closeCodec();
  _playing = false;
  return ok;
}

bool Audio::playMp3File(File &file) {
  AUDIO_LOGLN("playMp3File enter");

  void *decoder = nullptr;
  if (esp_mp3_dec_open(nullptr, 0, &decoder) != ESP_AUDIO_ERR_OK) {
    AUDIO_LOGLN("playMp3File failed: decoder open failed");
    return false;
  }

  uint8_t *inputBuffer = (uint8_t *)malloc(PLAY_INPUT_BUFFER_BYTES);
  uint8_t *outputBuffer = (uint8_t *)malloc(PLAY_OUTPUT_BUFFER_BYTES);
  if (!inputBuffer || !outputBuffer) {
    AUDIO_LOGLN("playMp3File failed: malloc failed");
    free(inputBuffer);
    free(outputBuffer);
    esp_mp3_dec_close(decoder);
    return false;
  }

  esp_audio_dec_info_t info = {};
  bool codecOpened = false;
  bool firstWriteLogged = false;
  bool ok = true;
  _playing = true;

  while (_playing) {
    size_t bytesRead = file.read(inputBuffer, PLAY_INPUT_BUFFER_BYTES);
    if (!bytesRead) break;

    esp_audio_dec_in_raw_t raw = {
        .buffer = inputBuffer,
        .len = (uint32_t)bytesRead,
        .consumed = 0,
        .frame_recover = ESP_AUDIO_DEC_RECOVERY_NONE,
    };

    while (raw.len && _playing) {
      esp_audio_dec_out_frame_t frame = {
          .buffer = outputBuffer,
          .len = PLAY_OUTPUT_BUFFER_BYTES,
          .needed_size = 0,
          .decoded_size = 0,
      };

      esp_audio_err_t err = esp_mp3_dec_decode(decoder, &raw, &frame, &info);
      if (err == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
        uint8_t *resizedBuffer = (uint8_t *)realloc(outputBuffer, frame.needed_size);
        if (!resizedBuffer) {
          AUDIO_LOGLN("playMp3File failed: realloc failed");
          ok = false;
          _playing = false;
          break;
        }
        outputBuffer = resizedBuffer;
        continue;
      }
      if (err != ESP_AUDIO_ERR_OK) {
        AUDIO_LOG("playMp3File decode failed: err=%d raw.len=%lu consumed=%lu\n",
                  (int)err,
                  (unsigned long)raw.len,
                  (unsigned long)raw.consumed);
        ok = false;
        _playing = false;
        break;
      }

      raw.buffer += raw.consumed;
      raw.len -= raw.consumed;

      if (frame.decoded_size > 0) {
        if (!codecOpened) {
          uint8_t bitsPerSample = info.bits_per_sample;
          uint8_t channels = info.channel ? info.channel : 2;
          if ((bitsPerSample == 1 || bitsPerSample == 2) &&
              (channels == 8 || channels == 16 || channels == 24 ||
               channels == 32)) {
            uint8_t swapped = bitsPerSample;
            bitsPerSample = channels;
            channels = swapped;
          }
          AUDIO_LOG("playMp3File decoded info: sr=%lu bits=%u ch=%u\n",
                    (unsigned long)info.sample_rate,
                    bitsPerSample,
                    channels);
          AUDIO_LOGLN("playMp3File openCodec before");
          if (!openCodec(info.sample_rate, bitsPerSample, channels, false,
                         true)) {
            AUDIO_LOGLN("playMp3File failed: openCodec failed");
            ok = false;
            _playing = false;
            break;
          }
          AUDIO_LOGLN("playMp3File openCodec after");
          codecOpened = true;
        }

        if (!firstWriteLogged) {
          AUDIO_LOG("playMp3File first write before: decoded_size=%lu\n",
                    (unsigned long)frame.decoded_size);
        }
        if (esp_codec_dev_write(_codecDev, outputBuffer, frame.decoded_size) !=
            ESP_CODEC_DEV_OK) {
          AUDIO_LOG("playMp3File write failed: decoded_size=%lu\n",
                    (unsigned long)frame.decoded_size);
          ok = false;
          _playing = false;
          break;
        }
        if (!firstWriteLogged) {
          AUDIO_LOGLN("playMp3File first write after");
          firstWriteLogged = true;
        }
      }
    }
  }

  AUDIO_LOGLN("playMp3File exit");
  free(inputBuffer);
  free(outputBuffer);
  esp_mp3_dec_close(decoder);
  closeCodec();
  _playing = false;
  return ok;
}

bool Audio::playFile(const char *path) {
  AUDIO_LOG("playFile(path=%s)\n", path ? path : "<null>");

  fs::FS *fs = &LittleFS;
  const char *label = "lfs2";
  if (!mountFS(fs, label)) {
    AUDIO_LOGLN("playFile failed: mountFS failed");
    return false;
  }

  auto type = getDecoderType(path);
  if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) {
    AUDIO_LOGLN("playFile failed: unsupported decoder type");
    return false;
  }

  File file = fs->open(path, FILE_READ);
  if (!file) {
    AUDIO_LOGLN("playFile failed: open file failed");
    return false;
  }

  AUDIO_LOG("playFile size=%lu type=%lu\n", (unsigned long)file.size(),
            (unsigned long)type);

  bool ok = false;
  if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_WAV) {
    AUDIO_LOGLN("playFile using direct WAV playback path");
    ok = playWavFile(file);
  } else if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_MP3) {
    AUDIO_LOGLN("playFile using direct MP3 playback path");
    ok = playMp3File(file);
  } else {
    AUDIO_LOGLN("playFile using simple decoder path");
    ok = playStream(file, type);
  }

  AUDIO_LOG("playFile result=%d\n", ok);
  AUDIO_LOGLN("playFile closing file");
  file.close();
  AUDIO_LOGLN("playFile file closed");
  return ok;
}

bool Audio::playUrl(const char *url) {
  (void)url;
  return false;
}

void Audio::stopPlay() {
  if (_playing) {
    _playing = false;
    closeCodec();
  }
}

bool Audio::playing() const { return _playing; }
