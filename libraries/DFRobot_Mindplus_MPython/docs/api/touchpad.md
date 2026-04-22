# TouchPad API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 TouchPad 类的 API 参考。

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
