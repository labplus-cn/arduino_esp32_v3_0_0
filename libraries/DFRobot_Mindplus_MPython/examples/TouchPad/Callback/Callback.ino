#include <MPython.h>

// 触摸回调函数
void onTouchP() {
  Serial.println("Touch P detected!");
  mPython.rgb.write(0, 255, 0, 0); // 点亮第一个LED为红色
}

// 释放回调函数
void onReleaseP() {
  Serial.println("Touch P released!");
  mPython.rgb.write(0, 0, 0, 0); // 关闭第一个LED
}

void setup() {
  // 初始化MPython
  mPython.begin();
  
  // 初始化串口
  Serial.begin(115200);
  Serial.println("TouchPad Callback example started!");
  
  // 设置触摸板P的回调函数
  mPython.touchPadP.setTouchedCallback(onTouchP);
  mPython.touchPadP.setUnTouchedCallback(onReleaseP);
  

}

void loop() {
  // 什么都不做，只演示回调功能
  delay(100);
}