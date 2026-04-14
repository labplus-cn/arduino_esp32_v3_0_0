/*
 * Arduino-adapted default simple decoder header
 */
#pragma once

#include "audio/esp_audio_codec/include/decoder/esp_audio_dec_default.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_wav_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_wav_parse.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_m4a_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_m4a_parse.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_ts_dec.h"
#include "audio/esp_audio_codec/include/simple_dec/impl/esp_ts_parse.h"
#include "audio/esp_audio_codec/include/simple_dec/esp_audio_simple_dec_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_audio_err_t esp_audio_simple_dec_register_default(void);
void esp_audio_simple_dec_unregister_default(void);

#ifdef __cplusplus
}
#endif
