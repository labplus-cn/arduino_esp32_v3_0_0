#include "audio/Audio.h"
#include <LittleFS.h>
#include <Wire.h>
#include "driver/i2s_std.h"
#include "audio/esp_codec_dev/include/esp_codec_dev.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_defaults.h"
#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/device/include/es8388_codec.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"

extern "C" {
int audio_codec_delete_codec_if(const audio_codec_if_t *codec_if);
int audio_codec_delete_ctrl_if(const audio_codec_ctrl_if_t *h);
int audio_codec_delete_data_if(const audio_codec_data_if_t *h);
int audio_codec_delete_gpio_if(const audio_codec_gpio_if_t *gpio_if);
void esp_codec_dev_delete(esp_codec_dev_handle_t handle);
}

namespace {
constexpr gpio_num_t SDA_PIN = GPIO_NUM_44, SCL_PIN = GPIO_NUM_43;
constexpr gpio_num_t MCLK_PIN = GPIO_NUM_39, BCLK_PIN = GPIO_NUM_41;
constexpr gpio_num_t WS_PIN = GPIO_NUM_42, DOUT_PIN = GPIO_NUM_38, DIN_PIN = GPIO_NUM_40;
constexpr size_t RECORD_BUFFER_BYTES = 2048;
static bool wr16(File &f, uint16_t v){ uint8_t b[2]={uint8_t(v),uint8_t(v>>8)}; return f.write(b,2)==2; }
static bool wr32(File &f, uint32_t v){ uint8_t b[4]={uint8_t(v),uint8_t(v>>8),uint8_t(v>>16),uint8_t(v>>24)}; return f.write(b,4)==4; }
}

Audio::Audio() : _gpioIf(nullptr), _ctrlIf(nullptr), _dataIf(nullptr), _codecIf(nullptr), _codecDev(nullptr), _begun(false), _recording(false), _playing(false), _decoderRegistered(false), _simpleDecoderRegistered(false), _volume(DEFAULT_VOLUME), _micGain(35.0f), _recordFs(nullptr), _recordDataBytes(0), _recordSampleRate(16000), _recordBitsPerSample(16), _recordChannels(1) {}
Audio::~Audio(){ end(); }

bool Audio::mountFS(fs::FS *fs, const char *label){ return fs && (fs != &LittleFS || LittleFS.begin(true, "/littlefs", 5, label)); }
bool Audio::initI2C(){ Wire.begin(SDA_PIN, SCL_PIN); Wire.setClock(100000); return true; }

bool Audio::initI2S(){
  i2s_chan_config_t cc = I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)I2S_PORT, I2S_ROLE_MASTER); cc.auto_clear = true;
  i2s_chan_handle_t tx = nullptr, rx = nullptr;
  if (i2s_new_channel(&cc, &tx, &rx) != ESP_OK) return false;
  i2s_std_config_t sc = { .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO), .gpio_cfg = { .mclk = MCLK_PIN, .bclk = BCLK_PIN, .ws = WS_PIN, .dout = DOUT_PIN, .din = DIN_PIN, .invert_flags = { false, false, false } } };
  if (i2s_channel_init_std_mode(tx, &sc) != ESP_OK || i2s_channel_init_std_mode(rx, &sc) != ESP_OK) { i2s_del_channel(tx); i2s_del_channel(rx); return false; }
  audio_codec_i2s_cfg_t cfg = { .port = I2S_PORT, .rx_handle = rx, .tx_handle = tx, .clk_src = I2S_CLK_SRC_DEFAULT };
  _dataIf = audio_codec_new_i2s_data(&cfg); return _dataIf != nullptr;
}

bool Audio::initCodec(){
  audio_codec_i2c_cfg_t i2c = { .port = I2C_PORT, .addr = ES8388_CODEC_DEFAULT_ADDR, .bus_handle = nullptr };
  _ctrlIf = audio_codec_new_i2c_ctrl(&i2c); if (!_ctrlIf) return false;
  _gpioIf = audio_codec_new_gpio(); if (!_gpioIf) return false;
  es8388_codec_cfg_t cfg = { .ctrl_if = _ctrlIf, .gpio_if = _gpioIf, .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH, .master_mode = false, .pa_pin = -1, .pa_reverted = false, .hw_gain = {} };
  _codecIf = es8388_codec_new(&cfg); if (!_codecIf) return false;
  esp_codec_dev_cfg_t dev = { .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT, .codec_if = _codecIf, .data_if = _dataIf };
  _codecDev = esp_codec_dev_new(&dev); return _codecDev != nullptr;
}

