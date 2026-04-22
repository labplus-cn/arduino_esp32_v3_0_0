#ifndef __MPYTHON_H
#define __MPYTHON_H

#include "Arduino.h"
#include "display/DisplayFB.h"
#include "rgb/RGB.h"
#include "buzz/Buzz.h"
#include "audio/Audio.h"
#include "button/Button.h"
#include "touch/TouchPad.h"
#include "fs/USBDISK.h"
#include "wifi/WiFi.h"
#include "sensor/Sensor.h"

/**
 * @brief MPython 类
 *
 * 主控类，提供对所有硬件模块的访问
 */
class MPython {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 MPython 实例
     */
    MPython();
    
    /**
     * @brief 初始化函数
     *
     * 初始化所有硬件模块
     */
    void begin(void);

    DisplayFB display;          /*!< 显示屏对象 */
    RGB rgb;                    /*!< RGB LED 对象 */
    Buzz buzz;                  /*!< 蜂鸣器对象 */
    Audio audio;                /*!< 音频对象 */
    Button buttonA;             /*!< 按钮 A 对象 */
    Button buttonB;             /*!< 按钮 B 对象 */
    Button buttonAB;            /*!< 按钮 AB 对象 */
    TouchPad touchPadP;         /*!< 触摸板 P 对象 */
    TouchPad touchPadY;         /*!< 触摸板 Y 对象 */
    TouchPad touchPadT;         /*!< 触摸板 T 对象 */
    TouchPad touchPadH;         /*!< 触摸板 H 对象 */
    TouchPad touchPadO;         /*!< 触摸板 O 对象 */
    TouchPad touchPadN;         /*!< 触摸板 N 对象 */
    USBDISK usbDisk;            /*!< USB 磁盘对象 */
    MPythonWiFi wifi;           /*!< WiFi 对象 */
    Accelerometer accelerometer; /*!< 加速度计对象 */
    Magnetometer magnetometer;   /*!< 磁力计对象 */
    LightSensor lightSensor;     /*!< 光线传感器对象 */
};

extern MPython mPython;

#endif
