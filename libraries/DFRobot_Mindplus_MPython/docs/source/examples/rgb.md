# RGB 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 RGB 类的使用示例。

## 示例 1：基本颜色控制

### 示例介绍

本示例演示了如何控制 RGB LED 的颜色。

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
  mPython.display.drawText(20, 10, "RGB 颜色控制", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 红色
  mPython.rgb.write(0, 255, 0, 0);
  mPython.display.drawText(20, 40, "红色", 0xFF0000);
  mPython.display.show();
  delay(1000);
  
  // 绿色
  mPython.rgb.write(0, 0, 255, 0);
  mPython.display.drawText(20, 40, "绿色", 0x00FF00);
  mPython.display.show();
  delay(1000);
  
  // 蓝色
  mPython.rgb.write(0, 0, 0, 255);
  mPython.display.drawText(20, 40, "蓝色", 0x0000FF);
  mPython.display.show();
  delay(1000);
  
  // 黄色
  mPython.rgb.write(0, 255, 255, 0);
  mPython.display.drawText(20, 40, "黄色", 0xFFFF00);
  mPython.display.show();
  delay(1000);
  
  // 紫色
  mPython.rgb.write(0, 255, 0, 255);
  mPython.display.drawText(20, 40, "紫色", 0xFF00FF);
  mPython.display.show();
  delay(1000);
  
  // 青色
  mPython.rgb.write(0, 0, 255, 255);
  mPython.display.drawText(20, 40, "青色", 0x00FFFF);
  mPython.display.show();
  delay(1000);
  
  // 白色
  mPython.rgb.write(0, 255, 255, 255);
  mPython.display.drawText(20, 40, "白色", 0xFFFF);
  mPython.display.show();
  delay(1000);
  
  // 关闭
  mPython.rgb.write(0, 0, 0, 0);
  mPython.display.drawText(20, 40, "关闭", 0xFFFF);
  mPython.display.show();
  delay(1000);
}
```

## 示例 2：多 LED 控制

### 示例介绍

本示例演示了如何控制多个 RGB LED。

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
  mPython.display.drawText(20, 10, "多 LED 控制", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 三个 LED 分别显示不同颜色
  mPython.rgb.write(0, 255, 0, 0);   // 红色
  mPython.rgb.write(1, 0, 255, 0);   // 绿色
  mPython.rgb.write(2, 0, 0, 255);   // 蓝色
  mPython.display.drawText(20, 40, "红 绿 蓝", 0xFFFF);
  mPython.display.show();
  delay(1000);
  
  // 三个 LED 都显示黄色
  mPython.rgb.write(0, 255, 255, 0);
  mPython.rgb.write(1, 255, 255, 0);
  mPython.rgb.write(2, 255, 255, 0);
  mPython.display.drawText(20, 40, "黄色", 0xFFFF00);
  mPython.display.show();
  delay(1000);
  
  // 三个 LED 都显示紫色
  mPython.rgb.write(0, 255, 0, 255);
  mPython.rgb.write(1, 255, 0, 255);
  mPython.rgb.write(2, 255, 0, 255);
  mPython.display.drawText(20, 40, "紫色", 0xFF00FF);
  mPython.display.show();
  delay(1000);
  
  // 三个 LED 都显示青色
  mPython.rgb.write(0, 0, 255, 255);
  mPython.rgb.write(1, 0, 255, 255);
  mPython.rgb.write(2, 0, 255, 255);
  mPython.display.drawText(20, 40, "青色", 0x00FFFF);
  mPython.display.show();
  delay(1000);
  
  // 关闭所有 LED
  mPython.rgb.write(0, 0, 0, 0);
  mPython.rgb.write(1, 0, 0, 0);
  mPython.rgb.write(2, 0, 0, 0);
  mPython.display.drawText(20, 40, "关闭", 0xFFFF);
  mPython.display.show();
  delay(1000);
}
```

## 示例 3：亮度控制

### 示例介绍

本示例演示了如何控制 RGB LED 的亮度。

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
  mPython.display.drawText(20, 10, "亮度控制", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 亮度从低到高
  for (int i = 0; i <= 255; i += 5) {
    mPython.rgb.brightness(i);
    mPython.rgb.write(0, 255, 0, 0);
    mPython.display.drawText(20, 40, String("亮度: " + String(i)), 0xFFFF);
    mPython.display.show();
    delay(50);
  }
  
  // 亮度从高到低
  for (int i = 255; i >= 0; i -= 5) {
    mPython.rgb.brightness(i);
    mPython.rgb.write(0, 255, 0, 0);
    mPython.display.drawText(20, 40, String("亮度: " + String(i)), 0xFFFF);
    mPython.display.show();
    delay(50);
  }
  
  // 关闭
  mPython.rgb.write(0, 0, 0, 0);
  mPython.display.drawText(20, 40, "关闭", 0xFFFF);
  mPython.display.show();
  delay(1000);
}
```

## 示例 4：颜色渐变效果

### 示例介绍

本示例演示了如何创建 RGB LED 的颜色渐变效果。

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
  mPython.display.drawText(20, 10, "颜色渐变效果", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 红色到绿色渐变
  for (int i = 0; i <= 255; i++) {
    mPython.rgb.write(0, 255 - i, i, 0);
    delay(10);
  }
  
  // 绿色到蓝色渐变
  for (int i = 0; i <= 255; i++) {
    mPython.rgb.write(0, 0, 255 - i, i);
    delay(10);
  }
  
  // 蓝色到红色渐变
  for (int i = 0; i <= 255; i++) {
    mPython.rgb.write(0, i, 0, 255 - i);
    delay(10);
  }
}
```
