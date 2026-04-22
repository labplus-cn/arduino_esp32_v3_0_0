# Button 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 Button 类的使用示例。

## 示例 1：基本按钮检测

### 示例介绍

本示例演示了如何检测按钮的按下状态。

### 功能演示图片

![基本按钮检测](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20buttons%20and%20display%20showing%20button%20state&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "按钮检测演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查按钮 A 状态
  if (mPython.buttonA.isPressed()) {
    mPython.display.drawText(20, 40, "按钮 A: 按下", 0x00FF00);
  } else {
    mPython.display.drawText(20, 40, "按钮 A: 释放", 0xFFFF);
  }
  
  // 检查按钮 B 状态
  if (mPython.buttonB.isPressed()) {
    mPython.display.drawText(20, 60, "按钮 B: 按下", 0x00FF00);
  } else {
    mPython.display.drawText(20, 60, "按钮 B: 释放", 0xFFFF);
  }
  
  // 检查按钮 AB 状态
  if (mPython.buttonAB.isPressed()) {
    mPython.display.drawText(20, 80, "按钮 AB: 按下", 0x00FF00);
  } else {
    mPython.display.drawText(20, 80, "按钮 AB: 释放", 0xFFFF);
  }
  
  mPython.display.show();
  delay(100);
}
```

## 示例 2：按钮控制 LED

### 示例介绍

本示例演示了如何使用按钮控制 RGB LED。

### 功能演示图片

![按钮控制 LED](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20buttons%20and%20RGB%20LEDs&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化 RGB LED
  mPython.rgb.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "按钮控制 LED", 0xFFFF);
  mPython.display.drawText(20, 40, "按钮 A: 红色", 0xFFFF);
  mPython.display.drawText(20, 60, "按钮 B: 绿色", 0xFFFF);
  mPython.display.drawText(20, 80, "按钮 AB: 蓝色", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 按钮 A 控制红色 LED
  if (mPython.buttonA.isPressed()) {
    mPython.rgb.write(0, 255, 0, 0);
  } else {
    mPython.rgb.write(0, 0, 0, 0);
  }
  
  // 按钮 B 控制绿色 LED
  if (mPython.buttonB.isPressed()) {
    mPython.rgb.write(1, 0, 255, 0);
  } else {
    mPython.rgb.write(1, 0, 0, 0);
  }
  
  // 按钮 AB 控制蓝色 LED
  if (mPython.buttonAB.isPressed()) {
    mPython.rgb.write(2, 0, 0, 255);
  } else {
    mPython.rgb.write(2, 0, 0, 0);
  }
  
  delay(100);
}
```

## 示例 3：按钮回调函数

### 示例介绍

本示例演示了如何使用按钮的回调函数。

### 功能演示图片

![按钮回调函数](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20button%20callback%20events&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

// 回调函数
void onButtonAPressed() {
  mPython.display.drawText(20, 110, "按钮 A 按下", 0x00FF00);
  mPython.display.show();
  mPython.rgb.write(0, 255, 0, 0);
}

void onButtonAReleased() {
  mPython.display.drawText(20, 110, "按钮 A 释放", 0xFFFF);
  mPython.display.show();
  mPython.rgb.write(0, 0, 0, 0);
}

void onButtonBPressed() {
  mPython.display.drawText(20, 130, "按钮 B 按下", 0x00FF00);
  mPython.display.show();
  mPython.rgb.write(1, 0, 255, 0);
}

void onButtonBReleased() {
  mPython.display.drawText(20, 130, "按钮 B 释放", 0xFFFF);
  mPython.display.show();
  mPython.rgb.write(1, 0, 0, 0);
}

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化 RGB LED
  mPython.rgb.begin();
  
  // 设置按钮回调
  mPython.buttonA.setPressedCallback(onButtonAPressed);
  mPython.buttonA.setUnPressedCallback(onButtonAReleased);
  mPython.buttonB.setPressedCallback(onButtonBPressed);
  mPython.buttonB.setUnPressedCallback(onButtonBReleased);
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "按钮回调演示", 0xFFFF);
  mPython.display.drawText(20, 40, "按下/释放按钮查看效果", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 主循环
  delay(100);
}
```

## 示例 4：按钮计数

### 示例介绍

本示例演示了如何使用按钮进行计数。

### 功能演示图片

![按钮计数](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20button%20count&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

int count = 0;
bool buttonAPressed = false;
bool buttonBPressed = false;

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "按钮计数演示", 0xFFFF);
  mPython.display.drawText(20, 40, "按钮 A: 增加", 0xFFFF);
  mPython.display.drawText(20, 60, "按钮 B: 减少", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 按钮 A 增加计数
  if (mPython.buttonA.isPressed() && !buttonAPressed) {
    buttonAPressed = true;
    count++;
  } else if (!mPython.buttonA.isPressed() && buttonAPressed) {
    buttonAPressed = false;
  }
  
  // 按钮 B 减少计数
  if (mPython.buttonB.isPressed() && !buttonBPressed) {
    buttonBPressed = true;
    count--;
  } else if (!mPython.buttonB.isPressed() && buttonBPressed) {
    buttonBPressed = false;
  }
  
  // 显示计数
  mPython.display.drawText(20, 90, String("计数: " + String(count)), 0x00FFFF);
  mPython.display.show();
  
  delay(100);
}
```
