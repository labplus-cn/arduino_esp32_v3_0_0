#ifndef DisplayFB_h
#define DisplayFB_h

#include "Arduino.h"
#include "lcd/LCD_jd9853.h"

class FontReader;

/**
 * @brief 字体字形结构定义
 *
 * 描述字体中单个字形的属性
 */
typedef struct {
    uint16_t bitmapOffset;   /*!< 指向字体位图的偏移 */
    uint8_t width, height;   /*!< 位图的宽度和高度（像素） */
    uint8_t xAdvance;        /*!< 光标前进距离（x轴） */
    int8_t xOffset, yOffset; /*!< 从光标位置到左上角的距离 */
} GFXglyph;

/**
 * @brief 字体结构定义
 *
 * 描述整个字体的属性和字形数据
 */
typedef struct {
    uint8_t *bitmap;     /*!< 字形位图，连接在一起 */
    GFXglyph *glyph;     /*!< 字形数组 */
    uint8_t first, last; /*!< ASCII范围 */
    uint8_t yAdvance;    /*!< 换行距离（y轴） */
    uint8_t yOffset;     /*!< 字体零线的Y偏移（y轴） */
} GFXfont;

// 包含字体库
#include "fb_gfx/FreeMonoBold12pt7b.h"

/**
 * @brief 像素格式定义
 *
 * 定义支持的像素格式
 */
typedef enum {
    PIXFORMAT_GRAYSCALE, /*!< 灰度格式 */
    PIXFORMAT_RGB565,     /*!< RGB565格式 */
    PIXFORMAT_RGB888,     /*!< RGB888格式 */
} pixformat_t;

/**
 * @brief 图像帧缓冲区结构
 *
 * 描述图像帧缓冲区的属性
 */
typedef struct {
    uint8_t *buf;     /*!< 帧缓冲区数据 */
    int width;        /*!< 宽度 */
    int height;       /*!< 高度 */
    int format;       /*!< 像素格式 */
} image_fb_t;

/**
 * @brief 显示帧缓冲区类
 *
 * 提供显示屏的基本绘图功能，包括文本、图形等绘制操作
 */
class DisplayFB {
public:
    /**
     * @brief 构造函数
     *
     * 初始化 DisplayFB 实例
     */
    DisplayFB();
    
    /**
     * @brief 初始化函数
     *
     * 初始化显示屏硬件
     *
     * @return esp_err_t 初始化结果，ESP_OK 表示成功
     */
    esp_err_t begin();
    
    /**
     * @brief 清空屏幕
     *
     * 用指定颜色填充整个屏幕
     *
     * @param color 填充颜色，格式为 RGB565
     */
    void fillScreen(uint16_t color);
    
    /**
     * @brief 擦除指定行
     *
     * 用背景色填充指定行
     *
     * @param line 行号，从0开始
     */
    void clearLine(int line);
    
    /**
     * @brief 绘制文本
     *
     * 在指定位置绘制ASCII文本
     *
     * @param x X坐标
     * @param y Y坐标
     * @param text 要绘制的文本
     * @param color 文本颜色，格式为 RGB565
     */
    void drawText(int x, int y, const char *text, uint16_t color);
    
    /**
     * @brief 绘制中文字符
     *
     * 在指定位置绘制中文字符
     *
     * @param x X坐标
     * @param y Y坐标
     * @param text 要绘制的中文文本
     * @param color 文本颜色，格式为 RGB565
     * @param wrap 是否自动换行
     */
    void drawTextCN(int x, int y, const char *text, uint16_t color, bool wrap = true);
    
    /**
     * @brief 按行绘制中文字符
     *
     * 按行绘制中文字符，行起始x=10，第一行y=10，行高30
     *
     * @param line 行号，从0开始
     * @param text 要绘制的中文文本
     * @param color 文本颜色，格式为 RGB565
     * @param wrap 是否自动换行
     */
    void drawTextCN(int line, const char *text, uint16_t color, bool wrap = true);
    
    /**
     * @brief 绘制直线
     *
     * 绘制从(x0,y0)到(x1,y1)的直线
     *
     * @param x0 起点X坐标
     * @param y0 起点Y坐标
     * @param x1 终点X坐标
     * @param y1 终点Y坐标
     * @param color 直线颜色，格式为 RGB565
     */
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    
    /**
     * @brief 绘制矩形
     *
     * 绘制指定大小的矩形边框
     *
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param width 矩形宽度
     * @param height 矩形高度
     * @param color 边框颜色，格式为 RGB565
     */
    void drawRect(int x, int y, int width, int height, uint16_t color);
    
    /**
     * @brief 填充矩形
     *
     * 用指定颜色填充矩形
     *
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param width 矩形宽度
     * @param height 矩形高度
     * @param color 填充颜色，格式为 RGB565
     */
    void fillRect(int x, int y, int width, int height, uint16_t color);
    
    /**
     * @brief 绘制圆形
     *
     * 绘制指定半径的圆形边框
     *
     * @param x0 圆心X坐标
     * @param y0 圆心Y坐标
     * @param radius 圆的半径
     * @param color 边框颜色，格式为 RGB565
     */
    void drawCircle(int x0, int y0, int radius, uint16_t color);
    
    /**
     * @brief 填充圆形
     *
     * 用指定颜色填充圆形
     *
     * @param x0 圆心X坐标
     * @param y0 圆心Y坐标
     * @param radius 圆的半径
     * @param color 填充颜色，格式为 RGB565
     */
    void fillCircle(int x0, int y0, int radius, uint16_t color);
    
    /**
     * @brief 显示logo
     *
     * 显示设备logo
     */
    void showLogo();
    
    /**
     * @brief 显示二维码
     *
     * 在指定位置显示二维码
     *
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param text 二维码内容
     * @param scale 每个模块的像素大小
     */
    void drawQRCode(int x, int y, const char *text, int scale = 3);
    
    /**
     * @brief 将帧缓冲区写入显示屏
     *
     * 将内部帧缓冲区的内容显示到屏幕上
     */
    void show();

private:
    // 帧缓冲区
    image_fb_t _fb;
    // LCD对象
    LCD_jd9853 _lcd;
    // 屏幕宽度
    int _width;
    // 屏幕高度
    int _height;
    // 背景色
    uint16_t _bgColor;
    // 中文字体读取器
    FontReader* _fontReader;
    // 内部绘图函数
    void fillRect(image_fb_t *fb, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void drawFastHLine(image_fb_t *fb, int32_t x, int32_t y, int32_t w, uint16_t color);
    void drawFastVLine(image_fb_t *fb, int32_t x, int32_t y, int32_t h, uint16_t color);
    uint8_t putc(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, unsigned char c);
    uint32_t print(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, const char * str);
    uint32_t printf(image_fb_t *fb, int32_t x, int32_t y, uint16_t color, const char *format, va_list args);
};

#endif