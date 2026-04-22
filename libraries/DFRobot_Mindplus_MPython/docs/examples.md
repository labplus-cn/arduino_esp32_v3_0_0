# DFRobot_Mindplus_MPython 库示例文档

本文档提供了 DFRobot_Mindplus_MPython 库的完整使用示例，包括所有功能部件的使用方法和代码示例。

## 目录

- [基本功能演示](#基本功能演示)
- [Display 示例](#display-示例)
- [RGB 示例](#rgb-示例)
- [Buzz 示例](#buzz-示例)
- [Audio 示例](#audio-示例)
- [Button 示例](#button-示例)
- [TouchPad 示例](#touchpad-示例)
- [WiFi 示例](#wifi-示例)
- [传感器示例](#传感器示例)
- [USBDISK 示例](#usbdisk-示例)

## 基本功能演示

### 示例介绍

本示例演示了 mPython 掌控板的基本功能，包括：
- 显示屏文本和图形绘制
- RGB LED 控制
- 蜂鸣器发声
- 按钮和触摸板检测
- 传感器数据读取

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕并显示标题
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "mPython 基本功能演示", 0xFFFF);
  
  // 初始化 RGB LED
  mPython.rgb.begin();
  
  // 初始化蜂鸣器
  mPython.buzz.begin();
  
  // 初始化传感器
  mPython.accelerometer.begin();
  mPython.magnetometer.begin();
  mPython.lightSensor.begin();
}

void loop() {
  // 读取加速度计数据
  float x = mPython.accelerometer.getX();
  float y = mPython.accelerometer.getY();
  float z = mPython.accelerometer.getZ();
  
  // 读取光线传感器数据
  uint16_t lux = mPython.lightSensor.getLux();
  
  // 读取按钮状态
  bool buttonA = mPython.buttonA.isPressed();
  bool buttonB = mPython.buttonB.isPressed();
  
  // 读取触摸板状态
  bool touchP = mPython.touchPadP.isTouched();
  
  // 显示传感器数据
  mPython.display.fillRect(10, 40, 300, 180, 0x0000);
  mPython.display.drawText(20, 50, "加速度计:", 0xFFFF);
  mPython.display.drawText(20, 70, String("X: " + String(x, 2)), 0xFFFF);
  mPython.display.drawText(20, 90, String("Y: " + String(y, 2)), 0xFFFF);
  mPython.display.drawText(20, 110, String("Z: " + String(z, 2)), 0xFFFF);
  
  mPython.display.drawText(20, 130, "光线强度:", 0xFFFF);
  mPython.display.drawText(20, 150, String("Lux: " + String(lux)), 0xFFFF);
  
  mPython.display.drawText(20, 170, "按钮状态:", 0xFFFF);
  mPython.display.drawText(20, 190, String("A: " + String(buttonA ? "按下" : "释放")), 0xFFFF);
  mPython.display.drawText(20, 210, String("B: " + String(buttonB ? "按下" : "释放")), 0xFFFF);
  
  // 控制 RGB LED
  if (touchP) {
    mPython.rgb.write(0, 255, 0, 0);  // 红色
    mPython.rgb.write(1, 0, 255, 0);  // 绿色
    mPython.rgb.write(2, 0, 0, 255);  // 蓝色
  } else {
    mPython.rgb.write(0, 0, 0, 0);
    mPython.rgb.write(1, 0, 0, 0);
    mPython.rgb.write(2, 0, 0, 0);
  }
  
  // 按钮控制蜂鸣器
  if (buttonA) {
    mPython.buzz.freq(1000);  // 1kHz
  } else if (buttonB) {
    mPython.buzz.freq(500);   // 500Hz
  } else {
    mPython.buzz.off();
  }
  
  // 显示更新
  mPython.display.show();
  
  delay(100);
}
```

## Display 示例

### 示例 1：基本文本显示

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

### 示例 2：图形绘制

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

### 示例 3：二维码显示

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

### 示例 4：动态文本显示

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

## RGB 示例

### 示例 1：基本颜色控制

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

### 示例 2：多 LED 控制

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

### 示例 3：亮度控制

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

### 示例 4：颜色渐变效果

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

## Buzz 示例

### 示例 1：基本音调控制

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化蜂鸣器
  mPython.buzz.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "基本音调控制", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 发出不同频率的声音
  mPython.display.drawText(20, 40, "500Hz", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(500);
  delay(1000);
  
  mPython.display.drawText(20, 40, "1000Hz", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(1000);
  delay(1000);
  
  mPython.display.drawText(20, 40, "1500Hz", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(1500);
  delay(1000);
  
  mPython.display.drawText(20, 40, "2000Hz", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(2000);
  delay(1000);
  
  // 停止发声
  mPython.buzz.off();
  mPython.display.drawText(20, 40, "停止", 0xFFFF);
  mPython.display.show();
  delay(1000);
}
```

### 示例 2：音乐播放

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化蜂鸣器
  mPython.buzz.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "音乐播放演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 播放生日快乐歌
  mPython.display.drawText(20, 40, "播放: 生日快乐", 0xFFFF);
  mPython.display.show();
  mPython.buzz.play(BIRTHDAY, Once);
  delay(3000);
  
  // 播放婚礼进行曲
  mPython.display.drawText(20, 40, "播放: 婚礼进行曲", 0xFFFF);
  mPython.display.show();
  mPython.buzz.play(WEDDING, Once);
  delay(4000);
  
  // 播放贝多芬第五交响曲
  mPython.display.drawText(20, 40, "播放: 贝多芬第五", 0xFFFF);
  mPython.display.show();
  mPython.buzz.play(DADADADUM, Once);
  delay(3000);
  
  // 播放彩虹猫
  mPython.display.drawText(20, 40, "播放: 彩虹猫", 0xFFFF);
  mPython.display.show();
  mPython.buzz.play(NYAN, Once);
  delay(5000);
  
  // 停止
  mPython.buzz.stop();
  mPython.display.drawText(20, 40, "停止", 0xFFFF);
  mPython.display.show();
  delay(2000);
}
```

### 示例 3：节拍控制

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化蜂鸣器
  mPython.buzz.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "节拍控制演示", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 播放不同节拍的音符
  mPython.display.drawText(20, 40, "1拍", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(500, BEAT_1);
  delay(1000);
  
  mPython.display.drawText(20, 40, "1/2拍", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(600, BEAT_1_2);
  delay(500);
  
  mPython.display.drawText(20, 40, "1/4拍", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(700, BEAT_1_4);
  delay(250);
  
  mPython.display.drawText(20, 40, "2拍", 0xFFFF);
  mPython.display.show();
  mPython.buzz.freq(800, BEAT_2);
  delay(2000);
  
  // 停止
  mPython.buzz.off();
  mPython.display.drawText(20, 40, "停止", 0xFFFF);
  mPython.display.show();
  delay(1000);
}
```

### 示例 4：按钮控制蜂鸣器

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化蜂鸣器
  mPython.buzz.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "按钮控制蜂鸣器", 0xFFFF);
  mPython.display.drawText(20, 40, "按钮 A: 开启", 0xFFFF);
  mPython.display.drawText(20, 60, "按钮 B: 关闭", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查按钮状态
  if (mPython.buttonA.isPressed()) {
    // 按钮 A 按下，开启蜂鸣器
    mPython.buzz.freq(1000);
    mPython.display.drawText(20, 90, "状态: 开启", 0x00FF00);
    mPython.display.show();
  } else if (mPython.buttonB.isPressed()) {
    // 按钮 B 按下，关闭蜂鸣器
    mPython.buzz.off();
    mPython.display.drawText(20, 90, "状态: 关闭", 0xFF0000);
    mPython.display.show();
  }
  
  delay(100);
}
```

## Audio 示例

### 示例 1：音频播放

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化音频
  mPython.audio.begin();
  
  // 设置音量
  mPython.audio.setVolume(50);
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "音频播放演示", 0xFFFF);
  mPython.display.drawText(20, 40, "按钮 A: 播放", 0xFFFF);
  mPython.display.drawText(20, 60, "按钮 B: 停止", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查按钮状态
  if (mPython.buttonA.isPressed()) {
    // 播放音频文件
    mPython.display.drawText(20, 90, "播放中...", 0x00FF00);
    mPython.display.show();
    
    mPython.audio.play("/spiffs/test.mp3");
    
    // 等待播放完成
    while (mPython.audio.state() == mPython.audio.PLAY_STATE_PLAYING) {
      delay(100);
    }
    
    mPython.display.drawText(20, 90, "播放完成", 0xFFFF);
    mPython.display.show();
  } else if (mPython.buttonB.isPressed()) {
    // 停止播放
    mPython.audio.stop();
    mPython.display.drawText(20, 90, "已停止", 0xFF0000);
    mPython.display.show();
  }
  
  delay(100);
}
```

### 示例 2：录音功能

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化音频
  mPython.audio.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "录音功能演示", 0xFFFF);
  mPython.display.drawText(20, 40, "按钮 A: 开始录音", 0xFFFF);
  mPython.display.drawText(20, 60, "按钮 B: 停止录音", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查按钮状态
  if (mPython.buttonA.isPressed()) {
    // 开始录音
    mPython.display.drawText(20, 90, "录音中...", 0x00FF00);
    mPython.display.show();
    
    mPython.audio.startRecord("/spiffs/record.wav", 16000, 16, 1);
    
    // 录音5秒
    delay(5000);
    
    mPython.audio.stopRecord();
    mPython.display.drawText(20, 90, "录音完成", 0xFFFF);
    mPython.display.show();
    
    // 播放录音
    mPython.display.drawText(20, 110, "播放录音...", 0xFFFF);
    mPython.display.show();
    
    mPython.audio.play("/spiffs/record.wav");
    while (mPython.audio.state() == mPython.audio.PLAY_STATE_PLAYING) {
      delay(100);
    }
    
    mPython.display.drawText(20, 110, "播放完成", 0xFFFF);
    mPython.display.show();
  } else if (mPython.buttonB.isPressed()) {
    // 停止录音
    mPython.audio.stopRecord();
    mPython.display.drawText(20, 90, "录音已停止", 0xFF0000);
    mPython.display.show();
  }
  
  delay(100);
}
```

### 示例 3：语音合成 (TTS)

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化音频
  mPython.audio.begin();
  
  // 初始化语音合成
  if (mPython.audio.ttsInit()) {
    mPython.display.fillScreen(0x0000);
    mPython.display.drawText(20, 10, "语音合成演示", 0xFFFF);
    mPython.display.drawText(20, 40, "按钮 A: 播放语音", 0xFFFF);
    mPython.display.show();
  } else {
    mPython.display.fillScreen(0x0000);
    mPython.display.drawText(20, 10, "TTS 初始化失败", 0xFF0000);
    mPython.display.show();
  }
}

void loop() {
  // 检查按钮状态
  if (mPython.buttonA.isPressed()) {
    // 播放语音
    mPython.display.drawText(20, 70, "播放中...", 0x00FF00);
    mPython.display.show();
    
    mPython.audio.textToSpeech("欢迎使用 mPython 掌控板，这是一个语音合成示例");
    
    // 等待播放完成
    delay(5000);
    
    mPython.display.drawText(20, 70, "播放完成", 0xFFFF);
    mPython.display.show();
  }
  
  delay(100);
}
```

### 示例 4：语音识别

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化音频
  mPython.audio.begin();
  
  // 初始化语音合成
  mPython.audio.ttsInit();
  
  // 初始化语音识别
  mPython.audio.srBegin("你好，我是 mPython");
  
  // 添加语音命令
  mPython.audio.srAddCommand(1, "打开灯");
  mPython.audio.srAddCommand(2, "关闭灯");
  mPython.audio.srAddCommand(3, "播放音乐");
  mPython.audio.srAddCommand(4, "停止播放");
  
  // 应用命令
  mPython.audio.srApplyCommands();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "语音识别演示", 0xFFFF);
  mPython.display.drawText(20, 40, "请说出命令:", 0xFFFF);
  mPython.display.drawText(20, 60, "打开灯 / 关闭灯", 0xFFFF);
  mPython.display.drawText(20, 80, "播放音乐 / 停止播放", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查语音命令
  int commandId = mPython.audio.srGetCommandId();
  
  if (commandId > 0) {
    switch (commandId) {
      case 1:
        mPython.display.drawText(20, 110, "命令: 打开灯", 0x00FF00);
        mPython.audio.textToSpeech("灯已打开");
        break;
      case 2:
        mPython.display.drawText(20, 110, "命令: 关闭灯", 0x00FF00);
        mPython.audio.textToSpeech("灯已关闭");
        break;
      case 3:
        mPython.display.drawText(20, 110, "命令: 播放音乐", 0x00FF00);
        mPython.audio.textToSpeech("开始播放音乐");
        break;
      case 4:
        mPython.display.drawText(20, 110, "命令: 停止播放", 0x00FF00);
        mPython.audio.textToSpeech("音乐已停止");
        break;
    }
    mPython.display.show();
    delay(2000);
  }
  
  delay(100);
}
```

## Button 示例

### 示例 1：基本按钮检测

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

### 示例 2：按钮控制 LED

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

### 示例 3：按钮回调函数

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

### 示例 4：按钮计数

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

## TouchPad 示例

### 示例 1：基本触摸检测

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

### 示例 2：触摸控制 LED

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

### 示例 3：触摸值读取

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

### 示例 4：触摸回调函数

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

## WiFi 示例

### 示例 1：WiFi 连接

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "WiFi 连接演示", 0xFFFF);
  mPython.display.show();
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 40, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 40, "连接成功！", 0x00FF00);
    mPython.display.drawText(20, 60, String("SSID: " + mPython.wifi.ssid()), 0xFFFF);
    mPython.display.drawText(20, 80, String("IP: " + mPython.wifi.localIP()), 0xFFFF);
    mPython.display.drawText(20, 100, String("RSSI: " + String(mPython.wifi.rssi()) + " dBm"), 0xFFFF);
    mPython.display.drawText(20, 120, String("MAC: " + mPython.wifi.macAddress()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

### 示例 2：WiFi 扫描

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "WiFi 扫描演示", 0xFFFF);
  mPython.display.show();
  
  // 扫描网络
  mPython.display.drawText(20, 40, "正在扫描网络...", 0xFFFF);
  mPython.display.show();
  
  int networkCount = mPython.wifi.scanNetworks();
  
  mPython.display.drawText(20, 40, String("发现 " + String(networkCount) + " 个网络"), 0xFFFF);
  
  // 显示前5个网络
  for (int i = 0; i < min(5, networkCount); i++) {
    String ssid = mPython.wifi.ssid(i);
    int32_t rssi = mPython.wifi.rssi(i);
    String encType = mPython.wifi.encryptionType(i);
    
    mPython.display.drawText(20, 60 + i * 30, String(ssid), 0xFFFF);
    mPython.display.drawText(200, 60 + i * 30, String(String(rssi) + " dBm"), 0xFFFF);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

### 示例 3：网络信息获取

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "网络信息演示", 0xFFFF);
  mPython.display.show();
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 40, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 40, "连接成功！", 0x00FF00);
    
    // 获取网络信息
    mPython.display.drawText(20, 60, String("SSID: " + mPython.wifi.ssid()), 0xFFFF);
    mPython.display.drawText(20, 80, String("BSSID: " + mPython.wifi.bssid()), 0xFFFF);
    mPython.display.drawText(20, 100, String("本地 IP: " + mPython.wifi.localIP()), 0xFFFF);
    mPython.display.drawText(20, 120, String("网关 IP: " + mPython.wifi.gatewayIP()), 0xFFFF);
    mPython.display.drawText(20, 140, String("子网掩码: " + mPython.wifi.subnetMask()), 0xFFFF);
    mPython.display.drawText(20, 160, String("DNS IP: " + mPython.wifi.dnsIP()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

### 示例 4：自动重连设置

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "自动重连设置", 0xFFFF);
  mPython.display.show();
  
  // 设置自动重连
  bool autoReconnect = mPython.wifi.setAutoReconnect(true);
  
  if (autoReconnect) {
    mPython.display.drawText(20, 40, "自动重连已启用", 0x00FF00);
  } else {
    mPython.display.drawText(20, 40, "自动重连设置失败", 0xFF0000);
  }
  
  // 显示当前自动重连状态
  bool currentStatus = mPython.wifi.autoReconnect();
  mPython.display.drawText(20, 60, String("当前状态: " + String(currentStatus ? "启用" : "禁用")), 0xFFFF);
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 80, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 80, "连接成功！", 0x00FF00);
    mPython.display.drawText(20, 100, String("IP: " + mPython.wifi.localIP()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 80, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 传感器示例

### 示例 1：加速度计数据读取

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

### 示例 2：手势检测

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
