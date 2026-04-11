#include <MPython.h>

void setup() {
  // 初始化MPython
  mPython.begin();
  
  // 设置按键回调函数
  mPython.buttonA.setPressedCallback(onButtonAPressed);
  mPython.buttonA.setUnPressedCallback(onButtonAReleased);
  
  mPython.buttonB.setPressedCallback(onButtonBPressed);
  mPython.buttonB.setUnPressedCallback(onButtonBReleased);
  
  mPython.buttonAB.setPressedCallback(onButtonABPressed);
  mPython.buttonAB.setUnPressedCallback(onButtonABReleased);
  
  Serial.begin(115200);
  Serial.println("Button example started");
}

void loop() {
  // 主循环不需要做任何事情，因为回调函数会在按键状态变化时被调用
  delay(100);
}

// 按键A按下回调
void onButtonAPressed() {
  Serial.println("Button A pressed");
  mPython.rgb.write(0, 255, 0, 0); // 点亮第一个LED为红色
}

// 按键A释放回调
void onButtonAReleased() {
  Serial.println("Button A released");
  mPython.rgb.write(0, 0, 0, 0); // 关闭第一个LED
}

// 按键B按下回调
void onButtonBPressed() {
  Serial.println("Button B pressed");
  mPython.rgb.write(1, 0, 255, 0); // 点亮第二个LED为绿色
}

// 按键B释放回调
void onButtonBReleased() {
  Serial.println("Button B released");
  mPython.rgb.write(1, 0, 0, 0); // 关闭第二个LED
}

// 按键AB按下回调
void onButtonABPressed() {
  Serial.println("Button AB pressed");
  mPython.rgb.write(2, 0, 0, 255); // 点亮第三个LED为蓝色
}

// 按键AB释放回调
void onButtonABReleased() {
  Serial.println("Button AB released");
  mPython.rgb.write(2, 0, 0, 0); // 关闭第三个LED
}