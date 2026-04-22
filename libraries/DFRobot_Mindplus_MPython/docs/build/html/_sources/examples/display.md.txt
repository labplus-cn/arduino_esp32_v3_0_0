# Display 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 DisplayFB 类的使用示例。

## 示例 1：基本文本显示

### 示例介绍

本示例演示了如何在显示屏上显示文本和基本图形。

### 功能演示图片

![基本文本显示](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20text%20and%20basic%20graphics&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  
  // 绘制标题
  mPython.display.drawText(20, 10, "基本文本显示", 0xFFFF);
  
  // 绘制普通文本
  mPython.display.drawText(20, 40, "Hello, mPython!", 0x00FF00);
  
  // 绘制中文字符
  mPython.display.drawTextCN(20, 70, "你好，掌控板！", 0x00FFFF);
  
  // 绘制线条
  mPython.display.drawLine(20, 100, 300, 100, 0xFF0000);
  
  // 显示更新
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 2：图形绘制

### 示例介绍

本示例演示了如何在显示屏上绘制各种图形，包括线条、矩形、圆形等。

### 功能演示图片

![图形绘制](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20various%20shapes%20and%20colors&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  
  // 绘制标题
  mPython.display.drawText(20, 10, "图形绘制演示", 0xFFFF);
  
  // 绘制线条
  mPython.display.drawLine(20, 40, 300, 40, 0xFF0000);
  
  // 绘制矩形
  mPython.display.drawRect(20, 60, 80, 60, 0x00FF00);
  mPython.display.fillRect(120, 60, 80, 60, 0x0000FF);
  
  // 绘制圆形
  mPython.display.drawCircle(70, 180, 30, 0xFFFF00);
  mPython.display.fillCircle(160, 180, 30, 0xFF00FF);
  
  // 绘制三角形（使用线条）
  mPython.display.drawLine(220, 150, 270, 210, 0x00FFFF);
  mPython.display.drawLine(270, 210, 170, 210, 0x00FFFF);
  mPython.display.drawLine(170, 210, 220, 150, 0x00FFFF);
  
  // 显示更新
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 3：二维码显示

### 示例介绍

本示例演示了如何在显示屏上显示二维码。

### 功能演示图片

![二维码显示](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20QR%20code&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  
  // 绘制标题
  mPython.display.drawText(20, 10, "二维码显示", 0xFFFF);
  
  // 显示二维码
  mPython.display.drawQRCode(50, 40, "https://www.dfrobot.com", 3);
  
  // 显示更新
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 4：动态文本显示

### 示例介绍

本示例演示了如何在显示屏上动态显示文本，例如显示传感器数据。

### 功能演示图片

![动态文本显示](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20dynamic%20sensor%20data&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化加速度计
  mPython.accelerometer.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "动态文本显示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 读取加速度计数据
  float x = mPython.accelerometer.getX();
  float y = mPython.accelerometer.getY();
  float z = mPython.accelerometer.getZ();
  
  // 清空数据显示区域
  mPython.display.fillRect(20, 40, 280, 180, 0x0000);
  
  // 显示加速度计数据
  mPython.display.drawText(20, 50, "加速度计数据:", 0xFFFF);
  mPython.display.drawText(20, 80, String("X: " + String(x, 2)), 0x00FF00);
  mPython.display.drawText(20, 110, String("Y: " + String(y, 2)), 0x00FFFF);
  mPython.display.drawText(20, 140, String("Z: " + String(z, 2)), 0xFF00FF);
  
  // 显示更新
  mPython.display.show();
  
  delay(100);
}
```
