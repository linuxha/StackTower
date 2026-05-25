#ifndef STACKTOWER_HW_H
#define STACKTOWER_HW_H

#include <stdint.h>

#include "esp_err.h"
#include "stacktower_core.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t stacktower_hw_init(void);
esp_err_t stacktower_hw_apply(const uint8_t brightness[STACKTOWER_LED_COUNT]);

#ifdef __cplusplus
}
#endif

#endif
