# DFRobot_Mindplus_MPython 库文档

本文档提供了 DFRobot_Mindplus_MPython 库的详细使用指南和 API 参考。

## 文档结构

- [API 参考文档](api_reference.md) - 完整的 API 参考
- [示例文档](examples.md) - 综合示例

### 按功能部件分类的文档

#### 显示相关
- [Display API](api/display.md) - 显示屏 API 参考
- [Display 示例](examples/display.md) - 显示屏使用示例

#### 灯光相关
- [RGB API](api/rgb.md) - RGB LED API 参考
- [RGB 示例](examples/rgb.md) - RGB LED 使用示例

#### 声音相关
- [Buzz API](api/buzz.md) - 蜂鸣器 API 参考
- [Buzz 示例](examples/buzz.md) - 蜂鸣器使用示例
- [Audio API](api/audio.md) - 音频模块 API 参考
- [Audio 示例](examples/audio.md) - 音频模块使用示例

#### 输入相关
- [Button API](api/button.md) - 按钮 API 参考
- [Button 示例](examples/button.md) - 按钮使用示例
- [TouchPad API](api/touchpad.md) - 触摸板 API 参考
- [TouchPad 示例](examples/touchpad.md) - 触摸板使用示例

#### 网络相关
- [WiFi API](api/wifi.md) - WiFi 模块 API 参考
- [WiFi 示例](examples/wifi.md) - WiFi 模块使用示例

#### 传感器相关
- [Sensors API](api/sensors.md) - 传感器 API 参考
- [Sensors 示例](examples/sensors.md) - 传感器使用示例

#### 存储相关
- [USBDISK API](api/usbdisk.md) - USB 存储 API 参考
- [USBDISK 示例](examples/usbdisk.md) - USB 存储使用示例

## 快速开始

1. 安装 DFRobot_Mindplus_MPython 库
2. 包含头文件：`#include <MPython.h>`
3. 初始化硬件：`mPython.begin();`
4. 开始使用各个模块的功能

## 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "Hello, mPython!", 0xFFFF);
  mPython.display.show();
  
  // 点亮 RGB LED
  mPython.rgb.begin();
  mPython.rgb.write(0, 255, 0, 0);
  
  // 蜂鸣器发声
  mPython.buzz.begin();
  mPython.buzz.freq(1000, BEAT_1);
}

void loop() {
  // 读取按钮状态
  if (mPython.buttonA.isPressed()) {
    mPython.display.drawText(20, 30, "Button A pressed!", 0x00FF00);
    mPython.display.show();
  }
  delay(100);
}
```