void Audio::deinitCodec(){
  closeCodec();
  if (_codecDev) { esp_codec_dev_delete(_codecDev); _codecDev = nullptr; }
  if (_codecIf) { audio_codec_delete_codec_if(_codecIf); _codecIf = nullptr; }
  if (_ctrlIf) { audio_codec_delete_ctrl_if(_ctrlIf); _ctrlIf = nullptr; }
  if (_dataIf) { audio_codec_delete_data_if(_dataIf); _dataIf = nullptr; }
  if (_gpioIf) { audio_codec_delete_gpio_if(_gpioIf); _gpioIf = nullptr; }
}

bool Audio::begin(){
  if (_begun) return true;
  if (!initI2C() || !initI2S() || !initCodec()) { deinitCodec(); return false; }
  _decoderRegistered = esp_audio_dec_register_default() == ESP_AUDIO_ERR_OK;
  _simpleDecoderRegistered = esp_audio_simple_dec_register_default() == ESP_AUDIO_ERR_OK;
  _begun = true; setVolume(_volume); setMicGain(_micGain); return true;
}

void Audio::end(){
  stopPlay(); stopRecord();
  if (_simpleDecoderRegistered) { esp_audio_simple_dec_unregister_default(); _simpleDecoderRegistered = false; }
  if (_decoderRegistered) { esp_audio_dec_unregister_default(); _decoderRegistered = false; }
  deinitCodec(); _begun = false;
}

bool Audio::openCodec(uint32_t sr, uint8_t bits, uint8_t ch, bool in, bool out){
  if (!_codecDev) return false;
  esp_codec_dev_sample_info_t s = { .bits_per_sample = bits, .channel = uint8_t(ch == 1 ? 2 : ch), .channel_mask = uint16_t(ch == 1 ? ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) : 0), .sample_rate = sr, .mclk_multiple = 256 };
  if (esp_codec_dev_open(_codecDev, &s) != ESP_CODEC_DEV_OK) return false;
  if (out) esp_codec_dev_set_out_vol(_codecDev, _volume);
  if (in) esp_codec_dev_set_in_gain(_codecDev, _micGain);
  return true;
}

void Audio::closeCodec(){ if (_codecDev) esp_codec_dev_close(_codecDev); }
bool Audio::setVolume(uint8_t v){ _volume = v > 100 ? 100 : v; return !_codecDev || esp_codec_dev_set_out_vol(_codecDev, _volume) == ESP_CODEC_DEV_OK; }
bool Audio::setMicGain(float g){ _micGain = g; return !_codecDev || esp_codec_dev_set_in_gain(_codecDev, _micGain) == ESP_CODEC_DEV_OK; }

bool Audio::writeWavHeader(File &f, uint32_t ds, uint32_t sr, uint8_t bits, uint8_t ch){
  if (!f.seek(0)) return false; uint32_t br = sr * ch * (bits / 8); uint16_t ba = ch * (bits / 8);
  f.write((const uint8_t*)"RIFF",4); if(!wr32(f,36+ds)) return false; f.write((const uint8_t*)"WAVEfmt ",8);
  if(!wr32(f,16)||!wr16(f,1)||!wr16(f,ch)||!wr32(f,sr)||!wr32(f,br)||!wr16(f,ba)||!wr16(f,bits)) return false;
  f.write((const uint8_t*)"data",4); return wr32(f,ds);
}

bool Audio::finalizeWavHeader(){ return _recordFile && writeWavHeader(_recordFile, _recordDataBytes, _recordSampleRate, _recordBitsPerSample, _recordChannels); }

bool Audio::startRecord(const char *path, uint32_t sr, uint8_t bits, uint8_t ch, fs::FS *fs, const char *label){
  if ((!_begun && !begin()) || !mountFS(fs, label)) return false; stopRecord();
  if (!openCodec(sr, bits, ch, true, false)) return false;
  _recordFile = fs->open(path, FILE_WRITE, true); if (!_recordFile) { closeCodec(); return false; }
  _recordFs = fs; _recordSampleRate = sr; _recordBitsPerSample = bits; _recordChannels = ch; _recordDataBytes = 0;
  _recording = writeWavHeader(_recordFile, 0, sr, bits, ch); return _recording;
}

