#ifndef __LV_FONT_READER_H
#define __LV_FONT_READER_H

#include <Arduino.h>
#include <esp_partition.h>
#include <vector>
#include <map>

// LVGL bin字体文件头结构（与lv_font_conv输出格式对应）
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;           // 魔数 0x544E4C46 ("LVGL")
    uint16_t version;         // 版本号
    uint16_t bpp;            // 每像素位数 (1,2,4,8)
    uint8_t compressed;      // 是否压缩
    uint8_t reserved[3];     // 保留
    uint32_t glyph_num;      // 字形数量
    uint32_t kern_class_num; // 字距类别数
    uint32_t kern_pairs_num; // 字距对数
    uint32_t cmap_table_pos; // cmap表位置（字节偏移）
    uint32_t glyph_pos;      // 字形数据起始位置
    uint32_t kern_table_pos; // 字距表位置
    int32_t line_height;     // 行高
    int32_t base_line;       // 基线
    uint32_t glyph_bitmap_pos; // 位图数据起始位置
    uint32_t glyph_dsc_pos;    // 字形描述符位置
} font_bin_header_t;

// 字形描述符
typedef struct {
    uint32_t bitmap_index;   // 位图数据索引（相对于glyph_bitmap_pos的偏移）
    uint32_t adv_w;          // 推进宽度（下一个字符的X偏移）
    uint8_t box_w;           // 边界框宽度（实际图形宽度）
    uint8_t box_h;           // 边界框高度（实际图形高度）
    int8_t ofs_x;            // X轴偏移（相对于光标位置）
    int8_t ofs_y;            // Y轴偏移（相对于基线）
    uint8_t bpp;             // 该字形的实际bpp
} glyph_descriptor_t;

// 字符映射条目
typedef struct {
    uint32_t unicode;        // Unicode码点
    uint32_t glyph_index;    // 字形索引
} cmap_entry_t;

// 渲染结果
typedef struct {
    uint8_t* bitmap;         // 位图数据指针
    uint16_t width;          // 实际宽度
    uint16_t height;         // 实际高度
    int16_t offset_x;        // X偏移
    int16_t offset_y;        // Y偏移
    uint16_t advance_x;      // X推进距离
    size_t bitmap_size;      // 位图数据大小（字节）
} glyph_render_t;
#pragma pack(pop)

class FontReader {
private:
    const esp_partition_t* fontPartition;  // 字体分区
    font_bin_header_t header;              // 字体头
    std::vector<cmap_entry_t> cmap;        // 字符映射表
    std::vector<glyph_descriptor_t> glyphs; // 字形描述符数组
    
    uint8_t* bitmapBuffer;                 // 位图数据缓冲区
    size_t bitmapBufferSize;               // 缓冲区大小
    bool initialized;
    
    // 从分区读取数据
    bool readPartition(uint32_t offset, void* buffer, size_t size);
    
    // 解析字体头
    bool parseHeader();
    
    // 解析字符映射表
    bool parseCMap();
    
    // 解析所有字形描述符
    bool parseGlyphDescriptors();
    
    // 根据Unicode查找字形索引
    int32_t findGlyphIndex(uint32_t unicode);
    
    // 读取字形位图数据
    bool readGlyphBitmap(uint32_t bitmapOffset, uint8_t* buffer, size_t size);
    
public:
    FontReader();
    ~FontReader();
    
    // 初始化字体读取器
    bool init();
    
    // 获取字体信息
    void printInfo();
    
    // 获取字符的字形数据
    bool getGlyph(uint32_t unicode, glyph_render_t* output);
    
    // 获取字符的宽度（用于文本布局）
    uint16_t getCharWidth(uint32_t unicode);
    
    // 获取字符串的渲染宽度
    uint32_t getStringWidth(const char* text);
    
    // 获取字符串的渲染宽度（UTF-8版本）
    uint32_t getUTF8StringWidth(const char* text);
    
    // 渲染文本到位图（简单版本）
    uint8_t* renderText(const char* text, uint32_t* out_width, uint32_t* out_height);
    
    // 释放渲染结果
    void freeGlyph(glyph_render_t* glyph);
    
    // 检查是否包含某个字符
    bool hasGlyph(uint32_t unicode);
    
    // 获取字体行高
    int32_t getLineHeight() { return header.line_height; }
    
    // 获取字体基线
    int32_t getBaseLine() { return header.base_line; }
    
    // 获取字形数量
    uint32_t getGlyphCount() { return header.glyph_num; }
};

#endif