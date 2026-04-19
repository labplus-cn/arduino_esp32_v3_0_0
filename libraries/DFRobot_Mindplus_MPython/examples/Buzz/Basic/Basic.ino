#include <MPython.h>

void setup() {
  // 初始化MPython
  mPython.begin();

  // 初始化串口
  Serial.begin(115200);
  Serial.println("Buzz example started!");
}

void loop() {
  // 播放一个频率
  Serial.println("Playing 500Hz for 1 second");
  mPython.buzz.freq(500);
  mPython.buzz.on();
  delay(1000);
  mPython.buzz.off();
  delay(1000);
  
  // 播放另一个频率
  Serial.println("Playing 1000Hz for 1 second");
  mPython.buzz.freq(1000);
  mPython.buzz.on();
  delay(1000);
  mPython.buzz.off();
  delay(1000);
  
  // 播放预设旋律
  Serial.println("Playing birthday melody");
  mPython.buzz.play(BIRTHDAY);
  delay(5000);
  
  // 播放另一个预设旋律
  Serial.println("Playing entertainer melody");
  mPython.buzz.play(ENTERTAINER);
  delay(5000);
}