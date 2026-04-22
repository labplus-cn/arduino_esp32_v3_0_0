#ifndef __AUDIO_H
#define __AUDIO_H

#include "Arduino.h"
#include "FS.h"
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <freertos/event_groups.h>
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

/**
 * @brief 音频类
 *
 * 提供音频播放、录音、语音合成和语音识别功能
 */
class Audio {
public:
    /**
     * @brief 播放状态枚举
     *
     * 定义音频播放的不同状态
     */
    enum PlayState : uint8_t {
        PLAY_STATE_IDLE = 0,    /*!< 空闲状态 */
        PLAY_STATE_PLAYING,     /*!< 播放中 */
        PLAY_STATE_PAUSED,      /*!< 暂停 */
        PLAY_STATE_STOPPED,     /*!< 已停止 */
        PLAY_STATE_ERROR,       /*!< 错误 */
    };

    /**
     * @brief 构造函数
     *
     * 初始化 Audio 实例
     */
    Audio();
    
    /**
     * @brief 析构函数
     *
     * 清理 Audio 实例
     */
    ~Audio();

    /**
     * @brief 初始化函数
     *
     * 初始化音频硬件
     *
     * @return bool 初始化结果
     */
    bool begin();

    /**
     * @brief 设置音量
     *
     * @param volume 音量值 (0-100)
     * @return bool 设置结果
     */
    bool setVolume(uint8_t volume);

    /**
     * @brief 开始录音
     *
     * @param path 录音文件路径
     * @param sampleRate 采样率
     * @param bitsPerSample 位宽
     * @param channels 声道数
     * @return bool 操作结果
     */
    bool startRecord(const char *path,
                     uint32_t sampleRate = 16000,
                     uint8_t bitsPerSample = 16,
                     uint8_t channels = 1);
    
    /**
     * @brief 开始录音（指定时长）
     *
     * @param path 录音文件路径
     * @param sampleRate 采样率
     * @param bitsPerSample 位宽
     * @param channels 声道数
     * @param durationMs 录音时长（毫秒）
     * @return bool 操作结果
     */
    bool startRecord(const char *path,
                     uint32_t sampleRate,
                     uint8_t bitsPerSample,
                     uint8_t channels,
                     uint32_t durationMs);
    
    /**
     * @brief 停止录音
     */
    void stopRecord();

    /**
     * @brief 播放音频
     *
     * @param path 音频文件路径
     * @return bool 操作结果
     */
    bool play(const char *path);
    
    /**
     * @brief 暂停播放
     *
     * @return bool 操作结果
     */
    bool pause();
    
    /**
     * @brief 恢复播放
     *
     * @return bool 操作结果
     */
    bool resume();
    
    /**
     * @brief 停止播放
     */
    void stop();
    
    /**
     * @brief 获取播放状态
     *
     * @return PlayState 播放状态
     */
    PlayState state() const;

    /**
     * @brief 初始化语音合成
     *
     * @return bool 初始化结果
     */
    bool ttsInit();
    
    /**
     * @brief 文本转语音
     *
     * @param text 要转换的文本
     * @return bool 操作结果
     */
    bool textToSpeech(const char *text);

    /**
     * @brief 初始化语音识别
     *
     * 离线语音命令识别（WakeNet + MultiNet，esp-sr）。
     * 需要 flash 上存在 label 为 "model" 的分区及对应模型；勿与 startRecord() 同时使用。
     *
     * @param wakeupReplyTts 唤醒后可选播放的 UTF-8 文本（需先 ttsInit）；nullptr 则不播
     * @param multinetTimeoutMs MultiNet 命令识别超时（毫秒）
     * @param loadCommandsFromSdkconfig 为 true 时从 sdkconfig 加载命令表（多数 Arduino 工程无此项，常为 false）
     * @return bool 操作结果
     */
    bool srBegin(const char *wakeupReplyTts = nullptr,
                                uint16_t multinetTimeoutMs = 6000,
                                bool loadCommandsFromSdkconfig = false);

    /**
     * @brief 添加语音命令
     *
     * @param commandId 命令 ID
     * @param phraseUtf8 命令短语（UTF-8）
     * @return bool 操作结果
     */
    bool srAddCommand(int commandId, const char *phraseUtf8);
    
    /**
     * @brief 清除所有语音命令
     *
     * @return bool 操作结果
     */
    bool srClearCommands();
    
    /**
     * @brief 应用语音命令
     *
     * @return bool 操作结果
     */
    bool srApplyCommands();
    
    /**
     * @brief 获取识别到的命令 ID
     *
     * @return int 命令 ID
     */
    int srGetCommandId();

private:
    void srEnd();
    static constexpr int I2C_PORT = 0;
    static constexpr int I2S_PORT = 0;
    static constexpr uint8_t DEFAULT_VOLUME = 80;

    const audio_codec_gpio_if_t *_gpioIf;
    const audio_codec_ctrl_if_t *_ctrlIf;
    const audio_codec_data_if_t *_dataIf;
    const audio_codec_if_t *_codecIf;
    esp_codec_dev_handle_t _codecDev;   // 录音+播放一体设备（IN_OUT）

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
    RingbufHandle_t _recordRingbuf;  // esp_ringbuf BYTEBUF，替代手写环形缓冲区

    bool _codecDeviceOpened;
    uint32_t _codecSampleRate;
    uint8_t _codecBitsPerSample;
    uint8_t _codecChannels;

    uint8_t _volume;
    float _micGain;

    File _recordFile;
    fs::FS *_recordFs;
    uint32_t _recordDataBytes;
    uint32_t _recordSampleRate;
    uint8_t _recordBitsPerSample;
    uint8_t _recordChannels;

    void end();
    bool setMicGain(float gain);

    bool mountFS(fs::FS *fs, const char *partitionLabel);
    bool codecCreate();
    void codecDestroy();

    bool openCodecDevice(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels);
    void closeCodecDevice();

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
    bool srWaitReady(uint32_t timeoutMs = 15000);

    // 语音合成相关（句柄类型见 esp-sr esp_tts_handle_t，此处用 void* 避免在公共头中包含 TTS 头）
    void *_ttsHandle;
    volatile bool _ttsInitialized;
    SemaphoreHandle_t _ttsSemaphore;

    // 语音合成任务
    static void ttsTask(void *arg);

    // esp-sr 语音识别（实现见 Audio.cpp，句柄均为 void* 避免在头文件中包含 esp-sr）
    void srFeedTask();
    void srDetectTask();
    static void srFeedTaskEntry(void *arg);
    static void srDetectTaskEntry(void *arg);
    bool speechRecognitionDeinitInternal();
    bool srRunning() const;

    void *_srAfeIface;
    void *_srAfeData;
    void *_srModels;
    volatile int _srTaskFlag;
    volatile int _srLatestCommandId;
    volatile int _srWakeupFlag;
    volatile bool _srWaitWakeRelease;
    volatile bool _srMultinetReady;
    void *_srMultinet;
    void *_srMnModel;
    uint16_t _srMnTimeoutMs;
    String _srWakeupReplyTts;
    bool _srLoadFromSdkconfig;
    EventGroupHandle_t _srDoneEvent;
    TaskHandle_t _srFeedTaskHandle;
    TaskHandle_t _srDetectTaskHandle;
    bool _srCommandsAllocated;
};

#endif