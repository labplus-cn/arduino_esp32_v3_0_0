#pragma once

#include <esp32-hal.h>
#include <esp_afe_config.h>
#include <esp_afe_sr_iface.h>
#include <esp_afe_sr_models.h>
#include <esp_mn_iface.h>
#include <esp_mn_models.h>
#include <esp_wn_iface.h>
#include <esp_wn_models.h>
#include <model_path.h>
#include <esp_mn_speech_commands.h>
#include <esp_process_sdkconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

afe_config_t *afe_config_init(const char *input_format, srmodel_list_t *models);
void afe_config_free(afe_config_t *cfg);
esp_afe_sr_iface_t *esp_afe_handle_from_config(const afe_config_t *cfg);

#ifdef __cplusplus
}
#endif
