#include "LCD_jd9853.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_common.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "string.h"

// 日志标签
static const char *TAG = "LCD_jd9853";

LCD_jd9853::LCD_jd9853() {
    _panel_handle = NULL;
    _io_handle = NULL;
    _lcd_initialized = false;
}

LCD_jd9853::~LCD_jd9853() {
    deinit();
}

esp_err_t LCD_jd9853::init() {
    // 检查是否已经初始化
    if (_lcd_initialized) {
        return ESP_OK;
    }

    // 配置SPI总线
    spi_bus_config_t bus_conf = {
        .mosi_io_num = BOARD_LCD_MOSI,
        .miso_io_num = BOARD_LCD_MISO,
        .sclk_io_num = BOARD_LCD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BOARD_LCD_H_RES * BOARD_LCD_V_RES * sizeof(uint16_t),
    };

    // 初始化SPI总线
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_conf, SPI_DMA_CH_AUTO));

    // 配置LCD面板IO
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = BOARD_LCD_CS,
        .dc_gpio_num = BOARD_LCD_DC,
        .spi_mode = 0,
        .pclk_hz = BOARD_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = BOARD_LCD_CMD_BITS,
        .lcd_param_bits = BOARD_LCD_PARAM_BITS,
    };

    // 创建LCD面板IO
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &_io_handle);
    if (ret != ESP_OK) {
        spi_bus_free(SPI2_HOST);
        return ret;
    }

    // 配置LCD面板设备
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_LCD_RST,
        .rgb_ele_order = BOARD_LCD_RGB_ELE_ORDER,
        .bits_per_pixel = 16,
    };

    // 为Ledong V2板添加屏幕复位代码
    #ifdef ARDUINO_LABPLUS_LEDONG_V2
        i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = 44,
            .scl_io_num = 43,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 400000,
        };

        i2c_param_config(0, &conf);
        i2c_driver_install(0, conf.mode, 0, 0, 0);
        uint8_t reg = BOARD_STM8_CMD;
        i2c_master_write_to_device(0, BOARD_STM8_ADDR, &reg, 1, 1000 / portTICK_PERIOD_MS);
        i2c_driver_delete(0);
    #endif

    // 创建LCD面板
    ret = esp_lcd_new_panel_jd9853(_io_handle, &panel_config, &_panel_handle);
    if (ret != ESP_OK) {
        esp_lcd_panel_io_del(_io_handle);
        _io_handle = NULL;
        spi_bus_free(SPI2_HOST);
        return ret;
    }

    // 初始化LCD面板
    ret = esp_lcd_panel_reset(_panel_handle);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ret = esp_lcd_panel_init(_panel_handle);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    // 配置LCD面板
    ret = esp_lcd_panel_invert_color(_panel_handle, BOARD_LCD_INVERT);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    ret = esp_lcd_panel_swap_xy(_panel_handle, BOARD_LCD_SWAP_XY);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    ret = esp_lcd_panel_mirror(_panel_handle, BOARD_LCD_MIRROR_X, BOARD_LCD_MIRROR_Y);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    ret = esp_lcd_panel_set_gap(_panel_handle, BOARD_LCD_GAP_X, BOARD_LCD_GAP_Y);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    // 控制背光
    if (BOARD_LCD_BL >= 0) {
        printf("BOARD_LCD_BL: %d\n", BOARD_LCD_BL);
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << BOARD_LCD_BL,
            .mode = GPIO_MODE_OUTPUT,
        };
        gpio_config(&io_conf);
        gpio_set_level((gpio_num_t)BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_ON_LEVEL);
    }

    // 开启LCD面板
    ret = esp_lcd_panel_disp_on_off(_panel_handle, true);
    if (ret != ESP_OK) {
        deinit();
        return ret;
    }

    // 标记为已初始化
    _lcd_initialized = true;

    return ESP_OK;
}

