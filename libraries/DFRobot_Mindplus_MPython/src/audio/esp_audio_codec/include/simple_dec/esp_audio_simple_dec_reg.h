/*
 * Arduino-adapted simple decoder registry header
 */
#pragma once

#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_es_parse_types.h"
#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_audio_dec_ops_t      decoder_ops;
    esp_es_parse_func_t      parser;
    esp_es_parse_free_func_t free;
} esp_audio_simple_dec_reg_info_t;

esp_audio_err_t esp_audio_simple_dec_register(esp_audio_simple_dec_type_t dec_type, esp_audio_simple_dec_reg_info_t *reg_info);
esp_audio_err_t esp_audio_simple_dec_unregister(esp_audio_simple_dec_type_t dec_type);
void esp_audio_simple_dec_unregister_all(void);

#ifdef __cplusplus
}
#endif
