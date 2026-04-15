#include "wifi/WiFi.h"

namespace {
String encryptionTypeToString(wifi_auth_mode_t type) {
    switch (type) {
        case WIFI_AUTH_OPEN:
            return "OPEN";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2_WPA3_PSK";
        case WIFI_AUTH_WAPI_PSK:
            return "WAPI_PSK";
        default:
            return "UNKNOWN";
    }
}
}

bool MPythonWiFi::connect(const char *ssid, const char *password, uint32_t timeoutMs) {
    if (!ssid || !ssid[0]) {
        return false;
    }

    ::WiFi.mode(WIFI_STA);
    ::WiFi.begin(ssid, password);

    uint32_t startMs = millis();
    while (::WiFi.status() != WL_CONNECTED && millis() - startMs < timeoutMs) {
        delay(100);
    }
    return ::WiFi.status() == WL_CONNECTED;
}

bool MPythonWiFi::connect(const char *ssid, uint32_t timeoutMs) {
    return connect(ssid, nullptr, timeoutMs);
}

bool MPythonWiFi::reconnect(uint32_t timeoutMs) {
    ::WiFi.reconnect();

    uint32_t startMs = millis();
    while (::WiFi.status() != WL_CONNECTED && millis() - startMs < timeoutMs) {
        delay(100);
    }
    return ::WiFi.status() == WL_CONNECTED;
}

void MPythonWiFi::disconnect(bool wifiOff) {
    ::WiFi.disconnect();
    if (wifiOff) {
        ::WiFi.mode(WIFI_OFF);
    }
}

bool MPythonWiFi::setAutoReconnect(bool enable) {
    return ::WiFi.setAutoReconnect(enable);
}

bool MPythonWiFi::autoReconnect() const {
    return ::WiFi.getAutoReconnect();
}

int16_t MPythonWiFi::scanNetworks(bool async, bool showHidden) {
    ::WiFi.mode(WIFI_STA);
    return ::WiFi.scanNetworks(async, showHidden);
}

void MPythonWiFi::clearScanResults() {
    ::WiFi.scanDelete();
}

wl_status_t MPythonWiFi::status() const {
    return ::WiFi.status();
}

bool MPythonWiFi::isConnected() const {
    return ::WiFi.status() == WL_CONNECTED;
}

String MPythonWiFi::ssid() const {
    return ::WiFi.SSID();
}

String MPythonWiFi::ssid(int32_t index) const {
    return ::WiFi.SSID(index);
}

String MPythonWiFi::bssid() const {
    return ::WiFi.BSSIDstr();
}

int32_t MPythonWiFi::rssi() const {
    return ::WiFi.RSSI();
}

int32_t MPythonWiFi::rssi(int32_t index) const {
    return ::WiFi.RSSI(index);
}

String MPythonWiFi::encryptionType(int32_t index) const {
    return encryptionTypeToString(::WiFi.encryptionType(index));
}

String MPythonWiFi::localIP() const {
    return ::WiFi.localIP().toString();
}

String MPythonWiFi::gatewayIP() const {
    return ::WiFi.gatewayIP().toString();
}

String MPythonWiFi::subnetMask() const {
    return ::WiFi.subnetMask().toString();
}

String MPythonWiFi::dnsIP(uint8_t index) const {
    return ::WiFi.dnsIP(index).toString();
}

String MPythonWiFi::macAddress() const {
    return ::WiFi.macAddress();
}
