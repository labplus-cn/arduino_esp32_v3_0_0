#include "MPython.h"

MPython::MPython() 
    : buttonA(0), buttonB(46), buttonAB(0, 46)
    , touchPadP(9), touchPadY(10), touchPadT(11), touchPadH(12), touchPadO(13), touchPadN(14)
{
}

void MPython::begin(void)
{
    // 初始化各功能部件
    rgb.begin();
    rgb.write(-1, 0);
    display.begin(); // 直接初始化显示屏，使用LCD驱动中的参数
    // buzz.begin();
    // buzz.off();
    // audio.begin();
    // accelerometer.init();
    // magnetometer.init();
    // lightSensor.init();
}


// 全局对象实例化
MPython mPython;