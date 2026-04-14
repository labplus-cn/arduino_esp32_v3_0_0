#ifndef USBDISK_H
#define USBDISK_H

#include "Arduino.h"
#include "USB.h"
#include "USBMSC.h"
#include "esp_partition.h"

// 将 flash 分区通过 USB MSC 暴露为 U 盘
class USBDISK {
public:
    USBDISK();

    // 初始化并启动 USB MSC，partitionLabel 为分区名（默认 "spiffs"）
    bool begin(const char *partitionLabel = "spiffs",
               const char *vendorID       = "mPython",
               const char *productID      = "Flash Disk",
               const char *revision       = "1.0");

    // 停止 USB MSC
    void end();

    // 是否已挂载
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
