#include <MPython.h>

namespace {
constexpr char kSsid[] = "YOUR_WIFI_SSID";
constexpr char kPassword[] = "YOUR_WIFI_PASSWORD";
unsigned long lastCheckMs = 0;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== WiFi Auto Reconnect Example ===");

    mPython.wifi.setAutoReconnect(true);
    if (mPython.wifi.connect(kSsid, kPassword)) {
        Serial.println("Initial connect success.");
        Serial.println(mPython.wifi.localIP());
    } else {
        Serial.println("Initial connect failed.");
    }
}

void loop() {
    if (millis() - lastCheckMs < 2000) {
        return;
    }
    lastCheckMs = millis();

    Serial.print("Connected: ");
    Serial.println(mPython.wifi.isConnected() ? "yes" : "no");

    if (!mPython.wifi.isConnected()) {
        Serial.println("Trying reconnect...");
        if (mPython.wifi.reconnect()) {
            Serial.print("Reconnected, IP: ");
            Serial.println(mPython.wifi.localIP());
        } else {
            Serial.println("Reconnect failed.");
        }
    }
}
