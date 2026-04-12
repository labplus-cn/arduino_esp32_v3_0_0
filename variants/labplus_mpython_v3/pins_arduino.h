#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

#define USB_VID 0x303a
#define USB_PID 0x1001

// Some boards have too low voltage on this pin (board design bug)
// Use different pin with 3V and connect with 48
// and change this setup for the chosen pin (for example 38)
// static const uint8_t LED_BUILTIN = SOC_GPIO_PIN_COUNT + 48;
// #define BUILTIN_LED    LED_BUILTIN  // backward compatibility
// #define LED_BUILTIN    LED_BUILTIN
#define RGB_BUILTIN    8
// #define RGB_BRIGHTNESS 30

// static const uint8_t TX = 43;
// static const uint8_t RX = 44;

// static const uint8_t TXD2 = 17;
// static const uint8_t RXD2 = 18;

static const uint8_t SDA = 44;
static const uint8_t SCL = 43;

static const uint8_t SS = 34;
static const uint8_t MOSI = 37;
static const uint8_t MISO = -1;
static const uint8_t SCK = 36;

static const uint8_t P0 = 1;
static const uint8_t P1 = 2;
static const uint8_t P2 = 3;
static const uint8_t P3 = 4;
static const uint8_t P4 = 5;
static const uint8_t P5 = 0;      // BUTTON A
static const uint8_t P6 = 7; 
static const uint8_t P7 = 8;      //RGG LED
static const uint8_t P8 = 15;
static const uint8_t P9 = 16;
static const uint8_t P10 = 6;     //SOUND
static const uint8_t P11 = 46;    //BUTTON B
static const uint8_t P12 = 21;    //BUZZER
static const uint8_t P13 = 17;
static const uint8_t P14 = 18;
static const uint8_t P15 = 48;
static const uint8_t P16 = 47;
static const uint8_t P19 = 43;
static const uint8_t P20 = 44;

static const uint8_t P23 = 9;
static const uint8_t P24 = 10;
static const uint8_t P25 = 11;
static const uint8_t P26 = 12;
static const uint8_t P27 = 13;
static const uint8_t P28 = 14;
static const uint8_t P = 9;
static const uint8_t Y = 10;
static const uint8_t T = 11;
static const uint8_t H = 12;
static const uint8_t O = 13;
static const uint8_t N = 14;

static const uint8_t A0 = 1;
static const uint8_t A1 = 2;
static const uint8_t A2 = 3;
static const uint8_t A3 = 4;
static const uint8_t A4 = 5;
static const uint8_t A5 = 6;
static const uint8_t A6 = 7;
static const uint8_t A7 = 8;

#endif /* Pins_Arduino_h */
