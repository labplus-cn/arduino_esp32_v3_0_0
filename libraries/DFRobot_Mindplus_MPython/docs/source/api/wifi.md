# WiFi API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 MPythonWiFi 类的 API 参考。

## MPythonWiFi 类

提供 WiFi 连接和管理功能。

### 构造函数

```cpp
MPythonWiFi();
```

初始化 MPythonWiFi 实例。

### 方法

```cpp
bool connect(const char *ssid, const char *password, uint32_t timeoutMs = 15000);
```

连接到 WiFi 网络（带密码）。

**参数**：
- ssid：SSID
- password：密码
- timeoutMs：超时时间

**返回值**：连接结果。

```cpp
bool connect(const char *ssid, uint32_t timeoutMs = 15000);
```

连接到 WiFi 网络（无密码）。

**参数**：
- ssid：SSID
- timeoutMs：超时时间

**返回值**：连接结果。

```cpp
bool reconnect(uint32_t timeoutMs = 15000);
```

重新连接。

**参数**：
- timeoutMs：超时时间

**返回值**：连接结果。

```cpp
void disconnect(bool wifiOff = false);
```

断开连接。

**参数**：
- wifiOff：是否关闭 WiFi

```cpp
bool setAutoReconnect(bool enable);
```

设置自动重连。

**参数**：
- enable：是否启用

**返回值**：设置结果。

```cpp
bool autoReconnect() const;
```

获取自动重连状态。

**返回值**：自动重连状态。

```cpp
int16_t scanNetworks(bool async = false, bool showHidden = false);
```

扫描网络。

**参数**：
- async：是否异步
- showHidden：是否显示隐藏网络

**返回值**：扫描到的网络数量。

```cpp
void clearScanResults();
```

清除扫描结果。

```cpp
wl_status_t status() const;
```

获取连接状态。

**返回值**：连接状态。

```cpp
bool isConnected() const;
```

检查是否已连接。

**返回值**：连接状态。

```cpp
String ssid() const;
```

获取当前连接的 SSID。

**返回值**：SSID。

```cpp
String ssid(int32_t index) const;
```

获取指定索引的 SSID。

**参数**：
- index：索引

**返回值**：SSID。

```cpp
String bssid() const;
```

获取当前连接的 BSSID。

**返回值**：BSSID。

```cpp
int32_t rssi() const;
```

获取当前连接的 RSSI。

**返回值**：RSSI。

```cpp
int32_t rssi(int32_t index) const;
```

获取指定索引的 RSSI。

**参数**：
- index：索引

**返回值**：RSSI。

```cpp
String encryptionType(int32_t index) const;
```

获取指定索引的加密类型。

**参数**：
- index：索引

**返回值**：加密类型。

```cpp
String localIP() const;
```

获取本地 IP。

**返回值**：本地 IP。

```cpp
String gatewayIP() const;
```

获取网关 IP。

**返回值**：网关 IP。

```cpp
String subnetMask() const;
```

获取子网掩码。

**返回值**：子网掩码。

```cpp
String dnsIP(uint8_t index = 0) const;
```

获取 DNS IP。

**参数**：
- index：索引

**返回值**：DNS IP。

```cpp
String macAddress() const;
```

获取 MAC 地址。

**返回值**：MAC 地址。
