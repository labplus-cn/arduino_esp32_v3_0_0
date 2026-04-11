#ifndef __MPYTHON_H
#define __MPYTHON_H

#include "Arduino.h"
// #include "display/Display.h"
#include "rgb/RGB.h"
// #include "buzz/Buzz.h"
// #include "audio/Audio.h"
#include "button/Button.h"
// #include "touch/TouchPad.h"
// #include "sensor/Sensor.h"
// #include "gpio/GPIO.h"

class MPython {
public:
    MPython();
    void begin(void);
    void setTouchThreshold(uint32_t threshold = 35);
    
    // 功能部件访问
    // Display display;
    RGB rgb;
    // Buzz buzz;  
    // Audio audio;
    Button buttonA;
    Button buttonB;
    Button buttonAB;
    // TouchPad touchPadP; 
    // TouchPad touchPadY; 
    // TouchPad touchPadT; 
    // TouchPad touchPadH; 
    // TouchPad touchPadO; 
    // TouchPad touchPadN; 
    // Accelerometer accelerometer;
    // Magnetometer magnetometer;
    // LightSensor lightSensor;
    // AnalogPin sound;
};

extern MPython mPython;

#endif