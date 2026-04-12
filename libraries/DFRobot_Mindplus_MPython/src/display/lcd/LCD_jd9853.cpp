#include "LCD_jd9853.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

// 日志标签
static const char *TAG = "LCD_jd9853";

// 构造函数
LCD_jd9853::LCD_jd9853() {
    _lcd_initialized = false;
    _spi_handle = NULL;
    _io_handle = NULL;
    _panel_handle = NULL;
}

// 初始化函数
esp_err_t LCD_jd9853::init() {
    // 检查是否已经初始化
    if (_lcd_initialized) {
        return ESP_OK;
    }
    
    // 初始化SPI总线
    spi_bus_config_t bus_conf = {
        .mosi_io_num = BOARD_LCD_MOSI,
        .miso_io_num = BOARD_LCD_MISO,
        .sclk_io_num = BOARD_LCD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 172 * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_conf, SPI_DMA_CH_AUTO));
    
    // 配置SPI设备
    spi_device_interface_config_t dev_conf = {
        .mode = 0,
        .clock_speed_hz = BOARD_LCD_PIXEL_CLOCK_HZ,
        .spics_io_num = BOARD_LCD_CS,
        .queue_size = 7,
        .pre_cb = NULL,
        .post_cb = NULL,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_conf, &_spi_handle));
    
    // 配置LCD面板IO
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BOARD_LCD_DC,
        .cs_gpio_num = BOARD_LCD_CS,
        .pclk_hz = BOARD_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = BOARD_LCD_CMD_BITS,
        .lcd_param_bits = BOARD_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(_spi_handle, &io_config, &_io_handle));
    
    // 创建LCD面板
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_LCD_RST,
        .width = BOARD_LCD_H_RES,
        .height = BOARD_LCD_V_RES,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9853(_io_handle, &panel_config, &_panel_handle));
    
    // 配置LCD面板
    ESP_ERROR_CHECK(esp_lcd_panel_reset(_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(_panel_handle, true));
    
    // 配置背光
    if (BOARD_LCD_BL >= 0) {
        gpio_config_t bl_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << BOARD_LCD_BL),
        };
        ESP_ERROR_CHECK(gpio_config(&bl_gpio_config));
        gpio_set_level(BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_ON_LEVEL);
    }
    
    // 对于Ledong V2板，需要复位屏幕
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
    
    // 设置初始化状态
    _lcd_initialized = true;
    
    return ESP_OK;
}

// 反初始化函数
esp_err_t LCD_jd9853::deinit() {
    // 检查是否已经初始化
    if (!_lcd_initialized) {
        return ESP_OK;
    }
    
    // 关闭背光
    if (BOARD_LCD_BL >= 0) {
        gpio_set_level(BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_OFF_LEVEL);
    }
    
    // 释放资源
    if (_panel_handle) {
        esp_lcd_panel_del(_panel_handle);
        _panel_handle = NULL;
    }
    if (_io_handle) {
        esp_lcd_panel_io_del(_io_handle);
        _io_handle = NULL;
    }
    if (_spi_handle) {
        spi_bus_remove_device(_spi_handle);
        _spi_handle = NULL;
    }
    spi_bus_free(SPI2_HOST);
    
    // 重置初始化状态
    _lcd_initialized = false;
    
    return ESP_OK;
}

// 绘制位图
esp_err_t LCD_jd9853::draw_bitmap(int x, int y, int width, int height, void *data) {
    // 检查是否已经初始化
    if (!_lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 参数范围限制
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    if (x >= BOARD_LCD_H_RES) x = BOARD_LCD_H_RES - 1;
    if (y >= BOARD_LCD_V_RES) y = BOARD_LCD_V_RES - 1;
    if (x + width > BOARD_LCD_H_RES) width = BOARD_LCD_H_RES - x;
    if (y + height > BOARD_LCD_V_RES) height = BOARD_LCD_V_RES - y;
    if (width == 0 || height == 0) {
        return ESP_OK;
    }
    
    // 绘制位图
    return esp_lcd_panel_draw_bitmap(_panel_handle, x, y, x + width, y + height, (uint16_t *)data);
}

// 填充区域
esp_err_t LCD_jd9853::fill(int x, int y, int width, int height, uint16_t color) {
    // 检查是否已经初始化
    if (!_lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 参数范围限制
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    if (x >= BOARD_LCD_H_RES) x = BOARD_LCD_H_RES - 1;
    if (y >= BOARD_LCD_V_RES) y = BOARD_LCD_V_RES - 1;
    if (x + width > BOARD_LCD_H_RES) width = BOARD_LCD_H_RES - x;
    if (y + height > BOARD_LCD_V_RES) height = BOARD_LCD_V_RES - y;
    if (width == 0 || height == 0) {
        return ESP_OK;
    }
    
    // 填充区域
    return esp_lcd_panel_fill(_panel_handle, x, y, x + width, y + height, color);
}

// 设置背光
esp_err_t LCD_jd9853::set_backlight(float brightness) {
    // 检查是否已经初始化
    if (!_lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 参数范围限制
    if (brightness < 0.0) brightness = 0.0;
    if (brightness > 1.0) brightness = 1.0;
    
    // 设置背光
    if (BOARD_LCD_BL >= 0) {
        if (brightness > 0.5) {
            gpio_set_level(BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_ON_LEVEL);
        } else {
            gpio_set_level(BOARD_LCD_BL, BOARD_LCD_BK_LIGHT_OFF_LEVEL);
        }
    }
    
    return ESP_OK;
}

// 显示logo
esp_err_t LCD_jd9853::show_logo() {
    // 检查是否已经初始化
    if (!_lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 根据板型选择logo
#ifdef ARDUINO_MPYTHON_V3
    // 这里应该引用mPython V3的logo
    // 暂时使用简单的颜色填充来模拟
    fill(0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, 0x001F); // 蓝色
#elif defined(ARDUINO_LABPLUS_LEDONG_V2)
    // 这里应该引用Ledong V2的logo
    // 暂时使用简单的颜色填充来模拟
    fill(0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, 0xF800); // 红色
#else
    // 默认使用mPython V3的logo
    fill(0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, 0x001F); // 蓝色
#endif
    
    return ESP_OK;
}