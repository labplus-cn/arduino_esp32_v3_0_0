#ifndef __TOUCHPAD_H
#define __TOUCHPAD_H

#include "Arduino.h"

typedef void (*CBFunc)(void); 

class TouchPad {
public:
    uint16_t threshold = 50000;
    TouchPad(uint8_t _io);
    bool isTouched(void);
    void setTouchedCallback(CBFunc _cb);
    void setUnTouchedCallback(CBFunc _cb);
    uint32_t read(void);
    
    // 回调函数，用于中断处理
    CBFunc touchedCb;
    CBFunc untouchedCb;
    bool touchDetected; // 触摸检测标志位
    uint8_t io; // 触摸板引脚

private:
    int valuePrevPrev,valuePrev,value;
    bool taskCreated; // 任务是否已创建
};

#endif