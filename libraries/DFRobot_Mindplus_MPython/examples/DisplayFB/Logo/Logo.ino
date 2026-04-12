#include <MPython.h>

void setup() {
    // 初始化MPython
    mPython.begin();
    // 初始化串口
    Serial.begin(115200);
    Serial.println("Display Logo Example");
    
    // 显示logo
    mPython.display.showLogo();
}

void loop() {
    // 什么都不做
}