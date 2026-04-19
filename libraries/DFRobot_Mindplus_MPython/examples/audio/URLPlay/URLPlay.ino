#include <MPython.h>

namespace {
constexpr char kWiFiSsid[] = "YOUR_WIFI_SSID";
constexpr char kWiFiPassword[] = "YOUR_WIFI_PASSWORD";

constexpr char kAudioUrl[] =
    "https://dl.espressif.com/dl/audio/gs-16b-2c-44100hz.mp3";
    // "http://cdn.makeymonkey.com/test/32_%E6%8B%94%E8%90%9D%E5%8D%9C.mp3";

// 播放音量，范围通常为 0~100。
constexpr uint8_t kPlayVolume = 50;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== URLPlay Example ===");

    // 初始化掌控板。
    mPython.begin();

    // 设置播放音量。
    mPython.audio.setVolume(kPlayVolume);

    // 连接 WiFi。
    Serial.print("Connecting WiFi...");
    if (!mPython.wifi.connect(kWiFiSsid, kWiFiPassword, 30000)) {
        Serial.println(" failed.");
        return;
    }
    Serial.print("IP: ");
    Serial.println(mPython.wifi.localIP());

    // 播放网络音频。play() 会立即返回，可在主线程里查看播放状态。
    Serial.println("Playing url...");
    if (!mPython.audio.play(kAudioUrl)) {
        Serial.println("Play start failed.");
        return;
    }


    // 输出最终播放状态，便于判断是正常结束还是异常停止。
    Serial.printf("final state=%s\n", mPython.audio.stateName());
    Serial.println("Done.");
}

void loop() {
}
