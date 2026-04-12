#include "LVFontReader.h"

#define LV_FONT_MAGIC_NUMBER 0x544E4C46  // "LVGL"

FontReader::FontReader() 
    : fontPartition(nullptr)
    , bitmapBuffer(nullptr)
    , bitmapBufferSize(0)
    , initialized(false) {
    memset(&header, 0, sizeof(header));
}

FontReader::~FontReader() {
    if (bitmapBuffer) {
        free(bitmapBuffer);
    }
}

bool FontReader::readPartition(uint32_t offset, void* buffer, size_t size) {
    if (fontPartition == nullptr) return false;
    
    if (offset + size > fontPartition->size) {
        Serial.printf("[FontReader] 读取超出分区范围: offset=0x%lX, size=%u, max=%lu\n", 
                      offset, size, fontPartition->size);
        return false;
    }
    
    esp_err_t err = esp_partition_read(fontPartition, offset, buffer, size);
    if (err != ESP_OK) {
        Serial.printf("[FontReader] 分区读取失败: 0x%X\n", err);
        return false;
    }
    
    return true;
}

bool FontReader::parseHeader() {
    // 读取文件头
    if (!readPartition(0, &header, sizeof(header))) {
        Serial.println("[FontReader] 读取字体头失败");
        return false;
    }
    
    // 验证魔数
    if (header.magic != LV_FONT_MAGIC_NUMBER) {
        Serial.printf("[FontReader] 无效的字体文件: magic=0x%lX, 期望=0x%X\n", 
                      header.magic, LV_FONT_MAGIC_NUMBER);
        return false;
    }
    
    // 验证版本
    if (header.version != 1) {
        Serial.printf("[FontReader] 不支持的字体版本: %d\n", header.version);
        return false;
    }
    
    Serial.println("[FontReader] 字体头解析成功");
    Serial.printf("  字形数量: %ld\n", header.glyph_num);
    Serial.printf("  行高: %ld\n", header.line_height);
    Serial.printf("  基线: %ld\n", header.base_line);
    Serial.printf("  BPP: %d\n", header.bpp);
    
    return true;
}

bool FontReader::parseCMap() {
    cmap.clear();
    
    // 读取所有字符映射
    for (uint32_t i = 0; i < header.glyph_num; i++) {
        cmap_entry_t entry;
        uint32_t offset = header.cmap_table_pos + i * 8;  // 每个条目8字节
        
        if (!readPartition(offset, &entry.unicode, 4)) {
            Serial.printf("[FontReader] 读取cmap条目 %ld 失败\n", i);
            return false;
        }
        
        if (!readPartition(offset + 4, &entry.glyph_index, 4)) {
            Serial.printf("[FontReader] 读取glyph索引 %ld 失败\n", i);
            return false;
        }
        
        cmap.push_back(entry);
    }
    
    Serial.printf("[FontReader] 成功加载 %d 个字符映射\n", cmap.size());
    return true;
}

bool FontReader::parseGlyphDescriptors() {
    glyphs.clear();
    glyphs.resize(header.glyph_num);
    
    // 读取所有字形描述符
    for (uint32_t i = 0; i < header.glyph_num; i++) {
        uint32_t offset = header.glyph_dsc_pos + i * sizeof(glyph_descriptor_t);
        
        if (!readPartition(offset, &glyphs[i], sizeof(glyph_descriptor_t))) {
            Serial.printf("[FontReader] 读取字形描述符 %ld 失败\n", i);
            return false;
        }
    }
    
    Serial.printf("[FontReader] 成功加载 %d 个字型描述符\n", glyphs.size());
    return true;
}

int32_t FontReader::findGlyphIndex(uint32_t unicode) {
    // 二分查找（假设cmap按unicode排序）
    int left = 0, right = cmap.size() - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (cmap[mid].unicode == unicode) {
            return cmap[mid].glyph_index;
        } else if (cmap[mid].unicode < unicode) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

bool FontReader::readGlyphBitmap(uint32_t bitmapOffset, uint8_t* buffer, size_t size) {
    uint32_t absOffset = header.glyph_bitmap_pos + bitmapOffset;
    return readPartition(absOffset, buffer, size);
}

bool FontReader::init() {
    if (initialized) return true;
    
    // 查找字体分区
    fontPartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, (esp_partition_subtype_t)0x40, "font");
    if (fontPartition == nullptr) {
        // 尝试通过标签查找
        fontPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, 
                                                  ESP_PARTITION_SUBTYPE_ANY, 
                                                  "font_data");
    }
    
    if (fontPartition == nullptr) {
        Serial.println("[FontReader] 未找到字体分区");
        return false;
    }
    
    Serial.printf("[FontReader] 找到字体分区: 地址=0x%08lX, 大小=%lu bytes\n", 
                 fontPartition->address, fontPartition->size);
    
    // 解析字体数据
    if (!parseHeader()) return false;
    if (!parseCMap()) return false;
    if (!parseGlyphDescriptors()) return false;
    
    // 分配位图缓冲区（默认4KB）
    bitmapBufferSize = 4096;
    bitmapBuffer = (uint8_t*)malloc(bitmapBufferSize);
    if (bitmapBuffer == nullptr) {
        Serial.println("[FontReader] 分配位图缓冲区失败");
        return false;
    }
    
    initialized = true;
    Serial.println("[FontReader] 初始化成功");
    return true;
}

