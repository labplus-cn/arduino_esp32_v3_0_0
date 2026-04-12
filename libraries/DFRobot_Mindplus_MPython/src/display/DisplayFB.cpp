#include "DisplayFB.h"

// 字体定义
#define gfxFont ((GFXfont *)(&FreeMonoBold12pt7b))

// 构造函数
DisplayFB::DisplayFB() {
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
    _fb.width = _width;
    _fb.height = _height;
    _fb.format = PIXFORMAT_RGB565;
    _fb.buf = (uint8_t *)heap_caps_malloc(_width * _height * 2, MALLOC_CAP_DMA);
    if (_fb.buf == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    // 清空屏幕
    fillScreen(0x0000);
    
    return ESP_OK;
}

// 清空屏幕
void DisplayFB::fillScreen(uint16_t color) {
    fillRect(&_fb, 0, 0, _width, _height, color);
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
}

// 绘制文本
void DisplayFB::drawText(int x, int y, const char *text, uint16_t color) {
    print(&_fb, x, y, color, text);
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
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
    
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
}

// 绘制矩形
void DisplayFB::drawRect(int x, int y, int width, int height, uint16_t color) {
    drawFastHLine(&_fb, x, y, width, color);
    drawFastHLine(&_fb, x, y + height - 1, width, color);
    drawFastVLine(&_fb, x, y, height, color);
    drawFastVLine(&_fb, x + width - 1, y, height, color);
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
}

// 填充矩形
void DisplayFB::fillRect(int x, int y, int width, int height, uint16_t color) {
    fillRect(&_fb, x, y, width, height, color);
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
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
    
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
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
    
    // 将帧缓冲区数据发送到LCD
    _lcd.draw_bitmap(0, 0, _width, _height, _fb.buf);
}

// 显示logo
void DisplayFB::showLogo() {
    _lcd.show_logo();
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

