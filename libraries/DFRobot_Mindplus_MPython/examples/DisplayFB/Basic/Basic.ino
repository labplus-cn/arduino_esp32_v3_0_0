#include <MPython.h>

void setup() {
    // 初始化MPython
    mPython.begin();
    // 初始化串口
    Serial.begin(115200);
    Serial.println("Display Basic Example");
    
    // 清空屏幕
    mPython.display.fillScreen(GUI_Black);
    
    // 显示文本
    mPython.display.drawText(10, 10, "Hello mPython!", GUI_White);
    
    // 显示带变量的文本
    int value = 42;
    char buffer[128];
    sprintf(buffer, "Value: %d", value);
    mPython.display.drawText(10, 30, buffer, GUI_White);
    
    // 绘制直线
    mPython.display.drawLine(0, 50, 319, 50, GUI_Red);
    
    // 绘制矩形
    mPython.display.drawRect(10, 60, 100, 50, GUI_Green);
    
    // 填充矩形
    mPython.display.fillRect(120, 60, 100, 50, GUI_Blue);
    
    // 绘制圆形
    mPython.display.drawCircle(160, 150, 30, GUI_Yellow);
    
    // 填充圆形
    mPython.display.fillCircle(240, 150, 30, GUI_Cyan);
}

void loop() {
    // 什么都不做
}