#ifndef __TOUCHPAD_H
#define __TOUCHPAD_H

#include "Arduino.h"

/**
 * @brief 回调函数类型
 *
 * 触摸板事件的回调函数类型
 */
typedef void (*CBFunc)(void); 

/**
 * @brief 触摸板类
 *
 * 处理触摸板的状态检测和回调
 */
class TouchPad {
public:
    uint16_t threshold = 50000; /*!< 触摸阈值 */
    
    /**
     * @brief 构造函数
     *
     * 初始化触摸板实例
     *
     * @param _io 触摸板引脚
     */
    TouchPad(uint8_t _io);
    
    /**
     * @brief 检查是否触摸
     *
     * @return bool 触摸状态
     */
    bool isTouched(void);
    
    /**
     * @brief 设置触摸回调函数
     *
     * @param _cb 触摸回调函数
     */
    void setTouchedCallback(CBFunc _cb);
    
    /**
     * @brief 设置未触摸回调函数
     *
     * @param _cb 未触摸回调函数
     */
    void setUnTouchedCallback(CBFunc _cb);
    
    /**
     * @brief 读取触摸值
     *
     * @return uint32_t 触摸值
     */
    uint32_t read(void);
    
    CBFunc touchedCb;     /*!< 触摸回调函数 */
    CBFunc untouchedCb;   /*!< 未触摸回调函数 */
    bool touchDetected;    /*!< 触摸检测标志位 */
    uint8_t io;           /*!< 触摸板引脚 */

private:
    int valuePrevPrev,valuePrev,value;
    bool taskCreated; // 任务是否已创建
};

#endif