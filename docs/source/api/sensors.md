# Sensors API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中传感器类的 API 参考。

## Accelerometer 类

提供加速度数据读取和手势检测功能。

### 构造函数

```cpp
Accelerometer();
```

初始化加速度计实例。

### 方法

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

### 枚举

#### Gesture

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

## Magnetometer 类

提供磁场数据读取功能。

### 构造函数

```cpp
Magnetometer();
```

初始化磁力计实例。

### 方法

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

## LightSensor 类

提供环境光强度读取功能。

### 构造函数

```cpp
LightSensor();
```

初始化光线传感器实例。

### 方法

```cpp
void begin(void);
```

初始化光线传感器硬件。

```cpp
uint16_t getLux(void);
```

获取光照强度。

**返回值**：光照强度值（单位：lux）。
