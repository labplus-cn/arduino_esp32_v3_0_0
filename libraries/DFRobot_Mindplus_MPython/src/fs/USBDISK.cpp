#include "USBDISK.h"

USBDISK *USBDISK::_instance = nullptr;

USBDISK::USBDISK() : _part(nullptr), _mounted(false) {}

bool USBDISK::begin(const char *partitionLabel, const char *vendorID,
                    const char *productID, const char *revision) {
#if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 1)
    ::printf("[USBDISK] USB MSC requires USB-OTG (TinyUSB) mode, current ARDUINO_USB_MODE=1 (Hardware CDC/JTAG)\n");
    return false;
#endif

    // 查找 flash 分区
    _part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                     ESP_PARTITION_SUBTYPE_ANY, partitionLabel);
    if (_part == nullptr) {
        ::printf("[USBDISK] partition '%s' not found\n", partitionLabel);
        return false;
    }

    uint32_t blockSize  = 512;
    uint32_t blockCount = _part->size / blockSize;

    _instance = this;

    _msc.vendorID(vendorID);
    _msc.productID(productID);
    _msc.productRevision(revision);
    _msc.onRead(_onRead);
    _msc.onWrite(_onWrite);
    _msc.onStartStop(_onStartStop);
    _msc.mediaPresent(true);
    _msc.isWritable(true);
    _msc.begin(blockCount, blockSize);

    USB.begin();
    _mounted = true;
    ::printf("[USBDISK] started: partition='%s' size=%lu blocks=%lu\n",
             partitionLabel, (unsigned long)_part->size, (unsigned long)blockCount);
    return true;
}

void USBDISK::end() {
    _msc.end();
    _mounted = false;
}

int32_t USBDISK::_onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
    if (!_instance || !_instance->_part) return -1;
    uint32_t addr = lba * 512 + offset;
    esp_err_t err = esp_partition_read(_instance->_part, addr, buffer, bufsize);
    return (err == ESP_OK) ? (int32_t)bufsize : -1;
}

int32_t USBDISK::_onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
    if (!_instance || !_instance->_part) return -1;
    uint32_t addr = lba * 512 + offset;
    // flash 写入前需按扇区擦除（4096 字节对齐）
    uint32_t eraseAddr  = addr & ~(4095U);
    uint32_t eraseSize  = ((addr + bufsize + 4095U) & ~(4095U)) - eraseAddr;
    esp_err_t err = esp_partition_erase_range(_instance->_part, eraseAddr, eraseSize);
    if (err != ESP_OK) return -1;
    err = esp_partition_write(_instance->_part, addr, buffer, bufsize);
    return (err == ESP_OK) ? (int32_t)bufsize : -1;
}

bool USBDISK::_onStartStop(uint8_t power_condition, bool start, bool load_eject) {
    ::printf("[USBDISK] start_stop: power=%d start=%d eject=%d\n",
             power_condition, start, load_eject);
    return true;
}
