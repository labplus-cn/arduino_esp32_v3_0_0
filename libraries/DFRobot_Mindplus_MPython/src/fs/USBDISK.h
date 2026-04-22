#ifndef USBDISK_H
#define USBDISK_H

#include "Arduino.h"
#include "USB.h"
#include "USBMSC.h"
#include "esp_partition.h"

/**
 * @brief USBDISK 类
 *
 * 将 flash 分区通过 USB MSC 暴露为 U 盘
 */
class USBDISK {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 USBDISK 实例
     */
    USBDISK();

    /**
     * @brief 初始化并启动 USB MSC
     *
     * @param partitionLabel 分区名（默认 "spiffs"）
     * @param vendorID 厂商 ID
     * @param productID 产品 ID
     * @param revision 版本号
     * @return bool 初始化结果
     */
    bool begin(const char *partitionLabel = "spiffs",
               const char *vendorID       = "mPython",
               const char *productID      = "Flash Disk",
               const char *revision       = "1.0");

    /**
     * @brief 停止 USB MSC
     *
     * 停止 USB 存储功能
     */
    void end();

    /**
     * @brief 检查是否已挂载
     *
     * @return bool 挂载状态
     */
    bool mounted() const { return _mounted; }

private:
    USBMSC _msc;
    const esp_partition_t *_part;
    bool _mounted;

    static int32_t _onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    static int32_t _onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
    static bool    _onStartStop(uint8_t power_condition, bool start, bool load_eject);

    // 单例指针，供静态回调访问
    static USBDISK *_instance;
};

#endif
