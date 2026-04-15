#include <MPython.h>

namespace {
// 录音文件保存到 LittleFS 中的固定路径。
constexpr char kRecordFile[] = "/record.wav";

// 录音时长，单位为毫秒。3000 表示录音 3 秒。
constexpr uint32_t kRecordDurationMs = 3000;

// 录音采样率。16000Hz 适合语音场景，占用空间也比较小。
constexpr uint32_t kSampleRate = 16000;

// 录音位宽。16bit 是常见的 PCM 音频格式。
constexpr uint8_t kBitsPerSample = 16;

// 录音声道数。1 表示单声道。
constexpr uint8_t kChannels = 1;

// 播放音量，范围通常为 0~100。
constexpr uint8_t kPlayVolume = 80;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== RecordPlay Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 设置播放音量。
    mPython.audio.setVolume(kPlayVolume);

    // 开始录音，并自动持续指定时长。
    Serial.println("Recording...");
    if (!mPython.audio.startRecord(kRecordFile,
                                   kSampleRate,
                                   kBitsPerSample,
                                   kChannels,
                                   kRecordDurationMs)) {
        Serial.println("Record failed.");
        return;
    }

    // 给播放前留一点间隔，避免录音刚结束就立刻播放。
    delay(1000);

    // 播放刚才录好的音频文件。
    Serial.println("Playing...");
    if (!mPython.audio.playFile(kRecordFile)) {
        Serial.println("Play failed.");
        return;
    }

    Serial.println("Done.");
}

void loop() {
}
