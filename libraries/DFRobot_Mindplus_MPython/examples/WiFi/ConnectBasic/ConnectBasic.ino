#include <MPython.h>

namespace {
constexpr char kSsid[] = "YOUR_WIFI_SSID";
constexpr char kPassword[] = "YOUR_WIFI_PASSWORD";
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== WiFi Connect Example ===");

    if (mPython.wifi.connect(kSsid, kPassword)) {
        Serial.println("WiFi connected.");
        Serial.print("IP: ");
        Serial.println(mPython.wifi.localIP());
    } else {
        Serial.println("WiFi connect failed.");
    }
}

void loop() {
}
