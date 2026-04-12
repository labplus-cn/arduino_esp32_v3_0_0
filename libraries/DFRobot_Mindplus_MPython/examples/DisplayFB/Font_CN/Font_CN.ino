#include <MPython.h>

void setup() {
  // 初始化串口
  Serial.begin(115200);
  
  // 初始化掌控板
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  
  // 显示中文字符
  mPython.display.drawTextCN(10, 10, "你好，世界！", 0xFFFF);
  
  // 显示英文字符
  mPython.display.drawText(10, 40, "Hello, World!", 0xFFFF);
  
  // 显示更多中文字符
  mPython.display.drawTextCN(10, 70, "这是一个中文字符显示示例", 0x00FF);
  
  // 显示不同颜色的中文字符
  mPython.display.drawTextCN(10, 100, "红色文字", 0xF800);
  mPython.display.drawTextCN(10, 130, "绿色文字", 0x07E0);
  mPython.display.drawTextCN(10, 160, "蓝色文字", 0x001F);
}

void loop() {
  // 什么都不做
}