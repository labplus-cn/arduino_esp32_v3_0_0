#include <MPython.h>

bool lastTouched = false;

void setup() {
  // 初始化MPython
  mPython.begin();
  
  // 初始化串口
  Serial.begin(115200);
  Serial.println("TouchPad ReadValue example started!");
  

}

void loop() {
  // 读取触摸板P的原始值
  uint32_t value = mPython.touchPadP.read();
  Serial.print("TouchPad P value: ");
  Serial.println(value);
  
  // 跟阈值比较，确定是按下还是松开
  bool isTouched = (value > mPython.touchPadP.threshold);
  
  if (isTouched != lastTouched) {
    if (isTouched) {
      Serial.println("Touch P detected!");
      mPython.rgb.write(0, 255, 0, 0); // 点亮第一个LED为红色
    } else {
      Serial.println("Touch P released!");
      mPython.rgb.write(0, 0, 0, 0); // 关闭第一个LED
    }
    lastTouched = isTouched;
  }
  
  delay(100);
}