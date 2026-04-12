#include <MPython.h>

void setup() {
  // 初始化MPython
  mPython.begin();
  
  // 初始化串口
  Serial.begin(115200);
  Serial.println("DisplayFB example started!");
  
  // 清空屏幕为黑色
  mPython.display.fillScreen(0x0000);
  
  // 绘制一个红色矩形
  mPython.display.fillRect(50, 50, 100, 50, 0xFF00);
  
  // 绘制一个绿色圆形
  mPython.display.fillCircle(200, 100, 30, 0x00FF);
  
  // 绘制一条蓝色线
  mPython.display.drawLine(0, 0, 319, 171, 0x0000FF);
  
  // 显示文本
  mPython.display.drawText(10, 10, "Hello, MPython!", 0xFFFF);
  mPython.display.drawText(10, 30, "DisplayFB Test", 0xFFFF);
  
  // 显示变量值
  int value = 123;
  mPython.display.drawText(10, 50, "Value: %d", 0xFFFF, value);
}

void loop() {
  // 等待一段时间
  delay(1000);
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  
  // 显示当前时间
  unsigned long time = millis() / 1000;
  mPython.display.drawText(10, 10, "Time: %lu seconds", 0xFFFF, time);
  
  // 绘制一个移动的矩形
  static int x = 0;
  mPython.display.fillRect(x, 50, 50, 50, 0xFF00);
  x = (x + 5) % 320;
}
