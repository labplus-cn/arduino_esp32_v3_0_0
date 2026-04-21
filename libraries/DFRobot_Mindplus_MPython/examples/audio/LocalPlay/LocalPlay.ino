#include <MPython.h>

namespace {
// 本地音频文件保存在 LittleFS 的固定路径。
// 请先确认这个文件已经写入 LittleFS 的 lfs2 分区中。
constexpr char kAudioFile[] = "test.mp3";

// 播放音量，范围通常为 0~100。
constexpr uint8_t kPlayVolume = 60;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== LocalPlay Example ===");

    // 初始化MPython（含音频模块）。
    mPython.begin();

    // 设置播放音量。
    mPython.audio.setVolume(kPlayVolume);

    // 播放 LittleFS 中已有的本地音频文件。play() 会立即返回，可在主线程里查看播放状态。
    Serial.println("Playing local file...");
    if (!mPython.audio.play(kAudioFile)) {
        Serial.println("Play failed. Check whether that file exists and format is supported.");
        return;
    }

    Serial.println("Done.");
}

void loop() {
}
