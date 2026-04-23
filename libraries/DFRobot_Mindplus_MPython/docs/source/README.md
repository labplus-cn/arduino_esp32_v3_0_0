# mPython库概述

本文档提供了 DFRobot_Mindplus_MPython 库的详细使用指南和 API 参考。

## 文档结构

- [库安装](arduino_setup.md) - 安装指南和配置说明
- [示例文档](examples/index) - 综合示例
- [API 参考文档](api/index) - 完整的 API 参考

## 快速开始

1. 安装 DFRobot_Mindplus_MPython 库
2. 包含头文件：`#include <MPython.h>`
3. 初始化硬件：`mPython.begin();`
4. 开始使用各个模块的功能

## 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "Hello, mPython!", 0xFFFF);
  mPython.display.show();
  
  // 点亮 RGB LED
  mPython.rgb.begin();
  mPython.rgb.write(0, 255, 0, 0);
  
  // 蜂鸣器发声
  mPython.buzz.begin();
  mPython.buzz.freq(1000, BEAT_1);
}

void loop() {
  // 读取按钮状态
  if (mPython.buttonA.isPressed()) {
    mPython.display.drawText(20, 30, "Button A pressed!", 0x00FF00);
    mPython.display.show();
  }
  delay(100);
}
```
