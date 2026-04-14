/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ES8388_CODEC_H_
#define _ES8388_CODEC_H_

#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_ctrl_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_gpio_if.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_vol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ES8388_CODEC_DEFAULT_ADDR   (0x20)
#define ES8388_CODEC_DEFAULT_ADDR_1 (0x22)

typedef struct {
    const audio_codec_ctrl_if_t *ctrl_if;
    const audio_codec_gpio_if_t *gpio_if;
    esp_codec_dec_work_mode_t    codec_mode;
    bool                         master_mode;
    int16_t                      pa_pin;
    bool                         pa_reverted;
    esp_codec_dev_hw_gain_t      hw_gain;
} es8388_codec_cfg_t;

const audio_codec_if_t *es8388_codec_new(es8388_codec_cfg_t *codec_cfg);

#ifdef __cplusplus
}
#endif

#endif
