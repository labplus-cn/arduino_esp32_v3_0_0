#pragma once

// Minimal defaults for Arduino build without ESP-IDF menuconfig.
// Keep only formats used by this library implementation.

#define CONFIG_AUDIO_DECODER_MP3_SUPPORT 1
#define CONFIG_AUDIO_DECODER_AAC_SUPPORT 1
#define CONFIG_AUDIO_DECODER_FLAC_SUPPORT 1
#define CONFIG_AUDIO_DECODER_PCM_SUPPORT 1

#define CONFIG_AUDIO_SIMPLE_DEC_WAV_SUPPORT 1
#define CONFIG_AUDIO_SIMPLE_DEC_M4A_SUPPORT 1
