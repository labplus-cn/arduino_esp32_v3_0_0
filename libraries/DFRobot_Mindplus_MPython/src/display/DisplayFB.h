#ifndef DisplayFB_h
#define DisplayFB_h

#include "Arduino.h"
#include "lcd/LCD_jd9853.h"

// 字体结构定义
typedef struct {
    uint16_t bitmapOffset;   // 指向字体位图的偏移
    uint8_t width, height;   // 位图的宽度和高度（像素）
    uint8_t xAdvance;        // 光标前进距离（x轴）
    int8_t xOffset, yOffset; // 从光标位置到左上角的距离
} GFXglyph;

typedef struct {
    uint8_t *bitmap;     // 字形位图，连接在一起
    GFXglyph *glyph;     // 字形数组
    uint8_t first, last; // ASCII范围
    uint8_t yAdvance;    // 换行距离（y轴）
    uint8_t yOffset;     // 字体零线的Y偏移（y轴）
} GFXfont;

// 包含字体库
#include "fb_gfx/FreeMonoBold12pt7b.h"

// 像素格式定义
typedef enum {
    PIXFORMAT_GRAYSCALE, PIXFORMAT_RGB565, PIXFORMAT_RGB888,
} pixformat_t;

// 图像帧缓冲区结构
typedef struct {
    uint8_t *buf;     // 帧缓冲区数据
    int width;        // 宽度
    int height;       // 高度
    int format;       // 像素格式
} image_fb_t;

class DisplayFB {
public:
    // 构造函数
    DisplayFB();
    // 初始化函数
    esp_err_t begin();
    // 清空屏幕
    void fillScreen(uint16_t color);
    // 绘制文本
    void drawText(int x, int y, const char *text, uint16_t color);
    // 绘制中文字符
    void drawTextCN(int x, int y, const char *text, uint16_t color);
    // 绘制直线
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    // 绘制矩形
    void drawRect(int x, int y, int width, int height, uint16_t color);
    // 填充矩形
    void fillRect(int x, int y, int width, int height, uint16_t color);
    // 绘制圆形
    void drawCircle(int x0, int y0, int radius, uint16_t color);
    // 填充圆形
    void fillCircle(int x0, int y0, int radius, uint16_t color);
    // 显示logo
    void showLogo();

private:
    // 帧缓冲区
    image_fb_t _fb;
    // LCD对象
    LCD_jd9853 _lcd;
    // 屏幕宽度
    int _width;
    // 屏幕高度
    int _height;
    // 内部绘图函数
    void fillRect(image_fb_t *fb, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void drawFastHLine(image_fb_t *fb, int32_t x, int32_t y, int32_t w, uint16_t color);
    void drawFastVLine(image_fb_t *fb, int32_t x, int32_t y, int32_t h, uint16_t color);
    uint8_t putc(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, unsigned char c);
    uint32_t print(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, const char * str);
    uint32_t printf(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, const char *format, va_list args);
};

#endif