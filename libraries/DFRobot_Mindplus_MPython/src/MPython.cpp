#include "MPython.h"

MPython::MPython()
    : buttonA(0), buttonB(46), buttonAB(0, 46),
      touchPadP(9), touchPadY(10), touchPadT(11), touchPadH(12), touchPadO(13), touchPadN(14) {
}

void MPython::begin(void) {
    rgb.begin();
    rgb.write(-1, 0);
    display.begin();
    audio.begin();
}

MPython mPython;
