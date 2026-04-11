#ifndef __GPIO_H
#define __GPIO_H

#include "Arduino.h"

class DigitalPin {
public:
    DigitalPin(uint8_t _io);
    void mode(uint8_t mode);
    void write(uint8_t value);
    uint8_t read(void);
    void toggle(void);

private:
    uint8_t io;
};

class AnalogPin {
public:
    AnalogPin(uint8_t _io);
    uint16_t read(void);
    uint16_t read(uint8_t bits);

private:
    uint8_t io;
};

#endif