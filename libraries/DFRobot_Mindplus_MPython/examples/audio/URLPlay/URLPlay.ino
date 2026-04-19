#include <MPython.h>

constexpr char kWiFiSsid[] = "YOUR_WIFI_SSID";
constexpr char kWiFiPassword[] = "YOUR_WIFI_PASSWORD";

constexpr char kAudioUrl[] =
    // "https://dl.espressif.com/dl/audio/gs-16b-2c-44100hz.mp3";
    "http://cdn.makeymonkey.com/test/32_%E6%8B%94%E8%90%9D%E5%8D%9C.mp3";

uint8_t kPlayVolume = 60;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== URLPlay Example ===");

    // 初始化MPython（含音频模块）。
    mPython.begin();

    // 设置播放音量。

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

    Serial.println("Done.");
}

void loop() {
}
