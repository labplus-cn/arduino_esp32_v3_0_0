#ifndef __MPYTHON_WIFI_H
#define __MPYTHON_WIFI_H

#include "Arduino.h"
#include <WiFi.h>

class MPythonWiFi {
public:
    MPythonWiFi() = default;

    bool connect(const char *ssid, const char *password, uint32_t timeoutMs = 15000);
    bool connect(const char *ssid, uint32_t timeoutMs = 15000);
    bool reconnect(uint32_t timeoutMs = 15000);
    void disconnect(bool wifiOff = false);

    bool setAutoReconnect(bool enable);
    bool autoReconnect() const;

    int16_t scanNetworks(bool async = false, bool showHidden = false);
    void clearScanResults();

    wl_status_t status() const;
    bool isConnected() const;
    String ssid() const;
    String ssid(int32_t index) const;
    String bssid() const;
    int32_t rssi() const;
    int32_t rssi(int32_t index) const;
    String encryptionType(int32_t index) const;

    String localIP() const;
    String gatewayIP() const;
    String subnetMask() const;
    String dnsIP(uint8_t index = 0) const;
    String macAddress() const;
};

#endif
