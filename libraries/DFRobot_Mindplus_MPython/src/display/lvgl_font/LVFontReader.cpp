#include "LVFontReader.h"

#define CMAP_FORMAT0_FULL   0
#define CMAP_SPARSE_FULL    1
#define CMAP_FORMAT0_TINY   2
#define CMAP_SPARSE_TINY    3

FontReader::FontReader()
    : _part(nullptr), _cmaps(nullptr), _cmap_num(0)
    , _loca_offset(0), _loca_count(0), _glyf_offset(0)
    , _bmp_buf(nullptr), _bmp_buf_size(0)
    , _bit_part_offset(0), _bit_buf(0), _bit_pos(-1) {
    memset(&_header, 0, sizeof(_header));
}

FontReader::~FontReader() {
    if (_cmaps) {
        for (uint32_t i = 0; i < _cmap_num; i++) {
            free(_cmaps[i].unicode_list);
            free(_cmaps[i].glyph_id_ofs);
        }
        free(_cmaps);
    }
    free(_bmp_buf);
}

bool FontReader::readPart(uint32_t offset, void* buf, size_t size) {
    if (!_part || offset + size > _part->size) return false;
    return esp_partition_read(_part, offset, buf, size) == ESP_OK;
}

bool FontReader::readLabel(uint32_t offset, const char* label, uint32_t* length_out) {
    uint32_t length; char tag[4];
    if (!readPart(offset, &length, 4)) return false;
    if (!readPart(offset + 4, tag, 4)) return false;
    if (memcmp(tag, label, 4) != 0) {
        printf("[FontReader] expected '%s' got '%.4s'\n", label, tag);
        return false;
    }
    *length_out = length;
    return true;
}

bool FontReader::loadHeader(uint32_t offset, uint32_t* next) {
    uint32_t length;
    if (!readLabel(offset, "head", &length)) return false;
    if (!readPart(offset + 8, &_header, sizeof(_header))) return false;
    printf("[FontReader] ascent=%d descent=%d bpp=%d\n",
        _header.ascent, _header.descent, _header.bits_per_pixel);
    *next = offset + length;
    return true;
}

bool FontReader::loadCmaps(uint32_t offset, uint32_t* next) {
    uint32_t length;
    if (!readLabel(offset, "cmap", &length)) return false;
    uint32_t count;
    if (!readPart(offset + 8, &count, 4)) return false;

    _cmaps = (lv_cmap_t*)calloc(count, sizeof(lv_cmap_t));
    if (!_cmaps) return false;
    _cmap_num = count;

    uint32_t tables_offset = offset + 12;
    printf("[FontReader] cmap_table_t size=%u\n", sizeof(lv_cmap_table_t));
    for (uint32_t i = 0; i < count && i < 3; i++) {
        lv_cmap_table_t tbl2;
        readPart(tables_offset + i * sizeof(lv_cmap_table_t), &tbl2, sizeof(tbl2));
        printf("  tbl[%lu] @%lu data_off=%lu range=0x%lX fmt=%d entries=%d\n",
            i, tables_offset + i * sizeof(lv_cmap_table_t),
            tbl2.data_offset, tbl2.range_start, tbl2.format_type, tbl2.data_entries_count);
    }
    for (uint32_t i = 0; i < count; i++) {
        lv_cmap_table_t tbl;
        if (!readPart(tables_offset + i * sizeof(lv_cmap_table_t), &tbl, sizeof(tbl))) return false;

        lv_cmap_t& c = _cmaps[i];
        c.range_start    = tbl.range_start;
        c.range_length   = tbl.range_length;
        c.glyph_id_start = tbl.glyph_id_start;
        c.format_type    = tbl.format_type;

        uint32_t data_abs = offset + tbl.data_offset;
        switch (tbl.format_type) {
            case CMAP_FORMAT0_FULL: {
                c.list_length = tbl.data_entries_count;
                uint8_t* tmp = (uint8_t*)malloc(tbl.data_entries_count);
                if (!tmp) return false;
                if (!readPart(data_abs, tmp, tbl.data_entries_count)) { free(tmp); return false; }
                c.glyph_id_ofs = (uint16_t*)malloc(tbl.data_entries_count * sizeof(uint16_t));
                if (!c.glyph_id_ofs) { free(tmp); return false; }
                for (uint16_t j = 0; j < tbl.data_entries_count; j++) c.glyph_id_ofs[j] = tmp[j];
                free(tmp);
                break;
            }
            case CMAP_FORMAT0_TINY:
                break;
            case CMAP_SPARSE_FULL:
            case CMAP_SPARSE_TINY: {
                c.list_length = tbl.data_entries_count;
                c.unicode_list = (uint16_t*)malloc(tbl.data_entries_count * sizeof(uint16_t));
                if (!c.unicode_list) return false;
                if (!readPart(data_abs, c.unicode_list, tbl.data_entries_count * sizeof(uint16_t))) return false;
                if (tbl.format_type == CMAP_SPARSE_FULL) {
                    c.glyph_id_ofs = (uint16_t*)malloc(tbl.data_entries_count * sizeof(uint16_t));
                    if (!c.glyph_id_ofs) return false;
                    if (!readPart(data_abs + tbl.data_entries_count * sizeof(uint16_t),
                                  c.glyph_id_ofs, tbl.data_entries_count * sizeof(uint16_t))) return false;
                }
                break;
            }
        }
    }
    *next = offset + length;
    return true;
}

