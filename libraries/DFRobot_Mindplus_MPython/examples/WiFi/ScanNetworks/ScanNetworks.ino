#include <MPython.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== WiFi Scan Example ===");
    mPython.begin();

    int16_t count = mPython.wifi.scanNetworks();
    if (count < 0) {
        Serial.println("Scan failed.");
        return;
    }

    Serial.print("Networks found: ");
    Serial.println(count);
    for (int16_t i = 0; i < count; ++i) {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(mPython.wifi.ssid(i));
        Serial.print(" | RSSI=");
        Serial.print(mPython.wifi.rssi(i));
        Serial.print(" | ENC=");
        Serial.println(mPython.wifi.encryptionType(i));
    }
    mPython.wifi.clearScanResults();
}

void loop() {
}
