#ifndef _LCD_JD9853_H_
#define _LCD_JD9853_H_

#include "esp_lcd_types.h"
#include "esp_lcd_panel_jd9853.h"

// 颜色常量定义
#define GUI_HotPink		            0x56FB		//热情的粉红
#define GUI_DeepPink		        0xB2F8		//深粉色
#define GUI_Purple		            0x1080		//紫色
#define GUI_Blue		            0x1F00		//纯蓝
#define GUI_MediumBlue		        0x1900		//适中的蓝色
#define GUI_DarkBlue		        0x1100		//深蓝色
#define GUI_LightSkyBlue		    0x7F86		//淡蓝色
#define GUI_SkyBlue		            0x7D86		//天蓝色
#define GUI_DeepSkyBlue		        0xFF05		//深天蓝
#define GUI_LightBLue		        0xDCAE		//淡蓝
#define GUI_LightCyan		        0xFFE7		//淡青色
#define GUI_Cyan		            0xFF07		//青色
#define GUI_DarkCyan		        0x5104		//深青色
#define GUI_SpringGreen		        0x8E3D		//春天的绿色
#define GUI_LightGreen		        0x7297		//淡绿色
#define GUI_Green		            0x0004		//纯绿
#define GUI_DarkGreen		        0x2003		//深绿色
#define GUI_GreenYellow		        0xE5AF		//绿黄色
#define GUI_LightYellow		        0xFCFF		//浅黄色
#define GUI_Yellow		            0xE0FF		//纯黄
#define GUI_Gold		            0xA0FE		//金
#define GUI_Orange		            0x20FD		//橙色
#define GUI_DarkOrange		        0x60FC		//深橙色
#define GUI_Red			            0x00F8		//纯红
#define GUI_DarkRed		            0x0088		//深红色
#define GUI_Pink		            0x19FE		//粉红
#define GUI_Brown		            0x45A1		//棕色
#define GUI_White		            0xFFFF		//纯白
#define GUI_LightGray		        0x9AD6		//浅灰色
#define GUI_DarkGray	            0x55AD		//深灰色
#define GUI_Gray		            0x1084		//灰色
#define GUI_Black		            0x0000		//纯黑

// 板类型定义
typedef enum {
    BOARD_MPYTHON_V3,      // 掌控板mpython v3
    BOARD_LABPLUS_LEDONG_V2  // labplus ledong v2
} BoardType;

// 屏幕参数定义 - 共有的参数
#define BOARD_LCD_MOSI 37
#define BOARD_LCD_MISO -1
#define BOARD_LCD_SCK 36
#define BOARD_LCD_DC 35
#define BOARD_LCD_RST -1
#define BOARD_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define BOARD_LCD_BK_LIGHT_ON_LEVEL 1
#define BOARD_LCD_BK_LIGHT_OFF_LEVEL !BOARD_LCD_BK_LIGHT_ON_LEVEL
#define BOARD_LCD_H_RES 320
#define BOARD_LCD_V_RES 172
#define BOARD_LCD_CMD_BITS 8
#define BOARD_LCD_PARAM_BITS 8
#define BOARD_LCD_RGB_ELE_ORDER LCD_RGB_ELEMENT_ORDER_BGR
#define BOARD_LCD_SWAP_XY    true
#define BOARD_LCD_MIRROR_X   true
#define BOARD_LCD_MIRROR_Y   true
#define BOARD_LCD_GAP_X      0
#define BOARD_LCD_GAP_Y      34
#define BOARD_LCD_INVERT     false

// 不同板型的参数
#ifdef ARDUINO_MPYTHON_V3
    #define BOARD_LCD_CS 34
    #define BOARD_LCD_BL GPIO_NUM_33
#elif defined(ARDUINO_LABPLUS_LEDONG_V2)
    #define BOARD_LCD_CS -1
    #define BOARD_LCD_BL GPIO_NUM_NC
    #define BOARD_STM8_ADDR 17
    #define BOARD_STM8_CMD 4
#else
    // 默认参数 - mPython V3
    #define BOARD_LCD_CS 34
    #define BOARD_LCD_BL GPIO_NUM_33
#endif

class LCD_jd9853 {
public:
    LCD_jd9853();
    ~LCD_jd9853();

    esp_err_t init();
    esp_err_t deinit();
    esp_err_t draw_bitmap(int x, int y, int width, int height, void *data);
    esp_err_t fill(int x, int y, int width, int height, uint16_t color);
    esp_err_t set_backlight(float brightness);
    esp_err_t show_logo(void);

private:
    esp_lcd_panel_handle_t _panel_handle;
    esp_lcd_panel_io_handle_t _io_handle;
    bool _lcd_initialized;
};

#endif /* _LCD_JD9853_H_ */