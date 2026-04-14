/*
 * Arduino-adapted default decoder header
 */
#pragma once

#include "audio/esp_audio_codec/include/decoder/impl/esp_aac_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_adpcm_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_alac_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_amrnb_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_amrwb_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_flac_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_g711_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_mp3_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_opus_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_vorbis_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_pcm_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_sbc_dec.h"
#include "audio/esp_audio_codec/include/decoder/impl/esp_lc3_dec.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_audio_err_t esp_audio_dec_register_default(void);
void esp_audio_dec_unregister_default(void);

#ifdef __cplusplus
}
#endif
