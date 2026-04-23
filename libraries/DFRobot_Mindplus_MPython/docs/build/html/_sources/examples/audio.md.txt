# Audio 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 Audio 类的使用示例。

## 示例 1：音频播放

### 示例介绍

本示例演示了如何使用 Audio 类播放音频文件。

### 示例代码

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

## 示例 2：录音功能

### 示例介绍

本示例演示了如何使用 Audio 类进行录音。

### 示例代码

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

## 示例 3：语音合成 (TTS)

### 示例介绍

本示例演示了如何使用 Audio 类的语音合成功能。

### 示例代码

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

## 示例 4：语音识别

### 示例介绍

本示例演示了如何使用 Audio 类的语音识别功能。

### 示例代码

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
