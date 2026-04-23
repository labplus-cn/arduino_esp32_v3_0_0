# WiFi 示例

本文档提供了 DFRobot_Mindplus_MPython 库中 MPythonWiFi 类的使用示例。

## 示例 1：WiFi 连接

### 示例介绍

本示例演示了如何连接到 WiFi 网络并获取网络信息。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "WiFi 连接演示", 0xFFFF);
  mPython.display.show();
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 40, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 40, "连接成功！", 0x00FF00);
    mPython.display.drawText(20, 60, String("SSID: " + mPython.wifi.ssid()), 0xFFFF);
    mPython.display.drawText(20, 80, String("IP: " + mPython.wifi.localIP()), 0xFFFF);
    mPython.display.drawText(20, 100, String("RSSI: " + String(mPython.wifi.rssi()) + " dBm"), 0xFFFF);
    mPython.display.drawText(20, 120, String("MAC: " + mPython.wifi.macAddress()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 2：WiFi 扫描

### 示例介绍

本示例演示了如何扫描周围的 WiFi 网络。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "WiFi 扫描演示", 0xFFFF);
  mPython.display.show();
  
  // 扫描网络
  mPython.display.drawText(20, 40, "正在扫描网络...", 0xFFFF);
  mPython.display.show();
  
  int networkCount = mPython.wifi.scanNetworks();
  
  mPython.display.drawText(20, 40, String("发现 " + String(networkCount) + " 个网络"), 0xFFFF);
  
  // 显示前5个网络
  for (int i = 0; i < min(5, networkCount); i++) {
    String ssid = mPython.wifi.ssid(i);
    int32_t rssi = mPython.wifi.rssi(i);
    String encType = mPython.wifi.encryptionType(i);
    
    mPython.display.drawText(20, 60 + i * 30, String(ssid), 0xFFFF);
    mPython.display.drawText(200, 60 + i * 30, String(String(rssi) + " dBm"), 0xFFFF);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 3：网络信息获取

### 示例介绍

本示例演示了如何获取详细的网络信息。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "网络信息演示", 0xFFFF);
  mPython.display.show();
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 40, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 40, "连接成功！", 0x00FF00);
    
    // 获取网络信息
    mPython.display.drawText(20, 60, String("SSID: " + mPython.wifi.ssid()), 0xFFFF);
    mPython.display.drawText(20, 80, String("BSSID: " + mPython.wifi.bssid()), 0xFFFF);
    mPython.display.drawText(20, 100, String("本地 IP: " + mPython.wifi.localIP()), 0xFFFF);
    mPython.display.drawText(20, 120, String("网关 IP: " + mPython.wifi.gatewayIP()), 0xFFFF);
    mPython.display.drawText(20, 140, String("子网掩码: " + mPython.wifi.subnetMask()), 0xFFFF);
    mPython.display.drawText(20, 160, String("DNS IP: " + mPython.wifi.dnsIP()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 40, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```

## 示例 4：自动重连设置

### 示例介绍

本示例演示了如何设置 WiFi 自动重连功能。

### 示例代码

```cpp
#include <MPython.h>

void setup() {
  // 初始化 mPython 硬件
  mPython.begin();
  
  // 清空屏幕
  mPython.display.fillScreen(0x0000);
  mPython.display.drawText(20, 10, "自动重连设置", 0xFFFF);
  mPython.display.show();
  
  // 设置自动重连
  bool autoReconnect = mPython.wifi.setAutoReconnect(true);
  
  if (autoReconnect) {
    mPython.display.drawText(20, 40, "自动重连已启用", 0x00FF00);
  } else {
    mPython.display.drawText(20, 40, "自动重连设置失败", 0xFF0000);
  }
  
  // 显示当前自动重连状态
  bool currentStatus = mPython.wifi.autoReconnect();
  mPython.display.drawText(20, 60, String("当前状态: " + String(currentStatus ? "启用" : "禁用")), 0xFFFF);
  
  // 尝试连接到 WiFi
  mPython.display.drawText(20, 80, "正在连接 WiFi...", 0xFFFF);
  mPython.display.show();
  
  bool connected = mPython.wifi.connect("您的WiFi名称", "您的WiFi密码");
  
  if (connected) {
    mPython.display.drawText(20, 80, "连接成功！", 0x00FF00);
    mPython.display.drawText(20, 100, String("IP: " + mPython.wifi.localIP()), 0xFFFF);
  } else {
    mPython.display.drawText(20, 80, "连接失败！", 0xFF0000);
  }
  
  mPython.display.show();
}

void loop() {
  // 保持程序运行
  delay(1000);
}
```
