#include "DisplayFB.h"
#include "esp_partition.h"
#include "lvgl_font/LVFontReader.h"

extern "C" {
#include "qrcode/qrcodegen.h"
}

// 字体定义
#define gfxFont ((GFXfont *)(&FreeMonoBold12pt7b))

// 构造函数
DisplayFB::DisplayFB() : _bgColor(0x0000), _fontReader(nullptr) {
}

// 初始化函数
esp_err_t DisplayFB::begin() {
    // 初始化LCD
    esp_err_t ret = _lcd.init();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 获取屏幕尺寸
    _width = BOARD_LCD_H_RES;
    _height = BOARD_LCD_V_RES;
    
    // 分配帧缓冲区
    // 使用 PSRAM 分配帧缓冲区，避免占用内部 DMA RAM。
    // show() 调用 esp_lcd_panel_draw_bitmap 时，IDF SPI LCD 驱动内部会将数据
    // 分段复制到内部 DMA 缓冲区再传输，不要求帧缓冲区本身在 DMA RAM 中。
    _fb.width = _width;
    _fb.height = _height;
    _fb.format = PIXFORMAT_RGB565;
    _fb.buf = (uint8_t *)heap_caps_malloc(_width * _height * 2, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (_fb.buf == NULL) {
        // PSRAM 不可用时回退到内部 RAM
        _fb.buf = (uint8_t *)heap_caps_malloc(_width * _height * 2, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    }
    if (_fb.buf == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    _lcd.show_logo(); // 显示logo
    // fillScreen(0x0000); // 清空帧缓冲区，避免未初始化数据后续被 show() 输出
    
    return ESP_OK;
}

// 清空屏幕
void DisplayFB::fillScreen(uint16_t color) {
    _bgColor = color;
    fillRect(&_fb, 0, 0, _width, _height, color);
}

// 擦除指定行
void DisplayFB::clearLine(int line) {
    int y = 10 + line * 30;
    fillRect(&_fb, 0, y - 10, _width, 35, _bgColor);
}

// 绘制文本
void DisplayFB::drawText(int x, int y, const char *text, uint16_t color) {
    print(&_fb, x, y, color, text);
}

// 绘制直线
void DisplayFB::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (true) {
        if (x >= 0 && x < _width && y >= 0 && y < _height) {
            int index = (y * _width + x) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        
        if (x == x1 && y == y1) {
            break;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    
}

// 绘制矩形
void DisplayFB::drawRect(int x, int y, int width, int height, uint16_t color) {
    drawFastHLine(&_fb, x, y, width, color);
    drawFastHLine(&_fb, x, y + height - 1, width, color);
    drawFastVLine(&_fb, x, y, height, color);
    drawFastVLine(&_fb, x + width - 1, y, height, color);
}

// 填充矩形
void DisplayFB::fillRect(int x, int y, int width, int height, uint16_t color) {
    fillRect(&_fb, x, y, width, height, color);
}

// 绘制圆形
void DisplayFB::drawCircle(int x0, int y0, int radius, uint16_t color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        if (x0 + x < _width && y0 + y < _height) {
            int index = ((y0 + y) * _width + (x0 + x)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 + y < _width && y0 + x < _height) {
            int index = ((y0 + x) * _width + (x0 + y)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 - y >= 0 && y0 + x < _height) {
            int index = ((y0 + x) * _width + (x0 - y)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 - x >= 0 && y0 + y < _height) {
            int index = ((y0 + y) * _width + (x0 - x)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 - x >= 0 && y0 - y >= 0) {
            int index = ((y0 - y) * _width + (x0 - x)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 - y >= 0 && y0 - x >= 0) {
            int index = ((y0 - x) * _width + (x0 - y)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 + y < _width && y0 - x >= 0) {
            int index = ((y0 - x) * _width + (x0 + y)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        if (x0 + x < _width && y0 - y >= 0) {
            int index = ((y0 - y) * _width + (x0 + x)) * 2;
            _fb.buf[index] = color & 0xFF;
            _fb.buf[index + 1] = color >> 8;
        }
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
    
}

// 填充圆形
void DisplayFB::fillCircle(int x0, int y0, int radius, uint16_t color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        drawFastHLine(&_fb, x0 - x, y0 + y, 2 * x + 1, color);
        drawFastHLine(&_fb, x0 - y, y0 + x, 2 * y + 1, color);
        drawFastHLine(&_fb, x0 - y, y0 - x, 2 * y + 1, color);
        drawFastHLine(&_fb, x0 - x, y0 - y, 2 * x + 1, color);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
    
}

// 显示logo
void DisplayFB::showLogo() {
    _lcd.show_logo();
}

// 显示二维码
void DisplayFB::drawQRCode(int x, int y, const char *text, int scale) {
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuf[qrcodegen_BUFFER_LEN_MAX];
    if (!qrcodegen_encodeText(text, tempBuf, qrcode,
            qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
            qrcodegen_Mask_AUTO, true)) {
        return;
    }
    int size = qrcodegen_getSize(qrcode);
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            uint16_t color = qrcodegen_getModule(qrcode, col, row) ? 0x0000 : 0xFFFF;
            fillRect(&_fb, x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}



// 绘制中文字符
void DisplayFB::drawTextCN(int x, int y, const char *text, uint16_t color, bool wrap) {
    // 懒初始化字体读取器
    if (_fontReader == nullptr) {
        _fontReader = new FontReader();
        if (!_fontReader->init()) {
            delete _fontReader;
            _fontReader = nullptr;
            return;
        }
    }

    int xc = x;
    int yc = y;
    int line_height = _fontReader->getLineHeight();
    int baseline    = _fontReader->getBaseLine();

    while (*text) {
        // UTF-8解码
        uint32_t code = 0;
        int char_len = 0;
        if ((*text & 0x80) == 0) {
            code = *text; char_len = 1;
        } else if ((*text & 0xE0) == 0xC0 && text[1]) {
            code = ((text[0] & 0x1F) << 6) | (text[1] & 0x3F); char_len = 2;
        } else if ((*text & 0xF0) == 0xE0 && text[1] && text[2]) {
            code = ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F); char_len = 3;
        } else if ((*text & 0xF8) == 0xF0 && text[1] && text[2] && text[3]) {
            code = ((text[0] & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F); char_len = 4;
        } else {
            text++; continue;
        }
        text += char_len;

        // 换行
        if (code == '\n') { yc += line_height; xc = x; continue; }
        if (code == '\r') { continue; }

        glyph_render_t glyph;
        if (!_fontReader->getGlyph(code, &glyph)) {
            ::printf("[drawTextCN] glyph not found: U+%04lX\n", code);
            xc += _fontReader->getFontSize() / 2;
            continue;
        }
        ::printf("[drawTextCN] U+%04lX w=%d h=%d ox=%d oy=%d adv=%d\n",
               code, glyph.width, glyph.height, glyph.offset_x, glyph.offset_y, glyph.advance_x);

        // 自动换行
        if (xc + glyph.advance_x > _width) {
            if (!wrap) break;
            yc += line_height - 7; xc = x;
        }

        uint8_t font_bpp = _fontReader->getBpp();
        uint8_t mask = (1 << font_bpp) - 1;

        int px_x = xc + glyph.offset_x;
        int px_y = yc + baseline - glyph.offset_y - glyph.height;

        int bit_pos = 0;
        for (int row = 0; row < glyph.height; row++) {
            for (int col = 0; col < glyph.width; col++) {
                int byte_idx = bit_pos / 8;
                int bit_off  = 7 - (bit_pos % 8);
                uint8_t alpha = (glyph.bitmap[byte_idx] >> (bit_off - (font_bpp - 1))) & mask;
                bit_pos += font_bpp;

                if (alpha > 0) {
                    int fx = px_x + col;
                    int fy = px_y + row;
                    if (fx >= 0 && fx < _width && fy >= 0 && fy < _height) {
                        int idx = (fy * _width + fx) * 2;
                        _fb.buf[idx]     = color & 0xFF;
                        _fb.buf[idx + 1] = color >> 8;
                    }
                }
            }
        }

        xc += glyph.advance_x;
    }
}

void DisplayFB::drawTextCN(int line, const char *text, uint16_t color, bool wrap) {
    drawTextCN(5, 10 + line * 30, text, color, wrap);
}

void DisplayFB::show() {
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
}

// 内部绘图函数实现
void DisplayFB::fillRect(image_fb_t *fb, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    int bytes_per_pixel = 0;
    switch (fb->format) {
    case PIXFORMAT_GRAYSCALE:
        bytes_per_pixel = 1;
        break;
    case PIXFORMAT_RGB565:
        bytes_per_pixel = 2;
        break;
    case PIXFORMAT_RGB888:
        bytes_per_pixel = 3;
    default:
        break;
    }
    int32_t line_step = (fb->width - w) * bytes_per_pixel;
    uint8_t *data = fb->buf + ((x + (y * fb->width)) * bytes_per_pixel);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            switch (bytes_per_pixel) {
            case 1:
                break;
            case 2:
                data[0] = color & 0xFF;
                data[1] = color >> 8;
                data += 2;
                break;
            case 3:
                break;
            default:
                break;
            }
        }
        data += line_step;
    }
}

void DisplayFB::drawFastHLine(image_fb_t *fb, int32_t x, int32_t y, int32_t w, uint16_t color) {
    fillRect(fb, x, y, w, 1, color);
}

void DisplayFB::drawFastVLine(image_fb_t *fb, int32_t x, int32_t y, int32_t h, uint16_t color) {
    fillRect(fb, x, y, 1, h, color);
}

uint8_t DisplayFB::putc(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, unsigned char c) {
    uint16_t line_width;
    uint8_t xa = 0, bit = 0, bits = 0, xx, yy;
    uint8_t *bitmap;
    GFXglyph *glyph;

    if ((c < 32) || (c < gfxFont->first) || (c > gfxFont->last)) {
        return xa;
    }

    c -= gfxFont->first;

    glyph = &(gfxFont->glyph[c]);
    bitmap = gfxFont->bitmap + glyph->bitmapOffset;

    xa = glyph->xAdvance;
    x += glyph->xOffset;
    y += glyph->yOffset;
    y += gfxFont->yOffset;
    line_width = 0;

    for (yy = 0; yy < glyph->height; yy++) {
        for (xx = 0; xx < glyph->width; xx++) {
            if (bit == 0) {
                bits = *bitmap++;
                bit = 0x80;
            }
            if (bits & bit) {
                line_width++;
            } else if (line_width) {
                drawFastHLine(fb, x + xx - line_width, y + yy, line_width, color);
                line_width = 0;
            }
            bit >>= 1;
        }
        if (line_width) {
            drawFastHLine(fb, x + xx - line_width, y + yy, line_width, color);
            line_width = 0;
        }
    }
    return xa;
}

uint32_t DisplayFB::print(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, const char *str) {
    uint32_t l = 0;
    int xc = x, yc = y, lc = fb->width - gfxFont->glyph[0].xAdvance;
    uint8_t fh = gfxFont->yAdvance;
    char c = *str++;
    while (c) {
        if (c != '\r') {
            if (c == '\n') {
                yc += fh;
                xc = x;
            } else {
                if (xc > lc) {
                    yc += fh;
                    xc = x;
                }
                xc += putc(fb, xc, yc, color, c);
            }
        }
        l++;
        c = *str++;
    }
    return l;
}

