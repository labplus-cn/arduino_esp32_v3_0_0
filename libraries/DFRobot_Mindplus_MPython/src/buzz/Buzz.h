#ifndef __BUZZ_H
#define __BUZZ_H

#include "Arduino.h"
#include "driver/ledc.h"

/**
 * @brief 节拍枚举
 *
 * 定义不同的节拍值
 */
enum Beat {
    BEAT_1 = 4,    /*!< 1拍 */
    BEAT_1_2 = 2,  /*!< 1/2拍 */
    BEAT_1_4 = 1,  /*!< 1/4拍 */
    BEAT_3_4 = 3,  /*!< 3/4拍 */
    BEAT_3_2 = 6,  /*!< 3/2拍 */
    BEAT_2 = 8,    /*!< 2拍 */
    BEAT_3 = 12,   /*!< 3拍 */
    BEAT_4 = 16    /*!< 4拍 */
};

/**
 * @brief 音乐枚举
 *
 * 定义内置的音乐曲目
 */
enum Music {
    DADADADUM = 0,     /*!< 贝多芬第五交响曲开头 */
    ENTERTAINER,        /*!< 艺人 */
    PRELUDE,           /*!< 前奏曲 */
    ODE,               /*!< 颂歌 */
    NYAN,              /*!< 彩虹猫 */
    RINGTONE,          /*!< 铃声 */
    FUNK,              /*!< 放克 */
    BLUES,             /*!< 蓝调 */
    BIRTHDAY,          /*!< 生日快乐 */
    WEDDING,           /*!< 婚礼进行曲 */
    FUNERAL,           /*!< 葬礼进行曲 */
    PUNCHLINE,         /*!< 笑点 */
    BADDY,             /*!< 反派 */
    CHASE,             /*!< 追逐 */
    BA_DING,           /*!< 叮 */
    WAWAWAWAA,         /*!< 哇哇哇哇 */
    JUMP_UP,           /*!< 跳上 */
    JUMP_DOWN,         /*!< 跳下 */
    POWER_UP,          /*!<  power up */
    POWER_DOWN         /*!<  power down */
};

/**
 * @brief 播放模式枚举
 *
 * 定义不同的播放模式
 */
enum MelodyOptions {
    Once = 1,                /*!< 播放一次 */
    Forever = 2,             /*!< 循环播放 */
    OnceInBackground = 4,     /*!< 在后台播放一次 */
    ForeverInBackground = 8   /*!< 在后台循环播放 */
};

/**
 * @brief 蜂鸣器类
 *
 * 控制蜂鸣器发声，包括播放音调、音乐等功能
 */
class Buzz {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 Buzz 实例
     */
    Buzz();
    
    /**
     * @brief 初始化函数
     *
     * 初始化蜂鸣器硬件
     *
     * @param pin 蜂鸣器引脚
     */
    void begin(uint8_t pin = 21);
    
    /**
     * @brief 打开蜂鸣器
     *
     * 启动蜂鸣器
     */
    void on(void);
    
    /**
     * @brief 关闭蜂鸣器
     *
     * 停止蜂鸣器发声
     */
    void off(void);
    
    /**
     * @brief 检查蜂鸣器是否开启
     *
     * @return bool 蜂鸣器状态
     */
    bool isOn(void);
    
    /**
     * @brief 设置节拍和 tempo
     *
     * @param _ticks 节拍数
     * @param _tempo 速度
     */
    void setTicksTempo(uint32_t _ticks, uint32_t _tempo);
    
    /**
     * @brief 设置频率
     *
     * @param _freq 频率值
     */
    void freq(uint32_t _freq = 500);
    
    /**
     * @brief 设置频率和节拍
     *
     * @param _freq 频率值
     * @param beat 节拍
     */
    void freq(uint32_t _freq, Beat beat);
    
    /**
     * @brief 停止播放
     *
     * 停止当前播放的音乐或音调
     */
    void stop();
    
    /**
     * @brief 播放音乐
     *
     * @param music 音乐曲目
     * @param options 播放选项
     */
    void play(Music music, MelodyOptions options = Once);

private:
    int16_t  _pin;
    uint16_t _freq;
    bool     _on;
    bool     _playing;
    float ticks;
    float tempo;

    struct buzzMelody_t {
        String melody;
        int currentDuration;
        int currentOctave;
        int beatsPerMinute;
        MelodyOptions opt;
        TaskHandle_t task;
    } buzzMelody;

    void     playNextNote(String note);
    const char* const getMelody(Music music);
    static void taskLoop(void *param);
};

#endif