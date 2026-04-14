#include <MPython.h>

// 将 flash 中的 "spiffs" 分区通过 USB 暴露为 U 盘
// 注意：USB MSC 与 LittleFS 不能同时挂载同一分区，
//       使用 U 盘模式时请勿同时调用 LittleFS.begin()

void setup() {
    Serial.begin(115200);
    mPython.begin();

    // 启动 U 盘，挂载 "spiffs" 分区
    if (mPython.usbDisk.begin("vfs")) {
        Serial.println("U盘已启动，请连接 USB 线");
    } else {
        Serial.println("U盘启动失败，请检查分区表");
    }
}

void loop() {
}
