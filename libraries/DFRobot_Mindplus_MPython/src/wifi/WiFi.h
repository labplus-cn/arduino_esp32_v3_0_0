#ifndef __MPYTHON_WIFI_H
#define __MPYTHON_WIFI_H

#include "Arduino.h"
#include <WiFi.h>

/**
 * @brief MPythonWiFi 类
 *
 * 提供 WiFi 连接和管理功能
 */
class MPythonWiFi {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 MPythonWiFi 实例
     */
    MPythonWiFi() = default;

    /**
     * @brief 连接到 WiFi 网络（带密码）
     *
     * @param ssid SSID
     * @param password 密码
     * @param timeoutMs 超时时间
     * @return bool 连接结果
     */
    bool connect(const char *ssid, const char *password, uint32_t timeoutMs = 15000);
    
    /**
     * @brief 连接到 WiFi 网络（无密码）
     *
     * @param ssid SSID
     * @param timeoutMs 超时时间
     * @return bool 连接结果
     */
    bool connect(const char *ssid, uint32_t timeoutMs = 15000);
    
    /**
     * @brief 重新连接
     *
     * @param timeoutMs 超时时间
     * @return bool 连接结果
     */
    bool reconnect(uint32_t timeoutMs = 15000);
    
    /**
     * @brief 断开连接
     *
     * @param wifiOff 是否关闭 WiFi
     */
    void disconnect(bool wifiOff = false);

    /**
     * @brief 设置自动重连
     *
     * @param enable 是否启用
     * @return bool 设置结果
     */
    bool setAutoReconnect(bool enable);
    
    /**
     * @brief 获取自动重连状态
     *
     * @return bool 自动重连状态
     */
    bool autoReconnect() const;

    /**
     * @brief 扫描网络
     *
     * @param async 是否异步
     * @param showHidden 是否显示隐藏网络
     * @return int16_t 扫描到的网络数量
     */
    int16_t scanNetworks(bool async = false, bool showHidden = false);
    
    /**
     * @brief 清除扫描结果
     */
    void clearScanResults();

    /**
     * @brief 获取连接状态
     *
     * @return wl_status_t 连接状态
     */
    wl_status_t status() const;
    
    /**
     * @brief 检查是否已连接
     *
     * @return bool 连接状态
     */
    bool isConnected() const;
    
    /**
     * @brief 获取当前连接的 SSID
     *
     * @return String SSID
     */
    String ssid() const;
    
    /**
     * @brief 获取指定索引的 SSID
     *
     * @param index 索引
     * @return String SSID
     */
    String ssid(int32_t index) const;
    
    /**
     * @brief 获取当前连接的 BSSID
     *
     * @return String BSSID
     */
    String bssid() const;
    
    /**
     * @brief 获取当前连接的 RSSI
     *
     * @return int32_t RSSI
     */
    int32_t rssi() const;
    
    /**
     * @brief 获取指定索引的 RSSI
     *
     * @param index 索引
     * @return int32_t RSSI
     */
    int32_t rssi(int32_t index) const;
    
    /**
     * @brief 获取指定索引的加密类型
     *
     * @param index 索引
     * @return String 加密类型
     */
    String encryptionType(int32_t index) const;

    /**
     * @brief 获取本地 IP
     *
     * @return String 本地 IP
     */
    String localIP() const;
    
    /**
     * @brief 获取网关 IP
     *
     * @return String 网关 IP
     */
    String gatewayIP() const;
    
    /**
     * @brief 获取子网掩码
     *
     * @return String 子网掩码
     */
    String subnetMask() const;
    
    /**
     * @brief 获取 DNS IP
     *
     * @param index 索引
     * @return String DNS IP
     */
    String dnsIP(uint8_t index = 0) const;
    
    /**
     * @brief 获取 MAC 地址
     *
     * @return String MAC 地址
     */
    String macAddress() const;
};

#endif
