#include <MPython.h>

namespace {
// 录音文件保存到 LittleFS 的固定路径。
// 现在可以不写前导 /，库内部会自动补齐。
constexpr char kRecordFile[] = "record.wav";

// 录音时长，单位为毫秒。3000 表示录音 3 秒。
constexpr uint32_t kRecordDurationMs = 3000;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== RecordBasic Example ===");

    // 初始化MPython（含音频模块）。
    mPython.begin();

    // 使用默认参数启动内部自动录音任务，持续 3 秒，然后播放录下来的文件。
    if (!mPython.audio.startRecord(kRecordFile, 16000, 16, 1, kRecordDurationMs)) {
        Serial.println("Record failed.");
        return;
    }

    delay(1000);

    // 播放刚才录好的音频文件。play() 会立即返回，可在主线程里查看播放状态。
    if (!mPython.audio.play(kRecordFile)) {
        Serial.println("Play start failed.");
        return;
    }

    // 循环输出当前播放状态，直到播放结束。
    while (mPython.audio.playing()) {
        Serial.printf("state=%s\n", mPython.audio.stateName());
        delay(300);
    }

    // 输出最终播放状态，便于判断是正常结束还是异常停止。
    Serial.printf("final state=%s\n", mPython.audio.stateName());
    Serial.println("Done.");
}

void loop() {
}
