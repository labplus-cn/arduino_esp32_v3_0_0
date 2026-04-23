# RGB API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 RGB 类的 API 参考。

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
