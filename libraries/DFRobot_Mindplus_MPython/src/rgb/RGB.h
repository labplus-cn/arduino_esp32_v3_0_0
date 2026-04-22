#ifndef __RGB_H
#define __RGB_H

#include "Arduino.h"
#include "mPython_WS2812.h"

/**
 * @brief RGB 类
 *
 * 控制 RGB LED 灯的亮度和颜色
 */
class RGB {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 RGB 实例
     *
     * @param pin RGB 引脚
     * @param numPixels LED 数量
     */
    RGB(uint8_t pin = 8, uint16_t numPixels = 3);
    
    /**
     * @brief 初始化函数
     *
     * 初始化 RGB 硬件
     */
    void begin();
    
    /**
     * @brief 设置指定索引的 LED 颜色
     *
     * @param index LED 索引
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     */
    void write(int8_t index, uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief 设置指定索引的 LED 颜色
     *
     * @param index LED 索引
     * @param color 颜色值，格式为 0xRRGGBB
     */
    void write(int8_t index, uint32_t color){write(index, 0xff&(color>>16), 0xff&(color>>8), 0xff&color);}
    
    /**
     * @brief 设置亮度
     *
     * @param b 亮度值 (0-255)
     */
    void brightness(uint8_t b);
    
    /**
     * @brief 获取当前亮度
     *
     * @return uint8_t 当前亮度值
     */
    uint8_t brightness();

private:
    mPython_WS2812 _ws2812;
    int _brightness;
    uint16_t _numPixels;
    uint8_t _pin;
};

#endif