bool FontReader::scanLoca(uint32_t offset, uint32_t* next) {
    uint32_t length;
    if (!readLabel(offset, "loca", &length)) return false;
    uint32_t count;
    if (!readPart(offset + 8, &count, 4)) return false;
    _loca_offset = offset + 12;
    _loca_count  = count;
    *next = offset + length;
    return true;
}

bool FontReader::readGlyphOffset(uint32_t glyph_id, uint32_t* out) {
    if (glyph_id >= _loca_count) return false;
    if (_header.index_to_loc_format == 0) {
        uint16_t v;
        if (!readPart(_loca_offset + glyph_id * 2, &v, 2)) return false;
        *out = v;
    } else {
        if (!readPart(_loca_offset + glyph_id * 4, out, 4)) return false;
    }
    return true;
}

bool FontReader::init() {
    _part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, (esp_partition_subtype_t)0x40, "font");
    if (!_part) { printf("[FontReader] partition not found\n"); return false; }
    printf("[FontReader] addr=0x%08lX size=%lu\n", _part->address, _part->size);

    uint32_t next;
    if (!loadHeader(0, &next)) return false;
    if (!loadCmaps(next, &next)) return false;
    if (!scanLoca(next, &next)) return false;
    _glyf_offset = next;
    printf("[FontReader] ready cmaps=%lu loca=%lu loc_fmt=%d\n", _cmap_num, _loca_count, _header.index_to_loc_format);
    // 打印cmap[9]的unicode_list前10个
    if (_cmap_num > 9 && _cmaps[9].unicode_list) {
        printf("cmap[9] fmt=%d range=0x%lX list=%d: ", _cmaps[9].format_type, _cmaps[9].range_start, _cmaps[9].list_length);
        for (int i = 0; i < 10 && i < _cmaps[9].list_length; i++)
            printf("0x%X ", _cmaps[9].range_start + _cmaps[9].unicode_list[i]);
        printf("\n");
    }
    return true;
}

