/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _AUDIO_CODEC_IF_H_
#define _AUDIO_CODEC_IF_H_

#include "audio/esp_codec_dev/include/esp_codec_dev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_codec_if_t audio_codec_if_t;

struct audio_codec_if_t {
    int (*open)(const audio_codec_if_t *h, void *cfg, int cfg_size);
    bool (*is_open)(const audio_codec_if_t *h);
    int (*enable)(const audio_codec_if_t *h, bool enable);
    int (*set_fs)(const audio_codec_if_t *h, esp_codec_dev_sample_info_t *fs);
    int (*mute)(const audio_codec_if_t *h, bool mute);
    int (*set_vol)(const audio_codec_if_t *h, float db);
    int (*set_mic_gain)(const audio_codec_if_t *h, float db);
    int (*set_mic_channel_gain)(const audio_codec_if_t *h, uint16_t channel_mask, float db);
    int (*mute_mic)(const audio_codec_if_t *h, bool mute);
    int (*set_reg)(const audio_codec_if_t *h, int reg, int value);
    int (*get_reg)(const audio_codec_if_t *h, int reg, int *value);
    void (*dump_reg)(const audio_codec_if_t *h);
    int (*close)(const audio_codec_if_t *h);
};

int audio_codec_delete_codec_if(const audio_codec_if_t *codec_if);

#ifdef __cplusplus
}
#endif

#endif
