/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _AUDIO_CODEC_CTRL_IF_H_
#define _AUDIO_CODEC_CTRL_IF_H_

#include "audio/esp_codec_dev/include/esp_codec_dev_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_codec_ctrl_if_t audio_codec_ctrl_if_t;

struct audio_codec_ctrl_if_t {
    int (*open)(const audio_codec_ctrl_if_t *ctrl, void *cfg, int cfg_size);
    bool (*is_open)(const audio_codec_ctrl_if_t *ctrl);
    int (*read_reg)(const audio_codec_ctrl_if_t *ctrl, int reg, int reg_len, void *data, int data_len);
    int (*write_reg)(const audio_codec_ctrl_if_t *ctrl, int reg, int reg_len, void *data, int data_len);
    int (*close)(const audio_codec_ctrl_if_t *ctrl);
};

int audio_codec_delete_ctrl_if(const audio_codec_ctrl_if_t *ctrl_if);

#ifdef __cplusplus
}
#endif

#endif
