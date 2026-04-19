#include <MPython.h>

namespace {
// WiFi 账号。
constexpr char kWiFiSsid[] = "YOUR_WIFI_SSID";
constexpr char kWiFiPassword[] = "YOUR_WIFI_PASSWORD";

// 网络音频文件的直链 URL。
// 建议使用可直接访问的音频文件地址，不要使用 m3u8 播放列表。
// 该 URL 也出现在 esp-gmf 的 http 播放示例中，便于对照验证。
constexpr char kAudioUrl[] =
    "https://dl.espressif.com/dl/audio/gs-16b-2c-44100hz.mp3";

// 播放音量，范围通常为 0~100。
constexpr uint8_t kPlayVolume = 80;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== URLPlay Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 设置播放音量。
    mPython.audio.setVolume(kPlayVolume);

    // 连接 WiFi。
    Serial.print("Connecting WiFi...");
    if (!mPython.wifi.connect(kWiFiSsid, kWiFiPassword, 30000)) {
        Serial.println(" failed.");
        return;
    }
    Serial.println(" ok.");
    Serial.print("IP: ");
    Serial.println(mPython.wifi.localIP());

    // 播放网络音频。play() 会立即返回，可在主线程里查看播放状态。
    Serial.println("Playing url...");
    if (!mPython.audio.play(kAudioUrl)) {
        Serial.println("Play start failed.");
        return;
    }

    // 循环输出当前播放状态，直到播放结束。
    while (mPython.audio.state() == Audio::PLAY_STATE_PLAYING ||
           mPython.audio.state() == Audio::PLAY_STATE_PAUSED) {
        Serial.printf("state=%s\n", mPython.audio.stateName());
        delay(500);
    }

    // 输出最终播放状态，便于判断是正常结束还是异常停止。
    Serial.printf("final state=%s\n", mPython.audio.stateName());
    Serial.println("Done.");
}

void loop() {
}
