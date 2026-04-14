/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ESP_CODEC_DEV_H_
#define _ESP_CODEC_DEV_H_

#include "audio/esp_codec_dev/interface/audio_codec_if.h"
#include "audio/esp_codec_dev/interface/audio_codec_data_if.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_types.h"
#include "audio/esp_codec_dev/include/esp_codec_dev_vol.h"
#include "audio/esp_codec_dev/interface/audio_codec_vol_if.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_codec_dev_type_t         dev_type;
    const audio_codec_if_t      *codec_if;
    const audio_codec_data_if_t *data_if;
} esp_codec_dev_cfg_t;

typedef void *esp_codec_dev_handle_t;

const char *esp_codec_dev_get_version(void);
esp_codec_dev_handle_t esp_codec_dev_new(esp_codec_dev_cfg_t *codec_dev_cfg);
int esp_codec_dev_open(esp_codec_dev_handle_t codec, esp_codec_dev_sample_info_t *fs);
int esp_codec_dev_read_reg(esp_codec_dev_handle_t codec, int reg, int *val);
int esp_codec_dev_write_reg(esp_codec_dev_handle_t codec, int reg, int val);
int esp_codec_dev_dump_reg(esp_codec_dev_handle_t codec);
int esp_codec_dev_read(esp_codec_dev_handle_t codec, void *data, int len);
int esp_codec_dev_write(esp_codec_dev_handle_t codec, void *data, int len);
int esp_codec_dev_set_out_vol(esp_codec_dev_handle_t codec, int volume);
int esp_codec_dev_set_vol_handler(esp_codec_dev_handle_t codec, const audio_codec_vol_if_t* vol_handler);
int esp_codec_dev_set_vol_curve(esp_codec_dev_handle_t codec, esp_codec_dev_vol_curve_t *curve);
int esp_codec_dev_get_out_vol(esp_codec_dev_handle_t codec, int *volume);
int esp_codec_dev_set_out_mute(esp_codec_dev_handle_t codec, bool mute);
int esp_codec_dev_get_out_mute(esp_codec_dev_handle_t codec, bool *muted);
int esp_codec_dev_set_in_gain(esp_codec_dev_handle_t codec, float db_value);
int esp_codec_dev_set_in_channel_gain(esp_codec_dev_handle_t codec, uint16_t channel_mask, float db_value);
int esp_codec_dev_get_in_gain(esp_codec_dev_handle_t codec, float *db_value);
int esp_codec_dev_set_in_mute(esp_codec_dev_handle_t codec, bool mute);
int esp_codec_dev_get_in_mute(esp_codec_dev_handle_t codec, bool *muted);
int esp_codec_set_disable_when_closed(esp_codec_dev_handle_t codec, bool disable);
int esp_codec_dev_close(esp_codec_dev_handle_t codec);
void esp_codec_dev_delete(esp_codec_dev_handle_t codec);

#ifdef __cplusplus
}
#endif

#endif
