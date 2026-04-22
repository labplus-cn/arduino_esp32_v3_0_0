# Buzz 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 Buzz 类的使用示例。

## 示例 1：基本音调控制

### 示例介绍

本示例演示了如何控制蜂鸣器发出不同音调。

### 功能演示图片

![基本音调控制](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20buzzer%20component&image_size=landscape_16_9)

### 示例代码

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

## 示例 2：音乐播放

### 示例介绍

本示例演示了如何使用蜂鸣器播放内置音乐。

### 功能演示图片

![音乐播放](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20music%20playback%20status&image_size=landscape_16_9)

### 示例代码

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

## 示例 3：节拍控制

### 示例介绍

本示例演示了如何使用蜂鸣器的节拍控制功能。

### 功能演示图片

![节拍控制](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20musical%20notes%20with%20beats&image_size=landscape_16_9)

### 示例代码

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

## 示例 4：按钮控制蜂鸣器

### 示例介绍

本示例演示了如何通过按钮控制蜂鸣器的开关和音调。

### 功能演示图片

![按钮控制蜂鸣器](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20buttons%20and%20buzzer&image_size=landscape_16_9)

### 示例代码

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
