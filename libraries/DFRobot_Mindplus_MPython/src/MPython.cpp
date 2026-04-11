#include "MPython.h"

MPython::MPython() 
    : buttonA(0), buttonB(46), buttonAB(0, 46)
    // , touchPadP(9), touchPadY(10), touchPadT(11), touchPadH(12), touchPadO(13), touchPadN(14),
    //   sound(6)
{
}

void MPython::begin(void)
{
    // 初始化各功能部件
    rgb.begin();
    rgb.write(-1, 0);
    // display.begin(0x3c);
    // buzz.begin();
    // buzz.off();
    // audio.begin();
    // accelerometer.init();
    // magnetometer.init();
    // lightSensor.init();
}

void MPython::setTouchThreshold(uint32_t threshold)
{
    threshold = constrain(threshold, 0, 80);
    // touchPadP.threshold = threshold;
    // touchPadY.threshold = threshold;
    // touchPadT.threshold = threshold;
    // touchPadH.threshold = threshold;
    // touchPadO.threshold = threshold;
    // touchPadN.threshold = threshold;
}

// 全局对象实例化
MPython mPython;