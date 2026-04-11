#ifndef __TOUCHPAD_H
#define __TOUCHPAD_H

#include "Arduino.h"

typedef void (*CBFunc)(void); 

class TouchPad {
public:
    uint16_t threshold = 35;
    TouchPad(uint8_t _io);
    bool isTouched(void);
    void setTouchedCallback(CBFunc _cb);
    void setUnTouchedCallback(CBFunc _cb);
    uint32_t read(void);

private:
    CBFunc touchedCb;
    CBFunc untouchedCb;
    uint8_t io;
    int valuePrevPrev,valuePrev,value;
    static void taskLoop(void *param);
};

#endif