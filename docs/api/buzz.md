# Buzz API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 Buzz 类的 API 参考。

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
