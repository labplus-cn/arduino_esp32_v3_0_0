#ifndef __BUTTON_H
#define __BUTTON_H

#include "Arduino.h"

// 回调函数类型
typedef void (*CBFunc)(void);

class Button {
public:
    Button(uint8_t _io);
    Button(uint8_t _io1, uint8_t _io2);
    bool isPressed(void);
    void setPressedCallback(CBFunc _cb);
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