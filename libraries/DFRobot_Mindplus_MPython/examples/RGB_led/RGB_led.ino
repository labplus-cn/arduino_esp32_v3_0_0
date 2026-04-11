#include <MPython.h>

void setup() {
  // 初始化mPython
  mPython.begin();
  
  // 打印测试信息
  Serial.begin(115200);
  Serial.println("RGB LED Test");
  Serial.println("Testing RGB LED functionality...");
}

void loop() {
  // 测试1: 点亮所有LED为红色
  Serial.println("Test 1: All LEDs Red");
  mPython.rgb.write(-1, 255, 0, 0);
  delay(1000);
  
  // 测试2: 点亮所有LED为绿色
  Serial.println("Test 2: All LEDs Green");
  mPython.rgb.write(-1, 0, 255, 0);
  delay(1000);
  
  // 测试3: 点亮所有LED为蓝色
  Serial.println("Test 3: All LEDs Blue");
  mPython.rgb.write(-1, 0, 0, 255);
  delay(1000);
  
  // 测试4: 点亮所有LED为白色
  Serial.println("Test 4: All LEDs White");
  mPython.rgb.write(-1, 255, 255, 255);
  delay(1000);
  
  // 测试5: 测试单个LED
  Serial.println("Test 5: Individual LEDs");
  mPython.rgb.write(0, 255, 0, 0);  // 第一个LED为红色
  mPython.rgb.write(1, 0, 255, 0);  // 第二个LED为绿色
  mPython.rgb.write(2, 0, 0, 255);  // 第三个LED为蓝色
  delay(1000);
  
  // 测试6: 测试亮度调节
  Serial.println("Test 6: Brightness Adjustment");
  for (int i = 0; i <= 9; i++) {
    mPython.rgb.brightness(i);
    Serial.print("Brightness level: ");
    Serial.println(i);
    mPython.rgb.write(-1, 255, 255, 255);
    delay(300);
  }
  
  // 测试7: 彩虹效果
  Serial.println("Test 7: Rainbow Effect");
  for (int i = 0; i < 255; i++) {
    uint32_t color = wheel(i);
    mPython.rgb.write(-1, color);
    delay(10);
  }
  
  // 测试8: 清除所有LED
  Serial.println("Test 8: Clear All LEDs");
  mPython.rgb.write(-1, 0, 0, 0);
  delay(1000);
  
  // 测试完成，等待一段时间后重复
  Serial.println("Test complete. Waiting for next cycle...");
  delay(2000);
}

// 彩虹颜色函数
uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return (255 - pos * 3) << 16 | pos * 3;
  }
  if (pos < 170) {
    pos -= 85;
    return (255 - pos * 3) << 8 | pos * 3;
  }
  pos -= 170;
  return pos * 3 << 16 | (255 - pos * 3);
}