void FontReader::printInfo() {
    if (!initialized) {
        Serial.println("[FontReader] 字体未初始化");
        return;
    }
    
    Serial.println("========== 字体信息 ==========");
    Serial.printf("魔数: 0x%08lX\n", header.magic);
    Serial.printf("版本: %d\n", header.version);
    Serial.printf("字形数量: %ld\n", header.glyph_num);
    Serial.printf("行高: %ld\n", header.line_height);
    Serial.printf("基线: %ld\n", header.base_line);
    Serial.printf("每像素位数: %d\n", header.bpp);
    Serial.printf("是否压缩: %s\n", header.compressed ? "是" : "否");
    Serial.printf("cmap表位置: 0x%lX\n", header.cmap_table_pos);
    Serial.printf("字形数据位置: 0x%lX\n", header.glyph_pos);
    Serial.printf("位图数据位置: 0x%lX\n", header.glyph_bitmap_pos);
    Serial.printf("字形描述符位置: 0x%lX\n", header.glyph_dsc_pos);
    Serial.println("================================\n");
    
    // 显示前20个字符
    Serial.println("字符映射表示例（前20个）:");
    for (int i = 0; i < min(20, (int)cmap.size()); i++) {
        if (cmap[i].unicode < 0x80) {
            // ASCII字符
            Serial.printf("  0x%04lX ('%c') -> 索引: %ld\n", 
                         cmap[i].unicode, 
                         (char)cmap[i].unicode,
                         cmap[i].glyph_index);
        } else {
            // 非ASCII字符
            Serial.printf("  0x%04lX -> 索引: %ld\n", 
                         cmap[i].unicode, 
                         cmap[i].glyph_index);
        }
    }
    Serial.println("================================\n");
}

bool FontReader::getGlyph(uint32_t unicode, glyph_render_t* output) {
    if (!initialized || output == nullptr) return false;
    
    // 查找字形索引
    int32_t glyphIndex = findGlyphIndex(unicode);
    if (glyphIndex < 0) {
        return false;
    }
    
    // 获取字形描述符
    glyph_descriptor_t& desc = glyphs[glyphIndex];
    
    // 计算位图大小（字节）
    size_t bitmapSize = (desc.box_w * desc.box_h * desc.bpp + 7) / 8;
    
    // 确保缓冲区足够大
    if (bitmapSize > bitmapBufferSize) {
        uint8_t* newBuffer = (uint8_t*)realloc(bitmapBuffer, bitmapSize);
        if (newBuffer == nullptr) {
            Serial.printf("[FontReader] 重新分配位图缓冲区失败: %d bytes\n", bitmapSize);
            return false;
        }
        bitmapBuffer = newBuffer;
        bitmapBufferSize = bitmapSize;
    }
    
    // 读取位图数据
    if (!readGlyphBitmap(desc.bitmap_index, bitmapBuffer, bitmapSize)) {
        Serial.printf("[FontReader] 读取位图失败: unicode=0x%lX\n", unicode);
        return false;
    }
    
    // 填充输出结构
    output->bitmap = bitmapBuffer;
    output->width = desc.box_w;
    output->height = desc.box_h;
    output->offset_x = desc.ofs_x;
    output->offset_y = desc.ofs_y;
    output->advance_x = desc.adv_w;
    output->bitmap_size = bitmapSize;
    
    return true;
}

uint16_t FontReader::getCharWidth(uint32_t unicode) {
    int32_t glyphIndex = findGlyphIndex(unicode);
    if (glyphIndex < 0) return 0;
    
    return glyphs[glyphIndex].adv_w;
}

uint32_t FontReader::getStringWidth(const char* text) {
    if (!initialized || text == nullptr) return 0;
    
    uint32_t width = 0;
    while (*text) {
        width += getCharWidth((uint32_t)*text);
        text++;
    }
    return width;
}

uint32_t FontReader::getUTF8StringWidth(const char* text) {
    if (!initialized || text == nullptr) return 0;
    
    uint32_t width = 0;
    while (*text) {
        uint32_t unicode = 0;
        int bytes = 0;
        
        // UTF-8解码
        if ((*text & 0x80) == 0) {
            // 1字节ASCII
            unicode = *text;
            bytes = 1;
        } else if ((*text & 0xE0) == 0xC0) {
            // 2字节
            unicode = (*text & 0x1F) << 6;
            bytes = 2;
        } else if ((*text & 0xF0) == 0xE0) {
            // 3字节
            unicode = (*text & 0x0F) << 12;
            bytes = 3;
        } else if ((*text & 0xF8) == 0xF0) {
            // 4字节
            unicode = (*text & 0x07) << 18;
            bytes = 4;
        }
        
        // 读取后续字节
        for (int i = 1; i < bytes; i++) {
            if ((text[i] & 0xC0) != 0x80) {
                // 无效的UTF-8
                break;
            }
            unicode |= (text[i] & 0x3F) << (6 * (bytes - i - 1));
        }
        
        width += getCharWidth(unicode);
        text += bytes;
    }
    return width;
}

uint8_t* FontReader::renderText(const char* text, uint32_t* out_width, uint32_t* out_height) {
    if (!initialized || text == nullptr) return nullptr;
    
    // 简化版本：只计算总宽度和最大高度
    uint32_t totalWidth = getUTF8StringWidth(text);
    uint32_t maxHeight = header.line_height;
    
    if (out_width) *out_width = totalWidth;
    if (out_height) *out_height = maxHeight;
    
    // 这里可以添加实际的位图渲染代码
    // 返回一个包含所有字符位图的内存块
    
    return nullptr;  // 简化版本先返回null
}

void FontReader::freeGlyph(glyph_render_t* glyph) {
    // 注意：bitmap指向内部缓冲区，不需要释放
    if (glyph) {
        glyph->bitmap = nullptr;
        glyph->width = 0;
        glyph->height = 0;
        glyph->bitmap_size = 0;
    }
}

bool FontReader::hasGlyph(uint32_t unicode) {
    return findGlyphIndex(unicode) >= 0;
}