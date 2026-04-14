/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _AUDIO_CODEC_DATA_IF_H_
#define _AUDIO_CODEC_DATA_IF_H_

#include "audio/esp_codec_dev/include/esp_codec_dev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_codec_data_if_t audio_codec_data_if_t;

struct audio_codec_data_if_t {
    int (*open)(const audio_codec_data_if_t *h, void *data_cfg, int cfg_size);
    bool (*is_open)(const audio_codec_data_if_t *h);
    int (*enable)(const audio_codec_data_if_t *h, esp_codec_dev_type_t dev_type, bool enable);
    int (*set_fmt)(const audio_codec_data_if_t *h, esp_codec_dev_type_t dev_type, esp_codec_dev_sample_info_t *fs);
    int (*read)(const audio_codec_data_if_t *h, uint8_t *data, int size);
    int (*write)(const audio_codec_data_if_t *h, uint8_t *data, int size);
    int (*close)(const audio_codec_data_if_t *h);
};

int audio_codec_delete_data_if(const audio_codec_data_if_t *data_if);

#ifdef __cplusplus
}
#endif

#endif
