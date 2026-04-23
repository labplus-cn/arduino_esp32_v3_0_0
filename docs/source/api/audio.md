# Audio API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 Audio 类的 API 参考。

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
