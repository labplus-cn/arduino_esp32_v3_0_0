/*
 * Arduino-adapted simple decoder API header
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "audio/esp_audio_codec/include/esp_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *esp_audio_simple_dec_handle_t;

typedef enum {
    ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE = 0,
    ESP_AUDIO_SIMPLE_DEC_RECOVERY_PLC  = 1,
} esp_audio_simple_dec_recovery_t;

typedef enum {
    ESP_AUDIO_SIMPLE_DEC_TYPE_NONE       = 0,
    ESP_AUDIO_SIMPLE_DEC_TYPE_AAC        = ESP_AUDIO_TYPE_AAC,
    ESP_AUDIO_SIMPLE_DEC_TYPE_MP3        = ESP_AUDIO_TYPE_MP3,
    ESP_AUDIO_SIMPLE_DEC_TYPE_AMRNB      = ESP_AUDIO_TYPE_AMRNB,
    ESP_AUDIO_SIMPLE_DEC_TYPE_AMRWB      = ESP_AUDIO_TYPE_AMRWB,
    ESP_AUDIO_SIMPLE_DEC_TYPE_FLAC       = ESP_AUDIO_TYPE_FLAC,
    ESP_AUDIO_SIMPLE_DEC_TYPE_WAV        = ESP_AUDIO_FOURCC_TO_INT('W', 'A', 'V', ' '),
    ESP_AUDIO_SIMPLE_DEC_TYPE_M4A        = ESP_AUDIO_FOURCC_TO_INT('M', '4', 'A', 'A'),
    ESP_AUDIO_SIMPLE_DEC_TYPE_TS         = ESP_AUDIO_FOURCC_TO_INT('M', '2', 'T', 'S'),
    ESP_AUDIO_SIMPLE_DEC_TYPE_RAW_OPUS   = ESP_AUDIO_TYPE_OPUS,
    ESP_AUDIO_SIMPLE_DEC_TYPE_G711A      = ESP_AUDIO_TYPE_G711A,
    ESP_AUDIO_SIMPLE_DEC_TYPE_G711U      = ESP_AUDIO_TYPE_G711U,
    ESP_AUDIO_SIMPLE_DEC_TYPE_PCM        = ESP_AUDIO_TYPE_PCM,
    ESP_AUDIO_SIMPLE_DEC_TYPE_ADPCM      = ESP_AUDIO_TYPE_ADPCM,
    ESP_AUDIO_SIMPLE_DEC_TYPE_SBC        = ESP_AUDIO_TYPE_SBC,
    ESP_AUDIO_SIMPLE_DEC_TYPE_LC3        = ESP_AUDIO_TYPE_LC3,
    ESP_AUDIO_SIMPLE_DEC_TYPE_ALAC       = ESP_AUDIO_TYPE_ALAC,
    ESP_AUDIO_SIMPLE_DEC_TYPE_CUSTOM     = 0x1,
    ESP_AUDIO_SIMPLE_DEC_TYPE_CUSTOM_MAX = 0x10,
} esp_audio_simple_dec_type_t;

typedef struct {
    esp_audio_simple_dec_type_t dec_type;
    void                       *dec_cfg;
    int                         cfg_size;
} esp_audio_simple_dec_cfg_t;

typedef struct {
    uint8_t                        *buffer;
    uint32_t                        len;
    bool                            eos;
    uint32_t                        consumed;
    esp_audio_simple_dec_recovery_t frame_recover;
} esp_audio_simple_dec_raw_t;

typedef struct {
    uint8_t *buffer;
    uint32_t len;
    uint32_t needed_size;
    uint32_t decoded_size;
} esp_audio_simple_dec_out_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t  channel;
    uint8_t  bits_per_sample;
} esp_audio_simple_dec_info_t;

esp_audio_err_t esp_audio_simple_dec_open(esp_audio_simple_dec_cfg_t *cfg, esp_audio_simple_dec_handle_t *handle);
esp_audio_err_t esp_audio_simple_dec_process(esp_audio_simple_dec_handle_t handle, esp_audio_simple_dec_raw_t *raw, esp_audio_simple_dec_out_t *out);
esp_audio_err_t esp_audio_simple_dec_get_info(esp_audio_simple_dec_handle_t handle, esp_audio_simple_dec_info_t *info);
esp_audio_err_t esp_audio_simple_dec_close(esp_audio_simple_dec_handle_t handle);

#ifdef __cplusplus
}
#endif
