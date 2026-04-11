#ifndef __RGB_H
#define __RGB_H

#include "Arduino.h"
#include "mPython_WS2812.h"

class RGB {
public:
    RGB(uint8_t pin = 8, uint16_t numPixels = 3);
    void begin();
    void write(int8_t index, uint8_t r, uint8_t g, uint8_t b);
    void write(int8_t index, uint32_t color){write(index, 0xff&(color>>16), 0xff&(color>>8), 0xff&color);}
    void brightness(uint8_t b);
    uint8_t brightness();

private:
    mPython_WS2812 _ws2812;
    int _brightness;
    uint16_t _numPixels;
    uint8_t _pin;
};

#endif