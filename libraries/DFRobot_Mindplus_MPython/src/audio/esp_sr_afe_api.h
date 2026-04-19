/**
 * Local esp-sr API aggregate.
 * Always include headers from src/audio/esp-sr to avoid toolchain version mismatch.
 */
#pragma once

#include "audio/esp-sr/include/esp32s3/esp_afe_config.h"
#include "audio/esp-sr/include/esp32s3/esp_afe_sr_iface.h"
#include "audio/esp-sr/include/esp32s3/esp_afe_sr_models.h"
#include "audio/esp-sr/include/esp32s3/esp_mn_iface.h"
#include "audio/esp-sr/include/esp32s3/esp_mn_models.h"
#include "audio/esp-sr/include/esp32s3/esp_wn_iface.h"
#include "audio/esp-sr/include/esp32s3/esp_wn_models.h"
#include "audio/esp-sr/src/include/model_path.h"
#include "audio/esp-sr/src/include/esp_mn_speech_commands.h"
#include "audio/esp-sr/src/include/esp_process_sdkconfig.h"
