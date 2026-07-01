#ifndef STACKTOWER_CORE_H
#define STACKTOWER_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STACKTOWER_LED_COUNT 5
#define STACKTOWER_FAST_FLASH_MS 100U
#define STACKTOWER_SLOW_FLASH_MS 500U
#define STACKTOWER_FADE_STEP_MS 50U
#define STACKTOWER_FADE_STEP_PERCENT 5U
#define STACKTOWER_STATE_JSON_MAX_LEN 512U
#define STACKTOWER_ERROR_JSON_MAX_LEN 192U

typedef enum {
    STACKTOWER_LED_RED = 0,
    STACKTOWER_LED_YELLOW,
    STACKTOWER_LED_GREEN,
    STACKTOWER_LED_BLUE,
    STACKTOWER_LED_DEBUG,
} stacktower_led_id_t;

typedef enum {
    STACKTOWER_MODE_OFF = 0,
    STACKTOWER_MODE_ON,
    STACKTOWER_MODE_FFLASH,
    STACKTOWER_MODE_SFLASH,
    STACKTOWER_MODE_FADE,
} stacktower_mode_t;

typedef struct {
    stacktower_mode_t mode;
    uint32_t phase_elapsed_ms;
    uint32_t cycle_count;
    uint8_t brightness_percent;
    int8_t fade_direction;
} stacktower_led_state_t;

typedef struct {
    stacktower_led_state_t leds[STACKTOWER_LED_COUNT];
} stacktower_controller_t;

typedef struct {
    bool applies_to_all;
    stacktower_led_id_t led;
    stacktower_mode_t mode;
} stacktower_command_t;

void stacktower_controller_init(stacktower_controller_t *controller);
void stacktower_controller_apply_command(stacktower_controller_t *controller, const stacktower_command_t *command);
void stacktower_controller_tick(stacktower_controller_t *controller, uint32_t elapsed_ms);
void stacktower_controller_copy_brightness(const stacktower_controller_t *controller, uint8_t brightness[STACKTOWER_LED_COUNT]);

bool stacktower_command_parse(const char *payload,
                              stacktower_command_t *command,
                              char *error_message,
                              size_t error_message_size);

bool stacktower_state_format_json(const stacktower_controller_t *controller,
                                  int64_t timestamp,
                                  char *buffer,
                                  size_t buffer_size);

bool stacktower_error_format_json(const char *message,
                                  int64_t timestamp,
                                  char *buffer,
                                  size_t buffer_size);

const char *stacktower_led_name(stacktower_led_id_t led);
const char *stacktower_mode_name(stacktower_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
