/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _AUDIO_CODEC_VOL_IF_H_
#define _AUDIO_CODEC_VOL_IF_H_

#include "audio/esp_codec_dev/include/esp_codec_dev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_codec_vol_if_t audio_codec_vol_if_t;

struct audio_codec_vol_if_t {
    int (*open)(const audio_codec_vol_if_t *h, esp_codec_dev_sample_info_t *fs, int fade_time);
    int (*set_vol)(const audio_codec_vol_if_t *h, float db_value);
    int (*process)(const audio_codec_vol_if_t *h, uint8_t *in, int len, uint8_t *out, int out_len);
    int (*close)(const audio_codec_vol_if_t *h);
};

int audio_codec_delete_vol_if(const audio_codec_vol_if_t *vol_if);

#ifdef __cplusplus
}
#endif

#endif