esp_err_t LCD_jd9853::deinit() {
    if (_panel_handle != NULL) {
        esp_lcd_panel_del(_panel_handle);
        _panel_handle = NULL;
    }

    if (_io_handle != NULL) {
        esp_lcd_panel_io_del(_io_handle);
        _io_handle = NULL;
        spi_bus_free(SPI2_HOST);
    }

    // 关闭背光
    if (BOARD_LCD_BL >= 0) {
        gpio_set_level(BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_OFF_LEVEL);
    }

    // 标记为未初始化
    _lcd_initialized = false;

    return ESP_OK;
}

esp_err_t LCD_jd9853::draw_bitmap(int x, int y, int width, int height, void *data) {
    if (_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // 参数范围限制
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= BOARD_LCD_H_RES) x = BOARD_LCD_H_RES - 1;
    if (y >= BOARD_LCD_V_RES) y = BOARD_LCD_V_RES - 1;
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    if (x + width > BOARD_LCD_H_RES) width = BOARD_LCD_H_RES - x;
    if (y + height > BOARD_LCD_V_RES) height = BOARD_LCD_V_RES - y;

    // 如果宽度或高度为0，直接返回
    if (width <= 0 || height <= 0) {
        return ESP_OK;
    }

    return esp_lcd_panel_draw_bitmap(_panel_handle, x, y, x + width, y + height, (uint16_t *)data);
}

esp_err_t LCD_jd9853::fill(int x, int y, int width, int height, uint16_t color) {
    if (_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // 参数范围限制
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= BOARD_LCD_H_RES) x = BOARD_LCD_H_RES - 1;
    if (y >= BOARD_LCD_V_RES) y = BOARD_LCD_V_RES - 1;
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    if (x + width > BOARD_LCD_H_RES) width = BOARD_LCD_H_RES - x;
    if (y + height > BOARD_LCD_V_RES) height = BOARD_LCD_V_RES - y;

    // 如果宽度或高度为0，直接返回
    if (width <= 0 || height <= 0) {
        return ESP_OK;
    }

    // 分配临时缓冲区
    size_t buffer_size = width * height * sizeof(uint16_t);
    uint16_t *buffer = (uint16_t *)malloc(buffer_size);
    if (buffer == NULL) {
        return ESP_ERR_NO_MEM;
    }

    // 填充缓冲区
    for (int i = 0; i < width * height; i++) {
        buffer[i] = color;
    }

    // 调用draw_bitmap函数
    esp_err_t ret = esp_lcd_panel_draw_bitmap(_panel_handle, x, y, x + width, y + height, buffer);

    // 释放缓冲区
    free(buffer);

    return ret;
}

esp_err_t LCD_jd9853::set_backlight(float brightness) {
    if (_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // 参数范围限制
    if (brightness < 0.0) brightness = 0.0;
    if (brightness > 1.0) brightness = 1.0;

    // 控制背光
    if (BOARD_LCD_BL >= 0) {
        // 将亮度值转换为GPIO电平
        int level = brightness > 0.5 ? BOARD_LCD_BK_LIGHT_ON_LEVEL : BOARD_LCD_BK_LIGHT_OFF_LEVEL;
        gpio_set_level(BOARD_LCD_BL, level);
    }

    return ESP_OK;
}

esp_err_t LCD_jd9853::show_logo(void) {
    if (_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    #ifdef ARDUINO_MPYTHON_V3
        #include "logo_mpython_v3_320x172_lcd.h"
    #elif defined(ARDUINO_LABPLUS_LEDONG_V2)
        #include "logo_labplus_ledong_v2_320x172_lcd.h"
    #else // 默认使用mPython V3的logo
        #include "logo_mpython_v3_320x172_lcd.h"
    #endif

    uint16_t *pixels = (uint16_t *)heap_caps_malloc((BOARD_LCD_H_RES * BOARD_LCD_V_RES) * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (NULL == pixels)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return ESP_ERR_NO_MEM;
    }
    memcpy(pixels, logo_en_320x172_lcd, (BOARD_LCD_H_RES * BOARD_LCD_V_RES) * sizeof(uint16_t));
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, (uint16_t *)pixels);
    heap_caps_free(pixels);
    return ESP_OK;
}