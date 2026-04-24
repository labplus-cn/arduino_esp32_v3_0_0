# Button API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 Button 类的 API 参考。

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
