#include <MPython.h>

void setup() {
  // 初始化MPython库
  mPython.begin();
  
  // 显示欢迎信息
  mPython.display.clearDisplay();
  mPython.display.setCursor(0, 0);
  mPython.display.print("Hello, MPython!");
  mPython.display.display();
  
  // 点亮RGB LED为蓝色
  mPython.rgb.write(-1, 0, 0, 255);
  
  // 播放欢迎音效
  mPython.buzz.play(BIRTHDAY, Once);
}

void loop() {
  // 读取按钮状态
  if (mPython.buttonA.isPressed()) {
    // 按钮A被按下，点亮RGB LED为红色
    mPython.rgb.write(-1, 255, 0, 0);
    mPython.display.clearDisplay();
    mPython.display.setCursor(0, 0);
    mPython.display.print("Button A pressed!");
    mPython.display.display();
    delay(500);
  } else if (mPython.buttonB.isPressed()) {
    // 按钮B被按下，点亮RGB LED为绿色
    mPython.rgb.write(-1, 0, 255, 0);
    mPython.display.clearDisplay();
    mPython.display.setCursor(0, 0);
    mPython.display.print("Button B pressed!");
    mPython.display.display();
    delay(500);
  } else {
    // 按钮未被按下，恢复蓝色
    mPython.rgb.write(-1, 0, 0, 255);
  }
  
  // 读取触摸板状态
  if (mPython.touchPadP.isTouched()) {
    mPython.display.clearDisplay();
    mPython.display.setCursor(0, 0);
    mPython.display.print("Touch P detected!");
    mPython.display.display();
    mPython.buzz.freq(1000, 200);
  }
  
  // 读取加速度传感器数据
  float x = mPython.accelerometer.getX();
  float y = mPython.accelerometer.getY();
  float z = mPython.accelerometer.getZ();
  
  // 读取光线传感器数据
  uint16_t lux = mPython.lightSensor.getLux();
  
  // 在串口输出传感器数据
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.print(z);
  Serial.print(" Lux: ");
  Serial.println(lux);
  
  delay(100);
}