#include "MPython.h"

MPython::MPython()
    : buttonA(0), buttonB(46), buttonAB(0, 46),
      touchPadP(9), touchPadY(10), touchPadT(11), touchPadH(12), touchPadO(13), touchPadN(14) {
}

void MPython::begin(void) {
    // 初始化公共 I2C 总线（SDA=44, SCL=43）
    Wire.begin(44, 43);
    Wire.setClock(100000);
    rgb.begin();
    rgb.write(-1, 0);
    buzz.begin();
    display.begin();
    audio.begin();
}

MPython mPython;
