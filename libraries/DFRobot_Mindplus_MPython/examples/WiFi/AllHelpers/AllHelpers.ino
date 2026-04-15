#include <MPython.h>

namespace {
constexpr char kSsid[] = "YOUR_WIFI_SSID";
constexpr char kPassword[] = "YOUR_WIFI_PASSWORD";
unsigned long lastReportMs = 0;
}

void printNetworkInfo() {
    Serial.print("SSID: ");
    Serial.println(mPython.wifi.ssid());
    Serial.print("IP: ");
    Serial.println(mPython.wifi.localIP());
    Serial.print("Gateway: ");
    Serial.println(mPython.wifi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(mPython.wifi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(mPython.wifi.dnsIP(0));
    Serial.print("MAC: ");
    Serial.println(mPython.wifi.macAddress());
    Serial.print("RSSI: ");
    Serial.println(mPython.wifi.rssi());
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== WiFi Full Helper Example ===");

    mPython.wifi.setAutoReconnect(true);

    Serial.println("Scanning nearby networks...");
    int16_t count = mPython.wifi.scanNetworks();
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

    if (mPython.wifi.connect(kSsid, kPassword)) {
        Serial.println("Connected.");
        printNetworkInfo();
    } else {
        Serial.println("Connect failed.");
    }
}

void loop() {
    if (millis() - lastReportMs < 5000) {
        return;
    }
    lastReportMs = millis();

    if (!mPython.wifi.isConnected()) {
        Serial.println("Lost WiFi, reconnecting...");
        if (mPython.wifi.reconnect()) {
            Serial.println("Reconnect success.");
            printNetworkInfo();
        } else {
            Serial.println("Reconnect failed.");
        }
        return;
    }

    Serial.print("WiFi alive, RSSI=");
    Serial.println(mPython.wifi.rssi());
}
