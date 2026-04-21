#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "audio/esp_sr_afe_api.h"

// When packaged esp-sr archives are not linked in Arduino build,
// provide weak fallback symbols to keep Audio SR path linkable.
// If strong symbols exist in linked archives, they override these.

extern const esp_afe_sr_iface_t esp_afe_sr_v1;

// Legacy AFE config layout used by the ESP32 Arduino packaged AFE binary.
// Keep field order aligned with the old esp-sr v1.x header.
typedef struct {
  int total_ch_num;
  int mic_num;
  int ref_num;
  int sample_rate;
} legacy_afe_pcm_config_t;

typedef struct {
  bool aec_init;
  bool se_init;
  bool vad_init;
  bool wakenet_init;
  bool voice_communication_init;
  bool voice_communication_agc_init;
  int voice_communication_agc_gain;
  vad_mode_t vad_mode;
  char *wakenet_model_name;
  char *wakenet_model_name_2;
  det_mode_t wakenet_mode;
  int afe_mode;
  int afe_perferred_core;
  int afe_perferred_priority;
  int afe_ringbuf_size;
  int memory_alloc_mode;
  float afe_linear_gain;
  int agc_mode;
  legacy_afe_pcm_config_t pcm_config;
  bool debug_init;
  afe_debug_hook_t debug_hook[AFE_DEBUG_HOOK_MAX];
  int afe_ns_mode;
  char *afe_ns_model_name;
} legacy_afe_config_t;

__attribute__((weak)) afe_config_t *afe_config_init(const char *input_format,
                                                    srmodel_list_t *models) {
  (void)input_format;
  legacy_afe_config_t *cfg =
      (legacy_afe_config_t *)calloc(1, sizeof(legacy_afe_config_t));
  if (!cfg) return NULL;

  cfg->afe_mode = SR_MODE_HIGH_PERF;
  cfg->afe_ringbuf_size = 50;
  cfg->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
  cfg->afe_linear_gain = 1.0f;

  cfg->aec_init = false;
  cfg->se_init = true;
  cfg->vad_init = true;
  cfg->wakenet_init = true;
  cfg->voice_communication_init = false;
  cfg->voice_communication_agc_init = false;
  cfg->voice_communication_agc_gain = 15;
  cfg->vad_mode = VAD_MODE_3;
  cfg->wakenet_mode = DET_MODE_90;
  cfg->agc_mode = AFE_MN_PEAK_NO_AGC;
  cfg->wakenet_model_name = models ? esp_srmodel_filter(models, ESP_WN_PREFIX, NULL) : NULL;
  if (!cfg->wakenet_model_name) {
    cfg->wakenet_init = false;
  }

  cfg->pcm_config.total_ch_num = 1;
  cfg->pcm_config.mic_num = 1;
  cfg->pcm_config.ref_num = 0;
  cfg->pcm_config.sample_rate = 16000;
  return (afe_config_t *)cfg;
}

__attribute__((weak)) void afe_config_free(afe_config_t *cfg) {
  free(cfg);
}

__attribute__((weak)) esp_afe_sr_iface_t *esp_afe_handle_from_config(const afe_config_t *cfg) {
  (void)cfg;
  return (esp_afe_sr_iface_t *)&esp_afe_sr_v1;
}
