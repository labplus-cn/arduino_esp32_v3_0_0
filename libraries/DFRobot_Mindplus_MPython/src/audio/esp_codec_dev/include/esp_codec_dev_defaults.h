/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ESP_CODEC_DEV_DEFAULTS_H_
#define _ESP_CODEC_DEV_DEFAULTS_H_

#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"

#ifdef CONFIG_CODEC_ES8311_SUPPORT
#include "audio/esp_codec_dev/device/include/es8311_codec.h"
#endif
#ifdef CONFIG_CODEC_ES7210_SUPPORT
#include "audio/esp_codec_dev/device/include/es7210_adc.h"
#endif
#ifdef CONFIG_CODEC_ES7243_SUPPORT
#include "audio/esp_codec_dev/device/include/es7243_adc.h"
#endif
#ifdef CONFIG_CODEC_ES7243E_SUPPORT
#include "audio/esp_codec_dev/device/include/es7243e_adc.h"
#endif
#ifdef CONFIG_CODEC_ES8156_SUPPORT
#include "audio/esp_codec_dev/device/include/es8156_dac.h"
#endif
#ifdef CONFIG_CODEC_AW88298_SUPPORT
#include "audio/esp_codec_dev/device/include/aw88298_dac.h"
#endif
#ifdef CONFIG_CODEC_ES8389_SUPPORT
#include "audio/esp_codec_dev/device/include/es8389_codec.h"
#endif
#ifdef CONFIG_CODEC_ES8374_SUPPORT
#include "audio/esp_codec_dev/device/include/es8374_codec.h"
#endif
#ifdef CONFIG_CODEC_ES8388_SUPPORT
#include "audio/esp_codec_dev/device/include/es8388_codec.h"
#endif
#ifdef CONFIG_CODEC_TAS5805M_SUPPORT
#include "audio/esp_codec_dev/device/include/tas5805m_dac.h"
#endif
#ifdef CONFIG_CODEC_ZL38063_SUPPORT
#include "audio/esp_codec_dev/device/include/zl38063_codec.h"
#endif
#if CONFIG_CODEC_CJC8910_SUPPORT
#include "audio/esp_codec_dev/device/include/cjc8910_codec.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t port;
    uint8_t addr;
    void   *bus_handle;
} audio_codec_i2c_cfg_t;

typedef struct {
    uint8_t port;
    void   *rx_handle;
    void   *tx_handle;
    int     clk_src;
} audio_codec_i2s_cfg_t;

typedef struct {
    uint8_t spi_port;
    int16_t cs_pin;
    int     clock_speed;
} audio_codec_spi_cfg_t;

const audio_codec_gpio_if_t *audio_codec_new_gpio(void);
const audio_codec_ctrl_if_t *audio_codec_new_spi_ctrl(audio_codec_spi_cfg_t *spi_cfg);
const audio_codec_ctrl_if_t *audio_codec_new_i2c_ctrl(audio_codec_i2c_cfg_t *i2c_cfg);
const audio_codec_data_if_t *audio_codec_new_i2s_data(audio_codec_i2s_cfg_t *i2s_cfg);

#ifdef __cplusplus
}
#endif

#endif
