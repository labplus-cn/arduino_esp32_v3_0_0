#ifndef __AUDIO_H
#define __AUDIO_H

#include "Arduino.h"
#include "FS.h"
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "audio/esp_codec_dev/include/esp_codec_dev.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_defaults.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"

class NetworkClient;

class Audio {
public:
    enum PlayState : uint8_t {
        PLAY_STATE_IDLE = 0,
        PLAY_STATE_PLAYING,
        PLAY_STATE_PAUSED,
        PLAY_STATE_STOPPED,
        PLAY_STATE_ERROR,
    };

    Audio();
    ~Audio();

    bool begin();
    void end();

    bool setVolume(uint8_t volume);
    bool setMicGain(float gainDb);

    bool startRecord(const char *path,
                     uint32_t sampleRate = 16000,
                     uint8_t bitsPerSample = 16,
                     uint8_t channels = 1);
    bool startRecord(const char *path,
                     uint32_t sampleRate,
                     uint8_t bitsPerSample,
                     uint8_t channels,
                     uint32_t durationMs);
    void stopRecord();
    bool recording() const;

    bool play(const char *path);
    bool pause();
    bool resume();
    void stop();
    PlayState state() const;
    const char *stateName() const;

private:
    static constexpr int I2C_PORT = 0;
    static constexpr int I2S_PORT = 0;
    static constexpr uint8_t DEFAULT_VOLUME = 100;

    const audio_codec_gpio_if_t *_gpioIf;
    const audio_codec_ctrl_if_t *_ctrlIf;
    const audio_codec_data_if_t *_playDataIf;  // 只含 TX handle，避免 open 时激活 RX 产生噪音
    const audio_codec_data_if_t *_recDataIf;   // 只含 RX handle
    const audio_codec_if_t *_codecIf;
    esp_codec_dev_handle_t _playDev;   // 仅用于播放（OUT）
    esp_codec_dev_handle_t _recDev;    // 仅用于录音（IN）

    bool _begun;
    bool _recording;

    volatile PlayState _playState;
    volatile bool _playStopRequested;
    volatile bool _playPauseRequested;
    TaskHandle_t _playReadTaskHandle;
    TaskHandle_t _playDecodeTaskHandle;
    void *_playContext;
    String _playSource;
    bool _playSourceIsUrl;

    volatile bool _recordStopRequested;
    volatile bool _recordCaptureDone;
    volatile bool _recordWriterDone;
    TaskHandle_t _recordCaptureTaskHandle;
    TaskHandle_t _recordWriterTaskHandle;
    uint8_t *_recordBuffer;
    size_t _recordBufferSize;
    volatile size_t _recordBufferHead;
    volatile size_t _recordBufferTail;
    volatile size_t _recordBufferCount;
    SemaphoreHandle_t _recordBufferLock;

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

    bool openPlayCodec(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels);
    void closePlayCodec();
    bool openRecCodec(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels);
    void closeRecCodec();

    bool writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels);
    bool finalizeWavHeader();

    void configureSimpleDecoder(esp_audio_simple_dec_cfg_t &cfg, uint8_t *storage, size_t storageSize) const;
    bool waitWhilePaused();
    void setPlayState(PlayState state);
    String normalizeFsPath(const char *path) const;
    bool isHttpSource(const char *path) const;
    bool initRecordBuffer(size_t bufferSize);
    void deinitRecordBuffer();
    size_t recordBufferWrite(const uint8_t *src, size_t len);
    size_t recordBufferRead(uint8_t *dst, size_t len);

    void playDecodeTask();
    void recordCaptureTask();
    void recordWriterTask();
};

#endif