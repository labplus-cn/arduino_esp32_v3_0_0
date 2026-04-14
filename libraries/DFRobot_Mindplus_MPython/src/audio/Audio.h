#ifndef __AUDIO_H
#define __AUDIO_H

#include "Arduino.h"
#include "FS.h"
#include <LittleFS.h>
#include "audio/esp_codec_dev/include/esp_codec_dev.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_defaults.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"

class Audio {
public:
    Audio();
    ~Audio();

    bool begin();
    void end();

    bool setVolume(uint8_t volume);
    bool setMicGain(float gainDb);

    bool startRecord(const char *path,
                     uint32_t sampleRate = 16000,
                     uint8_t bitsPerSample = 16,
                     uint8_t channels = 1,
                     fs::FS *fs = &LittleFS,
                     const char *partitionLabel = "lfs2");
    size_t recordChunk(size_t maxBytes = 2048);
    void stopRecord();
    bool recording() const;

    bool playFile(const char *path, fs::FS *fs = &LittleFS, const char *partitionLabel = "lfs2");
    bool playUrl(const char *url);
    void stopPlay();
    bool playing() const;

private:
    static constexpr int I2C_PORT = 0;
    static constexpr int I2S_PORT = 0;
    static constexpr uint8_t DEFAULT_VOLUME = 70;

    const audio_codec_gpio_if_t *_gpioIf;
    const audio_codec_ctrl_if_t *_ctrlIf;
    const audio_codec_data_if_t *_dataIf;
    const audio_codec_if_t *_codecIf;
    esp_codec_dev_handle_t _codecDev;

    bool _begun;
    bool _recording;
    bool _playing;
    bool _decoderRegistered;
    bool _simpleDecoderRegistered;

    uint8_t _volume;
    float _micGain;

    File _recordFile;
    fs::FS *_recordFs;
    uint32_t _recordDataBytes;
    uint32_t _recordSampleRate;
    uint8_t _recordBitsPerSample;
    uint8_t _recordChannels;

    bool mountFS(fs::FS *fs, const char *partitionLabel);
    bool initI2C();
    bool initI2S();
    bool initCodec();
    void deinitCodec();

    bool openCodec(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels, bool forInput, bool forOutput);
    void closeCodec();

    bool writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels);
    bool finalizeWavHeader();

    esp_audio_simple_dec_type_t getDecoderType(const char *path);
    bool playStream(Stream &stream, esp_audio_simple_dec_type_t type);
};

#endif
