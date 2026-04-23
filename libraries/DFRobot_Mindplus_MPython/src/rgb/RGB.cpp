#include "Arduino.h"
#include "RGB.h"

RGB::RGB(uint8_t pin, uint16_t numPixels) : _ws2812(numPixels, pin, 0, mPython_TYPE_GRB), _brightness(5), _numPixels(numPixels), _pin(pin) {
}

void RGB::begin() {
    _ws2812.begin();
    _ws2812.setBrightness((uint8_t)map(_brightness, 0, 9, 0, 64));
}

void RGB::write(int8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= 0 && index < _numPixels) {
        _ws2812.setLedColor(index, r, g, b);
    } else {
        _ws2812.setAllLedsColor(r, g, b);
    }
}

void RGB::brightness(uint8_t b) {
    if (_brightness == b) {
        return;
    }
    _brightness = max(min(b, (uint8_t)9), (uint8_t)0);
    _ws2812.setBrightness((uint8_t)map(_brightness, 0, 9, 0, 64));
}

uint8_t RGB::brightness() {
    return _brightness;
}