int32_t FontReader::findGlyphId(uint32_t unicode) {
    for (uint32_t i = 0; i < _cmap_num; i++) {
        lv_cmap_t& c = _cmaps[i];
        switch (c.format_type) {
            case CMAP_FORMAT0_TINY:
                if (unicode >= c.range_start && unicode < c.range_start + c.range_length)
                    return c.glyph_id_start + (unicode - c.range_start);
                break;
            case CMAP_FORMAT0_FULL:
                if (unicode >= c.range_start && unicode < c.range_start + c.range_length) {
                    uint16_t ofs = c.glyph_id_ofs[unicode - c.range_start];
                    return ofs ? c.glyph_id_start + ofs : 0;
                }
                break;
            case CMAP_SPARSE_TINY:
            case CMAP_SPARSE_FULL: {
                if (unicode < c.range_start || unicode >= c.range_start + c.range_length) break;
                uint16_t rel = (uint16_t)(unicode - c.range_start);
                int lo = 0, hi = (int)c.list_length - 1;
                while (lo <= hi) {
                    int mid = (lo + hi) / 2;
                    if (c.unicode_list[mid] == rel)
                        return c.format_type == CMAP_SPARSE_FULL
                            ? c.glyph_id_start + c.glyph_id_ofs[mid]
                            : c.glyph_id_start + mid;
                    else if (c.unicode_list[mid] < rel) lo = mid + 1;
                    else hi = mid - 1;
                }
                break;
            }
        }
    }
    return -1;
}

void FontReader::initBits(uint32_t byte_offset) {
    _bit_part_offset = byte_offset;
    _bit_pos = -1;
    _bit_buf = 0;
}

uint32_t FontReader::readBits(int n) {
    uint32_t value = 0;
    for (int i = n - 1; i >= 0; i--) {
        if (_bit_pos < 0) {
            readPart(_bit_part_offset++, &_bit_buf, 1);
            _bit_pos = 7;
        }
        value |= (uint32_t)((_bit_buf >> _bit_pos) & 1) << i;
        _bit_pos--;
    }
    return value;
}

int32_t FontReader::readBitsSigned(int n) {
    uint32_t v = readBits(n);
    if (v & (1u << (n - 1))) v |= ~0u << n;
    return (int32_t)v;
}

bool FontReader::getGlyph(uint32_t unicode, glyph_render_t* out) {
    int32_t id = findGlyphId(unicode);
    if (id <= 0) return false;

    uint32_t glyph_off, next_off;
    if (!readGlyphOffset(id, &glyph_off)) return false;
    if (!readGlyphOffset(id + 1, &next_off)) return false;

    int nbits = _header.advance_width_bits + 2 * _header.xy_bits + 2 * _header.wh_bits;
    initBits(_glyf_offset + glyph_off);

    uint16_t adv_w;
    if (_header.advance_width_bits == 0) {
        adv_w = _header.default_advance_width;
    } else {
        adv_w = readBits(_header.advance_width_bits);
        if (_header.advance_width_format == 0) adv_w *= 16;
    }
    int8_t  ofs_x = readBitsSigned(_header.xy_bits);
    int8_t  ofs_y = readBitsSigned(_header.xy_bits);
    uint8_t box_w = readBits(_header.wh_bits);
    uint8_t box_h = readBits(_header.wh_bits);

    if (box_w == 0 || box_h == 0) return false;

    int bmp_size = (int)(next_off - glyph_off) - nbits / 8;
    if (bmp_size <= 0) return false;

    if ((uint32_t)bmp_size > _bmp_buf_size) {
        free(_bmp_buf);
        _bmp_buf = (uint8_t*)malloc(bmp_size);
        if (!_bmp_buf) { _bmp_buf_size = 0; return false; }
        _bmp_buf_size = bmp_size;
    }

    if (nbits % 8 == 0) {
        if (!readPart(_glyf_offset + glyph_off + nbits / 8, _bmp_buf, bmp_size)) return false;
    } else {
        // bit-unaligned: already positioned after descriptor bits
        for (int k = 0; k < bmp_size - 1; k++)
            _bmp_buf[k] = readBits(8);
        _bmp_buf[bmp_size - 1] = readBits(8 - nbits % 8) << (nbits % 8);
    }

    out->bitmap    = _bmp_buf;
    out->width     = box_w;
    out->height    = box_h;
    out->offset_x  = ofs_x;
    out->offset_y  = ofs_y;
    out->advance_x = adv_w >> 4;
    return true;
}
