/*
 * Arduino-adapted audio decoder API header
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "audio/esp_audio_codec/include/esp_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_AUDIO_DEC_RECOVERY_NONE = 0,
    ESP_AUDIO_DEC_RECOVERY_PLC  = 1,
} esp_audio_dec_recovery_t;

typedef struct {
    uint8_t                 *buffer;
    uint32_t                 len;
    uint32_t                 consumed;
    esp_audio_dec_recovery_t frame_recover;
} esp_audio_dec_in_raw_t;

typedef struct {
    uint8_t  *buffer;
    uint32_t  len;
    uint32_t  needed_size;
    uint32_t  decoded_size;
} esp_audio_dec_out_frame_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t  channel;
    uint8_t  bits_per_sample;
} esp_audio_dec_info_t;

typedef void *esp_audio_dec_handle_t;

esp_audio_err_t esp_audio_dec_open(esp_audio_type_t dec_type, void *cfg, int cfg_sz, esp_audio_dec_handle_t *dec_handle);
esp_audio_err_t esp_audio_dec_process(esp_audio_dec_handle_t dec_handle, esp_audio_dec_in_raw_t *raw, esp_audio_dec_out_frame_t *frame, esp_audio_dec_info_t *dec_info);
esp_audio_err_t esp_audio_dec_close(esp_audio_dec_handle_t dec_handle);

#ifdef __cplusplus
}
#endif
