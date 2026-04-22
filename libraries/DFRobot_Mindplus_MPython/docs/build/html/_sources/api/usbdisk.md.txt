# USBDISK API 参考

本文档提供了 DFRobot_Mindplus_MPython 库中 USBDISK 类的 API 参考。

## USBDISK 类

将 flash 分区通过 USB MSC 暴露为 U 盘。

### 构造函数

```cpp
USBDISK();
```

初始化 USBDISK 实例。

### 方法

```cpp
bool begin(const char *partitionLabel = "spiffs", const char *vendorID = "mPython", const char *productID = "Flash Disk", const char *revision = "1.0");
```

初始化并启动 USB MSC。

**参数**：
- partitionLabel：分区名（默认 "spiffs"）
- vendorID：厂商 ID
- productID：产品 ID
- revision：版本号

**返回值**：初始化结果。

```cpp
void end();
```

停止 USB 存储功能。

```cpp
bool mounted() const;
```

检查是否已挂载。

**返回值**：挂载状态。
