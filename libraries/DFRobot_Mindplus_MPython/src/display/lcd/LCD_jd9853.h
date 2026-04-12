#ifndef LCD_jd9853_h
#define LCD_jd9853_h

#include "Arduino.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

// 共有参数定义
#define BOARD_LCD_MOSI 37
#define BOARD_LCD_MISO -1
#define BOARD_LCD_SCK 36
#define BOARD_LCD_DC 35
#define BOARD_LCD_RST -1
#define BOARD_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define BOARD_LCD_BK_LIGHT_ON_LEVEL 0
#define BOARD_LCD_BK_LIGHT_OFF_LEVEL !BOARD_LCD_BK_LIGHT_ON_LEVEL
#define BOARD_LCD_H_RES 320
#define BOARD_LCD_V_RES 172
#define BOARD_LCD_CMD_BITS 8
#define BOARD_LCD_PARAM_BITS 8
#define BOARD_LCD_RGB_ELE_ORDER LCD_RGB_ENDIAN_BGR
#define BOARD_LCD_SWAP_XY    true
#define BOARD_LCD_MIRROR_X   true
#define BOARD_LCD_MIRROR_Y   true
#define BOARD_LCD_GAP_X      0
#define BOARD_LCD_GAP_Y      34
#define BOARD_LCD_INVERT     false

// 板型特定参数
#ifdef ARDUINO_MPYTHON_V3
    #define BOARD_LCD_CS 34
    #define BOARD_LCD_BL 33
#elif defined(ARDUINO_LABPLUS_LEDONG_V2)
    #define BOARD_LCD_CS -1
    #define BOARD_LCD_BL -1
    #define BOARD_STM8_ADDR 17
    #define BOARD_STM8_CMD 4
#else
    // 默认参数 - mPython V3
    #define BOARD_LCD_CS 34
    #define BOARD_LCD_BL 33
#endif

class LCD_jd9853 {
public:
    // 构造函数
    LCD_jd9853();
    // 初始化函数
    esp_err_t init();
    // 反初始化函数
    esp_err_t deinit();
    // 绘制位图
    esp_err_t draw_bitmap(int x, int y, int width, int height, void *data);
    // 填充区域
    esp_err_t fill(int x, int y, int width, int height, uint16_t color);
    // 设置背光
    esp_err_t set_backlight(float brightness);
    // 显示logo
    esp_err_t show_logo();

private:
    // 初始化状态
    bool _lcd_initialized;
    // SPI总线句柄
    spi_device_handle_t _spi_handle;
    // LCD面板IO句柄
    esp_lcd_panel_io_handle_t _io_handle;
    // LCD面板句柄
    esp_lcd_panel_handle_t _panel_handle;
};

#endif