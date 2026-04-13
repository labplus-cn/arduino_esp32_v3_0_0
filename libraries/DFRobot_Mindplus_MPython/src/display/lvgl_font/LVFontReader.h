#ifndef __LV_FONT_READER_H
#define __LV_FONT_READER_H

#include <Arduino.h>
#include <esp_partition.h>
#include <vector>

#pragma pack(push, 1)
typedef struct {
    uint32_t version;
    uint16_t tables_count;
    uint16_t font_size;
    uint16_t ascent;
    int16_t  descent;
    uint16_t typo_ascent;
    int16_t  typo_descent;
    uint16_t typo_line_gap;
    int16_t  min_y;
    int16_t  max_y;
    uint16_t default_advance_width;
    uint16_t kerning_scale;
    uint8_t  index_to_loc_format;
    uint8_t  glyph_id_format;
    uint8_t  advance_width_format;
    uint8_t  bits_per_pixel;
    uint8_t  xy_bits;
    uint8_t  wh_bits;
    uint8_t  advance_width_bits;
    uint8_t  compression_id;
    uint8_t  subpixels_mode;
    uint8_t  padding;
    int16_t  underline_position;
    uint16_t underline_thickness;
} lv_font_header_t;

typedef struct {
    uint32_t data_offset;
    uint32_t range_start;
    uint16_t range_length;
    uint16_t glyph_id_start;
    uint16_t data_entries_count;
    uint8_t  format_type;
    uint8_t  padding;
} lv_cmap_table_t;
#pragma pack(pop)

typedef struct {
    uint32_t  range_start;
    uint16_t  range_length;
    uint16_t  glyph_id_start;
    uint8_t   format_type;
    uint16_t  list_length;
    uint16_t* unicode_list;
    uint16_t* glyph_id_ofs;
} lv_cmap_t;

typedef struct {
    const uint8_t* bitmap;
    uint8_t  width;
    uint8_t  height;
    int8_t   offset_x;
    int8_t   offset_y;
    uint16_t advance_x;
} glyph_render_t;

class FontReader {
public:
    FontReader();
    ~FontReader();

    bool init();
    bool getGlyph(uint32_t unicode, glyph_render_t* out);

    int32_t getLineHeight() { return _header.ascent - _header.descent; }
    int32_t getBaseLine()   { return -_header.descent; }
    int16_t getAscent()     { return _header.ascent; }
    int16_t getDescent()    { return _header.descent; }
    uint16_t getFontSize()  { return _header.font_size; }
    uint8_t getBpp()        { return _header.bits_per_pixel; }

private:
    const esp_partition_t* _part;
    lv_font_header_t _header;

    lv_cmap_t*  _cmaps;
    uint32_t    _cmap_num;

    uint32_t    _loca_offset;   // 分区内 loca 数据起始（跳过标签和count）
    uint32_t    _loca_count;
    uint32_t    _glyf_offset;   // 分区内 glyf section 起始

    uint8_t*    _bmp_buf;
    uint32_t    _bmp_buf_size;

    bool readPart(uint32_t offset, void* buf, size_t size);
    bool readLabel(uint32_t offset, const char* label, uint32_t* length_out);
    bool loadHeader(uint32_t offset, uint32_t* next);
    bool loadCmaps(uint32_t offset, uint32_t* next);
    bool scanLoca(uint32_t offset, uint32_t* next);

    int32_t findGlyphId(uint32_t unicode);
    bool    readGlyphOffset(uint32_t glyph_id, uint32_t* out_offset);

    // bit reader
    uint32_t _bit_part_offset;
    uint8_t  _bit_buf;
    int8_t   _bit_pos;
    void     initBits(uint32_t byte_offset);
    uint32_t readBits(int n);
    int32_t  readBitsSigned(int n);
};

#endif
