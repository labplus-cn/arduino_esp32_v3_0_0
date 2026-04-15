#include <MPython.h>

namespace {
// 录音文件保存到 LittleFS 中的固定路径。
constexpr char kRecordFile[] = "/record.wav";

// 录音时长，单位为毫秒。3000 表示录音 3 秒。
constexpr uint32_t kRecordDurationMs = 3000;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== RecordPlay Minimal Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 使用默认参数录音 3 秒，然后播放录下来的文件。
    if (!mPython.audio.startRecord(kRecordFile, 16000, 16, 1, kRecordDurationMs)) {
        Serial.println("Record failed.");
        return;
    }

    delay(1000);

    if (!mPython.audio.playFile(kRecordFile)) {
        Serial.println("Play failed.");
        return;
    }

    Serial.println("Done.");
}

void loop() {
}
