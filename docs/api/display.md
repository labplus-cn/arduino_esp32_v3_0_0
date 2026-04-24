# Display API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 DisplayFB 类的 API 参考。

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
