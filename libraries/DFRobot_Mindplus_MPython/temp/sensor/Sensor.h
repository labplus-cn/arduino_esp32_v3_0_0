#ifndef __SENSOR_H
#define __SENSOR_H

#include "Arduino.h"
#include <Wire.h>
#include "mPython_QMI8658.h"
#include "mPython_MMC56x3.h"
#include "mPython_LTR308.h"

typedef void (*mpythonGestureHandlePtr)(void);

class Accelerometer {
public:
    enum Gesture {
        Shake = 0,
        ScreenUp = 1,
        ScreenDown = 2,
        TiltLeft = 3,
        TiltRight = 4,
        TiltForward = 5,
        TiltBack = 6,
        GestureNone = 7
    };

    Accelerometer();
    void init(void);
    float getX(void);
    float getY(void);
    float getZ(void);
    void setOffset(int x, int y, int z);
    float getStrength(void);
    void onGesture(Gesture gesture, mpythonGestureHandlePtr body);
    bool isGesture(Gesture gesture);

private:
    mPython_QMI8658 _qmi8658;
    float rawX, rawY, rawZ;
    int offsetX, offsetY, offsetZ;
    mpythonGestureHandlePtr GestureHandle[7];
    Gesture currentGesture;
    bool isGestureEnable;
    bool _is_shaked;
    static void taskLoop(void *param);
};

class Magnetometer {
public:
    Magnetometer();
    void init(void);
    float getX(void);
    float getY(void);
    float getZ(void);
    void setOffset(int x, int y, int z);

private:
    mPython_MMC5603 _mmc5603;
    int offsetX, offsetY, offsetZ;
    float rawX, rawY, rawZ;
    void readData(void);
};

class LightSensor {
public:
    LightSensor();
    void init(void);
    uint16_t getLux(void);

private:
    mPython_LTR308 _ltr308;
    uint16_t readLux(void);
};

#endif