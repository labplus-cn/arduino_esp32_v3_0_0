# Sensors 示例

本文档提供了 DFRobot_Mindplus_MPython 库中传感器类的使用示例。

## 示例 1：加速度计数据读取

### 示例介绍

本示例演示了如何读取加速度计的数据。

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
  mPython.display.drawText(20, 10, "加速度计数据演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 读取加速度计数据
  float x = mPython.accelerometer.getX();
  float y = mPython.accelerometer.getY();
  float z = mPython.accelerometer.getZ();
  float strength = mPython.accelerometer.getStrength();
  
  // 显示数据
  mPython.display.fillRect(20, 40, 280, 160, 0x0000);
  
  mPython.display.drawText(20, 50, String("X: " + String(x, 2)), 0xFFFF);
  mPython.display.drawText(20, 70, String("Y: " + String(y, 2)), 0xFFFF);
  mPython.display.drawText(20, 90, String("Z: " + String(z, 2)), 0xFFFF);
  mPython.display.drawText(20, 110, String("强度: " + String(strength, 2)), 0xFFFF);
  
  mPython.display.show();
  delay(100);
}
```

## 示例 2：手势检测

### 示例介绍

本示例演示了如何使用加速度计进行手势检测。

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
  mPython.display.drawText(20, 10, "手势检测演示", 0xFFFF);
  mPython.display.drawText(20, 40, "尝试以下手势:", 0xFFFF);
  mPython.display.drawText(20, 60, "摇晃、倾斜、翻转", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查各种手势
  if (mPython.accelerometer.isGesture(mPython.accelerometer.Shake)) {
    mPython.display.drawText(20, 90, "手势: 摇晃", 0x00FF00);
    mPython.rgb.write(0, 255, 0, 0);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.TiltLeft)) {
    mPython.display.drawText(20, 90, "手势: 向左倾斜", 0x00FF00);
    mPython.rgb.write(1, 0, 255, 0);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.TiltRight)) {
    mPython.display.drawText(20, 90, "手势: 向右倾斜", 0x00FF00);
    mPython.rgb.write(2, 0, 0, 255);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.ScreenUp)) {
    mPython.display.drawText(20, 90, "手势: 屏幕朝上", 0x00FF00);
    mPython.rgb.write(0, 255, 255, 0);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.ScreenDown)) {
    mPython.display.drawText(20, 90, "手势: 屏幕朝下", 0x00FF00);
    mPython.rgb.write(1, 255, 0, 255);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.TiltForward)) {
    mPython.display.drawText(20, 90, "手势: 向前倾斜", 0x00FF00);
    mPython.rgb.write(2, 0, 255, 255);
  } else if (mPython.accelerometer.isGesture(mPython.accelerometer.TiltBack)) {
    mPython.display.drawText(20, 90, "手势: 向后倾斜", 0x00FF00);
    mPython.rgb.write(0, 255, 255, 255);
  } else {
    mPython.display.drawText(20, 90, "手势: 无", 0xFFFF);
    mPython.rgb.write(0, 0, 0, 0);
    mPython.rgb.write(1, 0, 0, 0);
    mPython.rgb.write(2, 0, 0, 0);
  }
  
  mPython.display.show();
  delay(100);
}
```

## 示例 3：磁力计数据读取

### 示例介绍

本示例演示了如何读取磁力计的数据。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化磁力计
  mPython.magnetometer.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "磁力计数据演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 读取磁力计数据
  float x = mPython.magnetometer.getX();
  float y = mPython.magnetometer.getY();
  float z = mPython.magnetometer.getZ();
  
  // 显示数据
  mPython.display.fillRect(20, 40, 280, 100, 0x0000);
  
  mPython.display.drawText(20, 50, String("X: " + String(x, 2)), 0xFFFF);
  mPython.display.drawText(20, 70, String("Y: " + String(y, 2)), 0xFFFF);
  mPython.display.drawText(20, 90, String("Z: " + String(z, 2)), 0xFFFF);
  
  mPython.display.show();
  delay(100);
}
```

## 示例 4：光线传感器数据读取

### 示例介绍

本示例演示了如何读取光线传感器的数据。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化光线传感器
  mPython.lightSensor.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "光线传感器演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 读取光线强度
  uint16_t lux = mPython.lightSensor.getLux();
  
  // 显示数据
  mPython.display.fillRect(20, 40, 280, 80, 0x0000);
  
  mPython.display.drawText(20, 50, String("光照强度: " + String(lux) + " lux"), 0xFFFF);
  
  // 根据光照强度调整 RGB LED 亮度
  uint8_t brightness = map(lux, 0, 1000, 0, 255);
  mPython.rgb.brightness(brightness);
  mPython.rgb.write(0, 255, 255, 255);
  
  mPython.display.show();
  delay(100);
}
```
