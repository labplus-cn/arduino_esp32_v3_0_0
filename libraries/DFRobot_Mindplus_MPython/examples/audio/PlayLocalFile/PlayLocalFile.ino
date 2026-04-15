#include <MPython.h>

namespace {
// 要播放的本地音频文件路径。
// 请先确认这个文件已经保存在 LittleFS 的 lfs2 分区中。
// constexpr char kAudioFile[] = "/record.wav";
constexpr char kAudioFile[] = "/test.mp3";

// 播放音量，范围通常为 0~100。
constexpr uint8_t kPlayVolume = 60;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== PlayLocalFile Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 设置播放音量。
    mPython.audio.setVolume(kPlayVolume);

    // 播放 LittleFS 中已有的本地音频文件。
    // 支持的格式取决于库内已启用的解码器，WAV 可直接播放。
    Serial.println("Playing local file...");
    if (!mPython.audio.playFile(kAudioFile)) {
        Serial.println("Play failed. Check whether the file exists and format is supported.");
        return;
    }

    Serial.println("Done.");
}

void loop() {
}
