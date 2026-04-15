#include <MPython.h>

namespace {
// 录音文件保存到 LittleFS 中的固定路径。
constexpr char kRecordFile[] = "/record.wav";

// 手动录音总时长，单位为毫秒。
constexpr uint32_t kRecordDurationMs = 3000;

// 每次从音频输入读取的数据块大小。
constexpr size_t kChunkBytes = 2048;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== ManualRecordChunk Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 先启动录音，再由用户自己循环调用 recordChunk()。
    if (!mPython.audio.startRecord(kRecordFile, 16000, 16, 1)) {
        Serial.println("Record start failed.");
        return;
    }

    uint32_t startMs = millis();
    size_t totalBytes = 0;

    // 在指定时间内反复拉取录音数据，并写入文件。
    while (millis() - startMs < kRecordDurationMs) {
        totalBytes += mPython.audio.recordChunk(kChunkBytes);
        delay(10);
    }

    mPython.audio.stopRecord();

    Serial.printf("Recorded %u bytes.\n", (unsigned)totalBytes);

    delay(1000);

    if (!mPython.audio.playFile(kRecordFile)) {
        Serial.println("Play failed.");
        return;
    }

    Serial.println("Done.");
}

void loop() {
}
