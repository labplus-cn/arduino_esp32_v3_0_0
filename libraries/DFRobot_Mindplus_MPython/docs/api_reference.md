# DFRobot_Mindplus_MPython 库 API 参考文档

本文档提供了 DFRobot_Mindplus_MPython 库的完整 API 参考，包括所有功能部件的使用方法和示例。

## 目录

- [MPython 类](#mpython-类)
- [DisplayFB 类](#displayfb-类)
- [RGB 类](#rgb-类)
- [Buzz 类](#buzz-类)
- [Audio 类](#audio-类)
- [Button 类](#button-类)
- [TouchPad 类](#touchpad-类)
- [USBDISK 类](#usbdisk-类)
- [MPythonWiFi 类](#mpythonwifi-类)
- [传感器类](#传感器类)
  - [Accelerometer 类](#accelerometer-类)
  - [Magnetometer 类](#magnetometer-类)
  - [LightSensor 类](#lightsensor-类)

## MPython 类

主控类，提供对所有硬件模块的访问。

### 构造函数

```cpp
MPython();
```

初始化 MPython 实例。

### 方法

```cpp
void begin(void);
```

初始化所有硬件模块。

### 成员变量

| 成员变量 | 类型 | 描述 |
|---------|------|------|
| display | DisplayFB | 显示屏对象 |
| rgb | RGB | RGB LED 对象 |
| buzz | Buzz | 蜂鸣器对象 |
| audio | Audio | 音频对象 |
| buttonA | Button | 按钮 A 对象 |
| buttonB | Button | 按钮 B 对象 |
| buttonAB | Button | 按钮 AB 对象 |
| touchPadP | TouchPad | 触摸板 P 对象 |
| touchPadY | TouchPad | 触摸板 Y 对象 |
| touchPadT | TouchPad | 触摸板 T 对象 |
| touchPadH | TouchPad | 触摸板 H 对象 |
| touchPadO | TouchPad | 触摸板 O 对象 |
| touchPadN | TouchPad | 触摸板 N 对象 |
| usbDisk | USBDISK | USB 磁盘对象 |
| wifi | MPythonWiFi | WiFi 对象 |
| accelerometer | Accelerometer | 加速度计对象 |
| magnetometer | Magnetometer | 磁力计对象 |
| lightSensor | LightSensor | 光线传感器对象 |

## DisplayFB 类

提供显示屏的基本绘图功能，包括文本、图形等绘制操作。

### 构造函数

```cpp
DisplayFB();
```

初始化 DisplayFB 实例。

### 方法

```cpp
esp_err_t begin();
```

初始化显示屏硬件。

**返回值**：初始化结果，ESP_OK 表示成功。

```cpp
void fillScreen(uint16_t color);
```

用指定颜色填充整个屏幕。

**参数**：
- color：填充颜色，格式为 RGB565

```cpp
void clearLine(int line);
```

用背景色填充指定行。

**参数**：
- line：行号，从0开始

```cpp
void drawText(int x, int y, const char *text, uint16_t color);
```

在指定位置绘制ASCII文本。

**参数**：
- x：X坐标
- y：Y坐标
- text：要绘制的文本
- color：文本颜色，格式为 RGB565

```cpp
void drawTextCN(int x, int y, const char *text, uint16_t color, bool wrap = true);
```

在指定位置绘制中文字符。

**参数**：
- x：X坐标
- y：Y坐标
- text：要绘制的中文文本
- color：文本颜色，格式为 RGB565
- wrap：是否自动换行

```cpp
void drawTextCN(int line, const char *text, uint16_t color, bool wrap = true);
```

按行绘制中文字符，行起始x=10，第一行y=10，行高30。

**参数**：
- line：行号，从0开始
- text：要绘制的中文文本
- color：文本颜色，格式为 RGB565
- wrap：是否自动换行

```cpp
void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
```

绘制从(x0,y0)到(x1,y1)的直线。

**参数**：
- x0：起点X坐标
- y0：起点Y坐标
- x1：终点X坐标
- y1：终点Y坐标
- color：直线颜色，格式为 RGB565

```cpp
void drawRect(int x, int y, int width, int height, uint16_t color);
```

绘制指定大小的矩形边框。

**参数**：
- x：左上角X坐标
- y：左上角Y坐标
- width：矩形宽度
- height：矩形高度
- color：边框颜色，格式为 RGB565

```cpp
void fillRect(int x, int y, int width, int height, uint16_t color);
```

用指定颜色填充矩形。

**参数**：
- x：左上角X坐标
- y：左上角Y坐标
- width：矩形宽度
- height：矩形高度
- color：填充颜色，格式为 RGB565

```cpp
void drawCircle(int x0, int y0, int radius, uint16_t color);
```

绘制指定半径的圆形边框。

**参数**：
- x0：圆心X坐标
- y0：圆心Y坐标
- radius：圆的半径
- color：边框颜色，格式为 RGB565

```cpp
void fillCircle(int x0, int y0, int radius, uint16_t color);
```

用指定颜色填充圆形。

**参数**：
- x0：圆心X坐标
- y0：圆心Y坐标
- radius：圆的半径
- color：填充颜色，格式为 RGB565

```cpp
void showLogo();
```

显示设备logo。

```cpp
void drawQRCode(int x, int y, const char *text, int scale = 3);
```

在指定位置显示二维码。

**参数**：
- x：左上角X坐标
- y：左上角Y坐标
- text：二维码内容
- scale：每个模块的像素大小

```cpp
void show();
```

将内部帧缓冲区的内容显示到屏幕上。

## RGB 类

控制 RGB LED 灯的亮度和颜色。

### 构造函数

```cpp
RGB(uint8_t pin = 8, uint16_t numPixels = 3);
```

初始化 RGB 实例。

**参数**：
- pin：RGB 引脚
- numPixels：LED 数量

### 方法

```cpp
void begin();
```

初始化 RGB 硬件。

```cpp
void write(int8_t index, uint8_t r, uint8_t g, uint8_t b);
```

设置指定索引的 LED 颜色。

**参数**：
- index：LED 索引
- r：红色分量 (0-255)
- g：绿色分量 (0-255)
- b：蓝色分量 (0-255)

```cpp
void write(int8_t index, uint32_t color);
```

设置指定索引的 LED 颜色。

**参数**：
- index：LED 索引
- color：颜色值，格式为 0xRRGGBB

```cpp
void brightness(uint8_t b);
```

设置亮度。

**参数**：
- b：亮度值 (0-255)

```cpp
uint8_t brightness();
```

获取当前亮度。

**返回值**：当前亮度值。

## Buzz 类

控制蜂鸣器发声，包括播放音调、音乐等功能。

### 构造函数

```cpp
Buzz();
```

初始化 Buzz 实例。

### 方法

```cpp
void begin(uint8_t pin = 21);
```

初始化蜂鸣器硬件。

**参数**：
- pin：蜂鸣器引脚

```cpp
void on(void);
```

启动蜂鸣器。

```cpp
void off(void);
```

停止蜂鸣器发声。

```cpp
bool isOn(void);
```

检查蜂鸣器是否开启。

**返回值**：蜂鸣器状态。

```cpp
void setTicksTempo(uint32_t _ticks, uint32_t _tempo);
```

设置节拍和 tempo。

**参数**：
- _ticks：节拍数
- _tempo：速度

```cpp
void freq(uint32_t _freq = 500);
```

设置频率。

**参数**：
- _freq：频率值

```cpp
void freq(uint32_t _freq, Beat beat);
```

设置频率和节拍。

**参数**：
- _freq：频率值
- beat：节拍

```cpp
void stop();
```

停止当前播放的音乐或音调。

```cpp
void play(Music music, MelodyOptions options = Once);
```

播放音乐。

**参数**：
- music：音乐曲目
- options：播放选项

### 枚举

#### Beat

| 值 | 描述 |
|-----|------|
| BEAT_1 | 1拍 |
| BEAT_1_2 | 1/2拍 |
| BEAT_1_4 | 1/4拍 |
| BEAT_3_4 | 3/4拍 |
| BEAT_3_2 | 3/2拍 |
| BEAT_2 | 2拍 |
| BEAT_3 | 3拍 |
| BEAT_4 | 4拍 |

#### Music

| 值 | 描述 |
|-----|------|
| DADADADUM | 贝多芬第五交响曲开头 |
| ENTERTAINER | 艺人 |
| PRELUDE | 前奏曲 |
| ODE | 颂歌 |
| NYAN | 彩虹猫 |
| RINGTONE | 铃声 |
| FUNK | 放克 |
| BLUES | 蓝调 |
| BIRTHDAY | 生日快乐 |
| WEDDING | 婚礼进行曲 |
| FUNERAL | 葬礼进行曲 |
| PUNCHLINE | 笑点 |
| BADDY | 反派 |
| CHASE | 追逐 |
| BA_DING | 叮 |
| WAWAWAWAA | 哇哇哇哇 |
| JUMP_UP | 跳上 |
| JUMP_DOWN | 跳下 |
| POWER_UP | power up |
| POWER_DOWN | power down |

#### MelodyOptions

| 值 | 描述 |
|-----|------|
| Once | 播放一次 |
| Forever | 循环播放 |
| OnceInBackground | 在后台播放一次 |
| ForeverInBackground | 在后台循环播放 |

## Audio 类

提供音频播放、录音、语音合成和语音识别功能。

### 构造函数

```cpp
Audio();
```

初始化 Audio 实例。

### 析构函数

```cpp
~Audio();
```

清理 Audio 实例。

### 方法

```cpp
bool begin();
```

初始化音频硬件。

**返回值**：初始化结果。

```cpp
bool setVolume(uint8_t volume);
```

设置音量。

**参数**：
- volume：音量值 (0-100)

**返回值**：设置结果。

```cpp
bool startRecord(const char *path, uint32_t sampleRate = 16000, uint8_t bitsPerSample = 16, uint8_t channels = 1);
```

开始录音。

**参数**：
- path：录音文件路径
- sampleRate：采样率
- bitsPerSample：位宽
- channels：声道数

**返回值**：操作结果。

```cpp
bool startRecord(const char *path, uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels, uint32_t durationMs);
```

开始录音（指定时长）。

**参数**：
- path：录音文件路径
- sampleRate：采样率
- bitsPerSample：位宽
- channels：声道数
- durationMs：录音时长（毫秒）

**返回值**：操作结果。

```cpp
void stopRecord();
```

停止录音。

```cpp
bool play(const char *path);
```

播放音频。

**参数**：
- path：音频文件路径

**返回值**：操作结果。

```cpp
bool pause();
```

暂停播放。

**返回值**：操作结果。

```cpp
bool resume();
```

恢复播放。

**返回值**：操作结果。

```cpp
void stop();
```

停止播放。

```cpp
PlayState state() const;
```

获取播放状态。

**返回值**：播放状态。

```cpp
bool ttsInit();
```

初始化语音合成。

**返回值**：初始化结果。

```cpp
bool textToSpeech(const char *text);
```

文本转语音。

**参数**：
- text：要转换的文本

**返回值**：操作结果。

```cpp
bool srBegin(const char *wakeupReplyTts = nullptr, uint16_t multinetTimeoutMs = 6000, bool loadCommandsFromSdkconfig = false);
```

初始化语音识别。

**参数**：
- wakeupReplyTts：唤醒后可选播放的 UTF-8 文本（需先 ttsInit）；nullptr 则不播
- multinetTimeoutMs：MultiNet 命令识别超时（毫秒）
- loadCommandsFromSdkconfig：为 true 时从 sdkconfig 加载命令表（多数 Arduino 工程无此项，常为 false）

**返回值**：操作结果。

```cpp
bool srAddCommand(int commandId, const char *phraseUtf8);
```

添加语音命令。

**参数**：
- commandId：命令 ID
- phraseUtf8：命令短语（UTF-8）

**返回值**：操作结果。

```cpp
bool srClearCommands();
```

清除所有语音命令。

**返回值**：操作结果。

```cpp
bool srApplyCommands();
```

应用语音命令。

**返回值**：操作结果。

```cpp
int srGetCommandId();
```

获取识别到的命令 ID。

**返回值**：命令 ID。

### 枚举

#### PlayState

| 值 | 描述 |
|-----|------|
| PLAY_STATE_IDLE | 空闲状态 |
| PLAY_STATE_PLAYING | 播放中 |
| PLAY_STATE_PAUSED | 暂停 |
| PLAY_STATE_STOPPED | 已停止 |
| PLAY_STATE_ERROR | 错误 |

## Button 类

处理按钮的状态检测和回调。

### 构造函数

```cpp
Button(uint8_t _io);
```

初始化单按钮实例。

**参数**：
- _io：按钮引脚

```cpp
Button(uint8_t _io1, uint8_t _io2);
```

初始化双按钮实例。

**参数**：
- _io1：第一个按钮引脚
- _io2：第二个按钮引脚

### 方法

```cpp
bool isPressed(void);
```

检查按钮是否按下。

**返回值**：按钮状态。

```cpp
void setPressedCallback(CBFunc _cb);
```

设置按下回调函数。

**参数**：
- _cb：按下回调函数

```cpp
void setUnPressedCallback(CBFunc _cb);
```

设置释放回调函数。

**参数**：
- _cb：释放回调函数

## TouchPad 类

处理触摸板的状态检测和回调。

### 构造函数

```cpp
TouchPad(uint8_t _io);
```

初始化触摸板实例。

**参数**：
- _io：触摸板引脚

### 方法

```cpp
bool isTouched(void);
```

检查是否触摸。

**返回值**：触摸状态。

```cpp
void setTouchedCallback(CBFunc _cb);
```

设置触摸回调函数。

**参数**：
- _cb：触摸回调函数

```cpp
void setUnTouchedCallback(CBFunc _cb);
```

设置未触摸回调函数。

**参数**：
- _cb：未触摸回调函数

```cpp
uint32_t read(void);
```

读取触摸值。

**返回值**：触摸值。

### 成员变量

| 成员变量 | 类型 | 描述 |
|---------|------|------|
| threshold | uint16_t | 触摸阈值 |
| touchedCb | CBFunc | 触摸回调函数 |
| untouchedCb | CBFunc | 未触摸回调函数 |
| touchDetected | bool | 触摸检测标志位 |
| io | uint8_t | 触摸板引脚 |

## USBDISK 类

将 flash 分区通过 USB MSC 暴露为 U 盘。

### 构造函数

```cpp
USBDISK();
```

初始化 USBDISK 实例。

### 方法

```cpp
bool begin(const char *partitionLabel = "spiffs", const char *vendorID = "mPython", const char *productID = "Flash Disk", const char *revision = "1.0");
```

初始化并启动 USB MSC。

**参数**：
- partitionLabel：分区名（默认 "spiffs"）
- vendorID：厂商 ID
- productID：产品 ID
- revision：版本号

**返回值**：初始化结果。

```cpp
void end();
```

停止 USB 存储功能。

```cpp
bool mounted() const;
```

检查是否已挂载。

**返回值**：挂载状态。

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

## 传感器类

### Accelerometer 类

提供加速度数据读取和手势检测功能。

#### 构造函数

```cpp
Accelerometer();
```

初始化加速度计实例。

#### 方法

```cpp
void begin(void);
```

初始化加速度计硬件。

```cpp
float getX(void);
```

获取 X 轴加速度。

**返回值**：X 轴加速度值。

```cpp
float getY(void);
```

获取 Y 轴加速度。

**返回值**：Y 轴加速度值。

```cpp
float getZ(void);
```

获取 Z 轴加速度。

**返回值**：Z 轴加速度值。

```cpp
void setOffset(int x, int y, int z);
```

设置偏移。

**参数**：
- x：X 轴偏移
- y：Y 轴偏移
- z：Z 轴偏移

```cpp
float getStrength(void);
```

获取加速度强度。

**返回值**：加速度强度。

```cpp
void onGesture(Gesture gesture, mpythonGestureHandlePtr body);
```

设置手势回调。

**参数**：
- gesture：手势类型
- body：回调函数

```cpp
bool isGesture(Gesture gesture);
```

检查是否为指定手势。

**参数**：
- gesture：手势类型

**返回值**：是否为指定手势。

#### 枚举

##### Gesture

| 值 | 描述 |
|-----|------|
| Shake | 摇晃 |
| ScreenUp | 屏幕朝上 |
| ScreenDown | 屏幕朝下 |
| TiltLeft | 向左倾斜 |
| TiltRight | 向右倾斜 |
| TiltForward | 向前倾斜 |
| TiltBack | 向后倾斜 |
| GestureNone | 无手势 |

### Magnetometer 类

提供磁场数据读取功能。

#### 构造函数

```cpp
Magnetometer();
```

初始化磁力计实例。

#### 方法

```cpp
void begin(void);
```

初始化磁力计硬件。

```cpp
float getX(void);
```

获取 X 轴磁场。

**返回值**：X 轴磁场值。

```cpp
float getY(void);
```

获取 Y 轴磁场。

**返回值**：Y 轴磁场值。

```cpp
float getZ(void);
```

获取 Z 轴磁场。

**返回值**：Z 轴磁场值。

```cpp
void setOffset(int x, int y, int z);
```

设置偏移。

**参数**：
- x：X 轴偏移
- y：Y 轴偏移
- z：Z 轴偏移

### LightSensor 类

提供环境光强度读取功能。

#### 构造函数

```cpp
LightSensor();
```

初始化光线传感器实例。

#### 方法

```cpp
void begin(void);
```

初始化光线传感器硬件。

```cpp
uint16_t getLux(void);
```

获取光照强度。

**返回值**：光照强度值（单位：lux）。
