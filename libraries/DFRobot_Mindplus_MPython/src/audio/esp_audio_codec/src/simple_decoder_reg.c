/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

#include "audio/esp_audio_codec/src/esp_audio_simple_dec_default.h"

// Arduino 环境：预编译库包含所有 simple 解码器，无条件注册
esp_audio_err_t esp_audio_simple_dec_register_default(void)
{
    esp_audio_err_t ret = ESP_AUDIO_ERR_OK;
    ret |= esp_wav_dec_register();
    ret |= esp_m4a_dec_register();
    ret |= esp_ts_dec_register();
    return ret;
}

void esp_audio_simple_dec_unregister_default(void)
{
    esp_wav_dec_unregister();
    esp_m4a_dec_unregister();
    esp_ts_dec_unregister();
}
