#ifndef __MPYTHON_H
#define __MPYTHON_H

#include "Arduino.h"
#include <Wire.h>
#include "display/DisplayFB.h"
#include "rgb/RGB.h"
#include "buzz/Buzz.h"
#include "audio/Audio.h"
#include "button/Button.h"
#include "touch/TouchPad.h"
#include "fs/USBDISK.h"
#include "wifi/WiFi.h"

class MPython {
public:
    MPython();
    void begin(void);

    DisplayFB display;
    RGB rgb;
    Buzz buzz;
    Audio audio;
    Button buttonA;
    Button buttonB;
    Button buttonAB;
    TouchPad touchPadP;
    TouchPad touchPadY;
    TouchPad touchPadT;
    TouchPad touchPadH;
    TouchPad touchPadO;
    TouchPad touchPadN;
    USBDISK usbDisk;
    MPythonWiFi wifi;
};

extern MPython mPython;

#endif
