#include <MPython.h>

namespace {
constexpr char kSsid[] = "YOUR_WIFI_SSID";
constexpr char kPassword[] = "YOUR_WIFI_PASSWORD";
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== WiFi Network Info Example ===");

    if (!mPython.wifi.connect(kSsid, kPassword)) {
        Serial.println("WiFi connect failed.");
        return;
    }

    Serial.print("SSID: ");
    Serial.println(mPython.wifi.ssid());
    Serial.print("BSSID: ");
    Serial.println(mPython.wifi.bssid());
    Serial.print("Local IP: ");
    Serial.println(mPython.wifi.localIP());
    Serial.print("Gateway: ");
    Serial.println(mPython.wifi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(mPython.wifi.subnetMask());
    Serial.print("DNS0: ");
    Serial.println(mPython.wifi.dnsIP(0));
    Serial.print("DNS1: ");
    Serial.println(mPython.wifi.dnsIP(1));
    Serial.print("MAC: ");
    Serial.println(mPython.wifi.macAddress());
    Serial.print("RSSI: ");
    Serial.println(mPython.wifi.rssi());
}

void loop() {
}
