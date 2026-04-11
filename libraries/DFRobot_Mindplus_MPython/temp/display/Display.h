#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "Arduino.h"

class Display {
public:
    Display();
    void begin(uint8_t address = 0x3c);
    void clearDisplay();
    void setCursor(uint8_t x, uint8_t y);
    void print(String text);
    void display();
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

private:
};

#endif