size_t Audio::recordChunk(size_t maxBytes){
  if (!_recording || !_recordFile || !_codecDev) return 0;
  size_t bytesToRead = maxBytes == 0 ? RECORD_BUFFER_BYTES : maxBytes;
  uint8_t *buffer = (uint8_t*)malloc(bytesToRead);
  if (!buffer) return 0;
  int readBytes = esp_codec_dev_read(_codecDev, buffer, (int)bytesToRead);
  if (readBytes > 0) {
    size_t written = _recordFile.write(buffer, (size_t)readBytes);
    if (written == (size_t)readBytes) {
      _recordDataBytes += (uint32_t)written;
    } else {
      _recording = false;
    }
  }
  free(buffer);
  return readBytes > 0 ? (size_t)readBytes : 0;
}

void Audio::stopRecord(){ if (_recording) { finalizeWavHeader(); _recordFile.flush(); _recordFile.close(); closeCodec(); _recording = false; } }
bool Audio::recording() const{ return _recording; }

esp_audio_simple_dec_type_t Audio::getDecoderType(const char *path){
  if (!path) return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE; const char *e = strrchr(path, '.'); if (!e) return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE; ++e;
  if (!strcasecmp(e, "wav")) return ESP_AUDIO_SIMPLE_DEC_TYPE_WAV; if (!strcasecmp(e, "mp3")) return ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
  if (!strcasecmp(e, "aac")) return ESP_AUDIO_SIMPLE_DEC_TYPE_AAC; if (!strcasecmp(e, "flac")) return ESP_AUDIO_SIMPLE_DEC_TYPE_FLAC;
  if (!strcasecmp(e, "m4a") || !strcasecmp(e, "mp4")) return ESP_AUDIO_SIMPLE_DEC_TYPE_M4A; return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
}

bool Audio::playStream(Stream &stream, esp_audio_simple_dec_type_t type){
  if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE || (!_begun && !begin())) return false; stopPlay();
  esp_audio_simple_dec_cfg_t cfg = { .dec_type = type, .dec_cfg = nullptr, .cfg_size = 0 }; esp_audio_simple_dec_handle_t dec = nullptr;
  if (esp_audio_simple_dec_open(&cfg, &dec) != ESP_AUDIO_ERR_OK) return false;
  uint8_t *in = (uint8_t*)malloc(2048), *out = (uint8_t*)malloc(8192); if (!in || !out) { free(in); free(out); esp_audio_simple_dec_close(dec); return false; }
  esp_audio_simple_dec_info_t info = {}; bool codecOpened = false, ok = true; _playing = true;
  while (_playing) {
    size_t n = stream.readBytes((char*)in, 2048); if (!n) break;
    esp_audio_simple_dec_raw_t raw = { .buffer = in, .len = (uint32_t)n, .eos = n < 2048, .consumed = 0, .frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE };
    while (raw.len && _playing) {
      esp_audio_simple_dec_out_t frame = { .buffer = out, .len = 8192, .needed_size = 0, .decoded_size = 0 };
      esp_audio_err_t err = esp_audio_simple_dec_process(dec, &raw, &frame);
      if (err == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) { uint8_t *b = (uint8_t*)realloc(out, frame.needed_size); if (!b) { ok = false; _playing = false; break; } out = b; continue; }
      if (err != ESP_AUDIO_ERR_OK) { ok = false; _playing = false; break; }
      raw.buffer += raw.consumed; raw.len -= raw.consumed;
      if (frame.decoded_size > 0) {
        if (!codecOpened) { if (esp_audio_simple_dec_get_info(dec, &info) != ESP_AUDIO_ERR_OK) continue; uint8_t ch = info.channel ? info.channel : 2; if (!openCodec(info.sample_rate, info.bits_per_sample, ch, false, true)) { ok = false; _playing = false; break; } codecOpened = true; }
        if (esp_codec_dev_write(_codecDev, out, frame.decoded_size) != ESP_CODEC_DEV_OK) { ok = false; _playing = false; break; }
      }
    }
  }
  free(in); free(out); esp_audio_simple_dec_close(dec); closeCodec(); _playing = false; return ok;
}

bool Audio::playFile(const char *path, fs::FS *fs, const char *label){ if(!mountFS(fs,label)) return false; auto t=getDecoderType(path); if(t==ESP_AUDIO_SIMPLE_DEC_TYPE_NONE) return false; File f=fs->open(path,FILE_READ); if(!f) return false; bool ok=playStream(f,t); f.close(); return ok; }

bool Audio::playUrl(const char *url){ (void)url; return false; }
void Audio::stopPlay(){ if(_playing){ _playing=false; closeCodec(); } }
bool Audio::playing() const{ return _playing; }
