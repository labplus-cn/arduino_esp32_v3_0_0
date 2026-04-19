#include "MPython.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  mPython.begin();
  
  Serial.println("Initializing TTS...");
  if (!mPython.audio.ttsInit()) {
    Serial.println("Failed to initialize TTS!");
    while (1) delay(1000);
  }
  Serial.println("TTS initialized successfully");
  
  Serial.println("Testing TTS...");
  
  // 测试文本转语音
  mPython.audio.textToSpeech("你好，欢迎使用语音合成功能！");
  delay(5000); // 等待语音播放完成
  
  mPython.audio.textToSpeech("这是一个测试，用于演示语音合成的功能。");
  delay(5000); // 等待语音播放完成
  
  mPython.audio.textToSpeech("语音合成功能已成功集成到 Audio 类中。");
  delay(5000); // 等待语音播放完成
  
  Serial.println("TTS test completed");
}

void loop() {
  // 循环中可以添加其他操作
  delay(1000);
}