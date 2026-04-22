# TouchPad 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 TouchPad 类的使用示例。

## 示例 1：基本触摸检测

### 示例介绍

本示例演示了如何检测触摸板的触摸状态。

### 功能演示图片

![基本触摸检测](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20touch%20pads%20and%20display%20showing%20touch%20state&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "触摸板检测演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查各个触摸板状态
  bool touchP = mPython.touchPadP.isTouched();
  bool touchY = mPython.touchPadY.isTouched();
  bool touchT = mPython.touchPadT.isTouched();
  bool touchH = mPython.touchPadH.isTouched();
  bool touchO = mPython.touchPadO.isTouched();
  bool touchN = mPython.touchPadN.isTouched();
  
  // 显示触摸状态
  mPython.display.fillRect(20, 40, 280, 160, 0x0000);
  
  mPython.display.drawText(20, 50, String("P: " + String(touchP ? "触摸" : "未触摸")), touchP ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(20, 70, String("Y: " + String(touchY ? "触摸" : "未触摸")), touchY ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(20, 90, String("T: " + String(touchT ? "触摸" : "未触摸")), touchT ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(20, 110, String("H: " + String(touchH ? "触摸" : "未触摸")), touchH ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(20, 130, String("O: " + String(touchO ? "触摸" : "未触摸")), touchO ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(20, 150, String("N: " + String(touchN ? "触摸" : "未触摸")), touchN ? 0x00FF00 : 0xFFFF);
  
  mPython.display.show();
  delay(100);
}
```

## 示例 2：触摸控制 LED

### 示例介绍

本示例演示了如何使用触摸板控制 RGB LED。

### 功能演示图片

![触摸控制 LED](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20touch%20pads%20controlling%20RGB%20LEDs&image_size=landscape_16_9)

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
  mPython.display.drawText(20, 10, "触摸控制 LED", 0xFFFF);
  mPython.display.drawText(20, 40, "触摸 P: 红色", 0xFFFF);
  mPython.display.drawText(20, 60, "触摸 Y: 绿色", 0xFFFF);
  mPython.display.drawText(20, 80, "触摸 T: 蓝色", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 触摸 P 控制红色 LED
  if (mPython.touchPadP.isTouched()) {
    mPython.rgb.write(0, 255, 0, 0);
  } else {
    mPython.rgb.write(0, 0, 0, 0);
  }
  
  // 触摸 Y 控制绿色 LED
  if (mPython.touchPadY.isTouched()) {
    mPython.rgb.write(1, 0, 255, 0);
  } else {
    mPython.rgb.write(1, 0, 0, 0);
  }
  
  // 触摸 T 控制蓝色 LED
  if (mPython.touchPadT.isTouched()) {
    mPython.rgb.write(2, 0, 0, 255);
  } else {
    mPython.rgb.write(2, 0, 0, 0);
  }
  
  delay(100);
}
```

## 示例 3：触摸值读取

### 示例介绍

本示例演示了如何读取触摸板的触摸值。

### 功能演示图片

![触摸值读取](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20touch%20pad%20values&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "触摸值读取演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 读取触摸值
  uint32_t valP = mPython.touchPadP.read();
  uint32_t valY = mPython.touchPadY.read();
  uint32_t valT = mPython.touchPadT.read();
  
  // 显示触摸值
  mPython.display.fillRect(20, 40, 280, 100, 0x0000);
  
  mPython.display.drawText(20, 50, String("P: " + String(valP)), 0xFFFF);
  mPython.display.drawText(20, 70, String("Y: " + String(valY)), 0xFFFF);
  mPython.display.drawText(20, 90, String("T: " + String(valT)), 0xFFFF);
  
  // 显示触摸状态
  bool touchP = mPython.touchPadP.isTouched();
  bool touchY = mPython.touchPadY.isTouched();
  bool touchT = mPython.touchPadT.isTouched();
  
  mPython.display.drawText(150, 50, String(touchP ? "触摸" : "未触摸"), touchP ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(150, 70, String(touchY ? "触摸" : "未触摸"), touchY ? 0x00FF00 : 0xFFFF);
  mPython.display.drawText(150, 90, String(touchT ? "触摸" : "未触摸"), touchT ? 0x00FF00 : 0xFFFF);
  
  mPython.display.show();
  delay(100);
}
```

## 示例 4：触摸回调函数

### 示例介绍

本示例演示了如何使用触摸板的回调函数。

### 功能演示图片

![触摸回调函数](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20touch%20callback%20events&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

// 回调函数
void onTouchPTouched() {
  mPython.display.drawText(20, 110, "触摸板 P: 触摸", 0x00FF00);
  mPython.display.show();
  mPython.rgb.write(0, 255, 0, 0);
}

void onTouchPUnTouched() {
  mPython.display.drawText(20, 110, "触摸板 P: 未触摸", 0xFFFF);
  mPython.display.show();
  mPython.rgb.write(0, 0, 0, 0);
}

void onTouchYTouched() {
  mPython.display.drawText(20, 130, "触摸板 Y: 触摸", 0x00FF00);
  mPython.display.show();
  mPython.rgb.write(1, 0, 255, 0);
}

void onTouchYUnTouched() {
  mPython.display.drawText(20, 130, "触摸板 Y: 未触摸", 0xFFFF);
  mPython.display.show();
  mPython.rgb.write(1, 0, 0, 0);
}

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化 RGB LED
  mPython.rgb.begin();
  
  // 设置触摸板回调
  mPython.touchPadP.setTouchedCallback(onTouchPTouched);
  mPython.touchPadP.setUnTouchedCallback(onTouchPUnTouched);
  mPython.touchPadY.setTouchedCallback(onTouchYTouched);
  mPython.touchPadY.setUnTouchedCallback(onTouchYUnTouched);
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "触摸回调演示", 0xFFFF);
  mPython.display.drawText(20, 40, "触摸/释放触摸板查看效果", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 主循环
  delay(100);
}
```
