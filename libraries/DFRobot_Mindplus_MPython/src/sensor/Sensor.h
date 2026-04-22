#ifndef __SENSOR_H
#define __SENSOR_H

#include "Arduino.h"
#include <Wire.h>
#include "mPython_QMI8658.h"
#include "mPython_MMC56x3.h"
#include "mPython_LTR308.h"

/**
 * @brief 手势回调函数类型
 *
 * 加速度计手势事件的回调函数类型
 */
typedef void (*mpythonGestureHandlePtr)(void);

/**
 * @brief 加速度计类
 *
 * 提供加速度数据读取和手势检测功能
 */
class Accelerometer {
public:
    /**
     * @brief 手势枚举
     *
     * 定义支持的手势类型
     */
    enum Gesture {
        Shake = 0,       /*!< 摇晃 */
        ScreenUp = 1,    /*!< 屏幕朝上 */
        ScreenDown = 2,  /*!< 屏幕朝下 */
        TiltLeft = 3,    /*!< 向左倾斜 */
        TiltRight = 4,   /*!< 向右倾斜 */
        TiltForward = 5, /*!< 向前倾斜 */
        TiltBack = 6,    /*!< 向后倾斜 */
        GestureNone = 7  /*!< 无手势 */
    };

    /**
     * @brief 构造函数
     *
     * 初始化加速度计实例
     */
    Accelerometer();
    
    /**
     * @brief 初始化函数
     *
     * 初始化加速度计硬件
     */
    void begin(void);
    
    /**
     * @brief 获取 X 轴加速度
     *
     * @return float X 轴加速度值
     */
    float getX(void);
    
    /**
     * @brief 获取 Y 轴加速度
     *
     * @return float Y 轴加速度值
     */
    float getY(void);
    
    /**
     * @brief 获取 Z 轴加速度
     *
     * @return float Z 轴加速度值
     */
    float getZ(void);
    
    /**
     * @brief 设置偏移
     *
     * @param x X 轴偏移
     * @param y Y 轴偏移
     * @param z Z 轴偏移
     */
    void setOffset(int x, int y, int z);
    
    /**
     * @brief 获取加速度强度
     *
     * @return float 加速度强度
     */
    float getStrength(void);
    
    /**
     * @brief 设置手势回调
     *
     * @param gesture 手势类型
     * @param body 回调函数
     */
    void onGesture(Gesture gesture, mpythonGestureHandlePtr body);
    
    /**
     * @brief 检查是否为指定手势
     *
     * @param gesture 手势类型
     * @return bool 是否为指定手势
     */
    bool isGesture(Gesture gesture);

private:
    mPython_QMI8658 _qmi8658;
    float rawX, rawY, rawZ;
    int offsetX, offsetY, offsetZ;
    mpythonGestureHandlePtr GestureHandle[7];
    Gesture currentGesture;
    bool isGestureEnable;
    bool _is_shaked;
    static void taskLoop(void *param);
};

/**
 * @brief 磁力计类
 *
 * 提供磁场数据读取功能
 */
class Magnetometer {
public:
    /**
     * @brief 构造函数
     *
     * 初始化磁力计实例
     */
    Magnetometer();
    
    /**
     * @brief 初始化函数
     *
     * 初始化磁力计硬件
     */
    void begin(void);
    
    /**
     * @brief 获取 X 轴磁场
     *
     * @return float X 轴磁场值
     */
    float getX(void);
    
    /**
     * @brief 获取 Y 轴磁场
     *
     * @return float Y 轴磁场值
     */
    float getY(void);
    
    /**
     * @brief 获取 Z 轴磁场
     *
     * @return float Z 轴磁场值
     */
    float getZ(void);
    
    /**
     * @brief 设置偏移
     *
     * @param x X 轴偏移
     * @param y Y 轴偏移
     * @param z Z 轴偏移
     */
    void setOffset(int x, int y, int z);

private:
    mPython_MMC5603 _mmc5603;
    int offsetX, offsetY, offsetZ;
    float rawX, rawY, rawZ;
    void readData(void);
};

/**
 * @brief 光线传感器类
 *
 * 提供环境光强度读取功能
 */
class LightSensor {
public:
    /**
     * @brief 构造函数
     *
     * 初始化光线传感器实例
     */
    LightSensor();
    
    /**
     * @brief 初始化函数
     *
     * 初始化光线传感器硬件
     */
    void begin(void);
    
    /**
     * @brief 获取光照强度
     *
     * @return uint16_t 光照强度值（单位：lux）
     */
    uint16_t getLux(void);

private:
    mPython_LTR308 _ltr308;
    uint16_t readLux(void);
};

#endif