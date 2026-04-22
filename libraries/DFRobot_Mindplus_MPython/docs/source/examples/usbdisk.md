# USBDISK 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 USBDISK 类的使用示例。

## 示例 1：基本 USB 存储功能

### 示例介绍

本示例演示了如何启用 USB 存储功能，将 mPython 掌控板作为 U 盘使用。

### 功能演示图片

![USB 存储功能](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20connected%20to%20computer%20as%20USB%20drive&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "USB 存储演示", 0xFFFF);
  mPython.display.show();
  
  // 启动 USB 存储功能
  mPython.display.drawText(20, 40, "正在启动 USB 存储...", 0xFFFF);
  mPython.display.show();
  
  bool success = mPython.usbDisk.begin();
  
  if (success) {
    mPython.display.drawText(20, 40, "USB 存储已启动", 0x00FF00);
    mPython.display.drawText(20, 60, "请连接到电脑", 0xFFFF);
    mPython.display.drawText(20, 80, "作为 U 盘使用", 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "USB 存储启动失败", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 检查 USB 存储状态
  if (mPython.usbDisk.mounted()) {
    mPython.display.drawText(20, 100, "状态: 已挂载", 0x00FF00);
  } else {
    mPython.display.drawText(20, 100, "状态: 未挂载", 0xFFFF);
  }
  
  mPython.display.show();
  delay(1000);
}
```

## 示例 2：自定义 USB 存储配置

### 示例介绍

本示例演示了如何使用自定义配置启动 USB 存储功能。

### 功能演示图片

![自定义 USB 存储配置](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20custom%20USB%20storage%20configuration&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "USB 存储配置", 0xFFFF);
  mPython.display.show();
  
  // 启动 USB 存储功能（自定义配置）
  mPython.display.drawText(20, 40, "正在启动 USB 存储...", 0xFFFF);
  mPython.display.show();
  
  bool success = mPython.usbDisk.begin(
    "spiffs",  // 分区名
    "DFRobot", // 厂商 ID
    "mPython Disk", // 产品 ID
    "2.0"      // 版本号
  );
  
  if (success) {
    mPython.display.drawText(20, 40, "USB 存储已启动", 0x00FF00);
    mPython.display.drawText(20, 60, "厂商: DFRobot", 0xFFFF);
    mPython.display.drawText(20, 80, "产品: mPython Disk", 0xFFFF);
    mPython.display.drawText(20, 100, "版本: 2.0", 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "USB 存储启动失败", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 3：USB 存储状态监控

### 示例介绍

本示例演示了如何监控 USB 存储的挂载状态。

### 功能演示图片

![USB 存储状态监控](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20display%20showing%20USB%20storage%20status&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "USB 存储监控", 0xFFFF);
  mPython.display.show();
  
  // 启动 USB 存储功能
  mPython.usbDisk.begin();
  
  mPython.display.drawText(20, 40, "USB 存储已启动", 0x00FF00);
  mPython.display.show();
}

void loop() {
  // 检查 USB 存储状态
  if (mPython.usbDisk.mounted()) {
    mPython.display.drawText(20, 60, "状态: 已挂载", 0x00FF00);
    mPython.rgb.write(0, 0, 255, 0); // 绿色
  } else {
    mPython.display.drawText(20, 60, "状态: 未挂载", 0xFFFF);
    mPython.rgb.write(0, 255, 0, 0); // 红色
  }
  
  mPython.display.show();
  delay(1000);
}
```

## 示例 4：USB 存储与文件操作

### 示例介绍

本示例演示了如何结合 USB 存储功能进行文件操作。

### 功能演示图片

![USB 存储与文件操作](https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=mPython%20board%20with%20file%20operations%20interface&image_size=landscape_16_9)

### 示例代码

```cpp
#include <MPython.h>
#include <FS.h>
#include <SPIFFS.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 初始化 SPIFFS
  if (!SPIFFS.begin()) {
    mPython.display.fillScreen(0x0000);
    mPython.display.drawText(20, 10, "SPIFFS 初始化失败", 0xFF0000);
    mPython.display.show();
    return;
  }
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "USB 存储与文件操作", 0xFFFF);
  mPython.display.show();
  
  // 创建测试文件
  File file = SPIFFS.open("/test.txt", "w");
  if (file) {
    file.println("Hello, mPython!");
    file.println("This is a test file.");
    file.close();
    mPython.display.drawText(20, 40, "测试文件已创建", 0x00FF00);
  } else {
    mPython.display.drawText(20, 40, "创建文件失败", 0xFF0000);
  }
  
  // 启动 USB 存储功能
  mPython.usbDisk.begin();
  mPython.display.drawText(20, 60, "USB 存储已启动", 0x00FF00);
  mPython.display.drawText(20, 80, "请连接到电脑查看文件", 0xFFFF);
  mPython.display.show();
}

void loop() {
  // 检查 USB 存储状态
  if (mPython.usbDisk.mounted()) {
    mPython.display.drawText(20, 100, "状态: 已挂载", 0x00FF00);
  } else {
    mPython.display.drawText(20, 100, "状态: 未挂载", 0xFFFF);
  }
  
  mPython.display.show();
  delay(1000);
}
```
