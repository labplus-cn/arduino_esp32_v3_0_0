#ifndef __BUTTON_H
#define __BUTTON_H

#include "Arduino.h"

/**
 * @brief 回调函数类型
 *
 * 按钮事件的回调函数类型
 */
typedef void (*CBFunc)(void);

/**
 * @brief 按钮类
 *
 * 处理按钮的状态检测和回调
 */
class Button {
public:
    /**
     * @brief 构造函数（单按钮）
     *
     * 初始化单按钮实例
     *
     * @param _io 按钮引脚
     */
    Button(uint8_t _io);
    
    /**
     * @brief 构造函数（双按钮）
     *
     * 初始化双按钮实例
     *
     * @param _io1 第一个按钮引脚
     * @param _io2 第二个按钮引脚
     */
    Button(uint8_t _io1, uint8_t _io2);
    
    /**
     * @brief 检查按钮是否按下
     *
     * @return bool 按钮状态
     */
    bool isPressed(void);
    
    /**
     * @brief 设置按下回调函数
     *
     * @param _cb 按下回调函数
     */
    void setPressedCallback(CBFunc _cb);
    
    /**
     * @brief 设置释放回调函数
     *
     * @param _cb 释放回调函数
     */
    void setUnPressedCallback(CBFunc _cb);

private:
    CBFunc pressedCb;
    CBFunc unpressedCb;
    uint8_t io;
    uint8_t io1;
    uint8_t io2;
    uint8_t pinNum;
    bool prevState;
    
    static void taskLoop(void *param);
};

#endif