#include "GPIO.h"

DigitalPin::DigitalPin(uint8_t _io) {
    io = _io;
    pinMode(io, OUTPUT);
}

void DigitalPin::mode(uint8_t mode) {
    pinMode(io, mode);
}

void DigitalPin::write(uint8_t value) {
    digitalWrite(io, value);
}

uint8_t DigitalPin::read(void) {
    return digitalRead(io);
}

void DigitalPin::toggle(void) {
    digitalWrite(io, !digitalRead(io));
}

AnalogPin::AnalogPin(uint8_t _io) {
    io = _io;
}

uint16_t AnalogPin::read(void) {
    return analogRead(io);
}

uint16_t AnalogPin::read(uint8_t bits) {
    uint16_t value = analogRead(io);
    if (bits < 12) {
        value >>= (12 - bits);
    }
    return value;
}