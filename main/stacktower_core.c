#include "stacktower_core.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define STACKTOWER_PARSE_BUFFER_LEN 64U

static const char *const s_led_names[STACKTOWER_LED_COUNT] = {
    "red",
    "yellow",
    "green",
    "blue",
    "debug",
};

static const char *const s_mode_names[] = {
    "off",
    "on",
    "fflash",
    "sflash",
    "fade",
};

static void stacktower_channel_reset(stacktower_led_state_t *channel, stacktower_mode_t mode)
{
    channel->mode = mode;
    channel->phase_elapsed_ms = 0;
    channel->cycle_count = 0;
    channel->fade_direction = 1;

    switch (mode) {
    case STACKTOWER_MODE_OFF:
        channel->brightness_percent = 0;
        break;
    case STACKTOWER_MODE_ON:
    case STACKTOWER_MODE_FFLASH:
    case STACKTOWER_MODE_SFLASH:
        channel->brightness_percent = 100;
        break;
    case STACKTOWER_MODE_FADE:
        channel->brightness_percent = 0;
        break;
    }
}

static stacktower_led_id_t stacktower_led_from_name(const char *name, bool *is_all, bool *found)
{
    size_t index;

    *is_all = false;
    *found = false;

    if (strcmp(name, "all") == 0) {
        *is_all = true;
        *found = true;
        return STACKTOWER_LED_RED;
    }

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        if (strcmp(name, s_led_names[index]) == 0) {
            *found = true;
            return (stacktower_led_id_t)index;
        }
    }

    return STACKTOWER_LED_RED;
}

static bool stacktower_mode_from_name(const char *name, stacktower_mode_t *mode)
{
    size_t index;

    for (index = 0; index < sizeof(s_mode_names) / sizeof(s_mode_names[0]); ++index) {
        if (strcmp(name, s_mode_names[index]) == 0) {
            *mode = (stacktower_mode_t)index;
            return true;
        }
    }

    return false;
}

static void stacktower_set_error(char *error_message, size_t error_message_size, const char *message)
{
    if ((error_message == NULL) || (error_message_size == 0)) {
        return;
    }

    snprintf(error_message, error_message_size, "%s", message);
}

static size_t stacktower_normalize_payload(const char *payload, char *normalized, size_t normalized_size)
{
    size_t write_index = 0;
    bool in_space = true;

    while ((*payload != '\0') && isspace((unsigned char)*payload)) {
        ++payload;
    }

    while ((*payload != '\0') && (write_index + 1 < normalized_size)) {
        unsigned char current = (unsigned char)*payload++;

        if (isspace(current)) {
            if (!in_space) {
                normalized[write_index++] = ' ';
                in_space = true;
            }
            continue;
        }

        normalized[write_index++] = (char)tolower(current);
        in_space = false;
    }

    if ((write_index > 0) && (normalized[write_index - 1] == ' ')) {
        --write_index;
    }

    normalized[write_index] = '\0';
    return write_index;
}

void stacktower_controller_init(stacktower_controller_t *controller)
{
    size_t index;

    if (controller == NULL) {
        return;
    }

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        stacktower_channel_reset(&controller->leds[index], STACKTOWER_MODE_OFF);
    }
}

void stacktower_controller_apply_command(stacktower_controller_t *controller, const stacktower_command_t *command)
{
    size_t index;

    if ((controller == NULL) || (command == NULL)) {
        return;
    }

    if (command->applies_to_all) {
        for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
            stacktower_channel_reset(&controller->leds[index], command->mode);
        }
        return;
    }

    stacktower_channel_reset(&controller->leds[command->led], command->mode);
}

void stacktower_controller_tick(stacktower_controller_t *controller, uint32_t elapsed_ms)
{
    size_t index;

    if (controller == NULL) {
        return;
    }

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        stacktower_led_state_t *channel = &controller->leds[index];
        uint32_t interval_ms;

        switch (channel->mode) {
        case STACKTOWER_MODE_OFF:
            channel->brightness_percent = 0;
            break;

        case STACKTOWER_MODE_ON:
            channel->brightness_percent = 100;
            break;

        case STACKTOWER_MODE_FFLASH:
        case STACKTOWER_MODE_SFLASH:
            interval_ms = (channel->mode == STACKTOWER_MODE_FFLASH) ? STACKTOWER_FAST_FLASH_MS : STACKTOWER_SLOW_FLASH_MS;
            channel->phase_elapsed_ms += elapsed_ms;
            channel->brightness_percent = ((channel->phase_elapsed_ms / interval_ms) % 2U == 0U) ? 100U : 0U;
            channel->cycle_count = channel->phase_elapsed_ms / (interval_ms * 2U);
            break;

        case STACKTOWER_MODE_FADE:
            channel->phase_elapsed_ms += elapsed_ms;

            while (channel->phase_elapsed_ms >= STACKTOWER_FADE_STEP_MS) {
                int next_brightness;

                channel->phase_elapsed_ms -= STACKTOWER_FADE_STEP_MS;
                next_brightness = (int)channel->brightness_percent + ((int)channel->fade_direction * (int)STACKTOWER_FADE_STEP_PERCENT);

                if (next_brightness >= 100) {
                    channel->brightness_percent = 100;
                    channel->fade_direction = -1;
                    continue;
                }

                if (next_brightness <= 0) {
                    if (channel->brightness_percent != 0) {
                        ++channel->cycle_count;
                    }
                    channel->brightness_percent = 0;
                    channel->fade_direction = 1;
                    continue;
                }

                channel->brightness_percent = (uint8_t)next_brightness;
            }
            break;
        }
    }
}

void stacktower_controller_copy_brightness(const stacktower_controller_t *controller,
                                           uint8_t brightness[STACKTOWER_LED_COUNT])
{
    size_t index;

    if ((controller == NULL) || (brightness == NULL)) {
        return;
    }

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        brightness[index] = controller->leds[index].brightness_percent;
    }
}

bool stacktower_command_parse(const char *payload,
                              stacktower_command_t *command,
                              char *error_message,
                              size_t error_message_size)
{
    char normalized[STACKTOWER_PARSE_BUFFER_LEN];
    char *separator;
    char *target_token;
    char *mode_token;
    bool is_all;
    bool found_led;
    stacktower_mode_t parsed_mode;

    if ((payload == NULL) || (command == NULL)) {
        stacktower_set_error(error_message, error_message_size, "command payload is required");
        return false;
    }

    if (stacktower_normalize_payload(payload, normalized, sizeof(normalized)) == 0U) {
        stacktower_set_error(error_message, error_message_size, "command payload is empty");
        return false;
    }

    separator = strchr(normalized, ' ');
    if (separator == NULL) {
        stacktower_set_error(error_message, error_message_size, "command must be '<target> <mode>'");
        return false;
    }

    *separator = '\0';
    target_token = normalized;
    mode_token = separator + 1;

    if (strchr(mode_token, ' ') != NULL) {
        stacktower_set_error(error_message, error_message_size, "command must contain exactly two words");
        return false;
    }

    command->led = stacktower_led_from_name(target_token, &is_all, &found_led);
    if (!found_led) {
        stacktower_set_error(error_message, error_message_size, "unknown target");
        return false;
    }

    if (!stacktower_mode_from_name(mode_token, &parsed_mode)) {
        stacktower_set_error(error_message, error_message_size, "unknown mode");
        return false;
    }

    command->applies_to_all = is_all;
    command->mode = parsed_mode;
    error_message[0] = '\0';
    return true;
}

bool stacktower_state_format_json(const stacktower_controller_t *controller,
                                  int64_t timestamp,
                                  char *buffer,
                                  size_t buffer_size)
{
    int written;

    if ((controller == NULL) || (buffer == NULL) || (buffer_size == 0U)) {
        return false;
    }

    written = snprintf(buffer,
                       buffer_size,
                       "{\"status\":\"ok\",\"timestamp\":%lld,"
                       "\"red\":{\"mode\":\"%s\",\"brightness\":%u,\"cycle_count\":%" PRIu32 "},"
                       "\"yellow\":{\"mode\":\"%s\",\"brightness\":%u,\"cycle_count\":%" PRIu32 "},"
                       "\"green\":{\"mode\":\"%s\",\"brightness\":%u,\"cycle_count\":%" PRIu32 "},"
                       "\"blue\":{\"mode\":\"%s\",\"brightness\":%u,\"cycle_count\":%" PRIu32 "},"
                       "\"debug\":{\"mode\":\"%s\",\"brightness\":%u,\"cycle_count\":%" PRIu32 "}}",
                       (long long)timestamp,
                       stacktower_mode_name(controller->leds[STACKTOWER_LED_RED].mode),
                       controller->leds[STACKTOWER_LED_RED].brightness_percent,
                       controller->leds[STACKTOWER_LED_RED].cycle_count,
                       stacktower_mode_name(controller->leds[STACKTOWER_LED_YELLOW].mode),
                       controller->leds[STACKTOWER_LED_YELLOW].brightness_percent,
                       controller->leds[STACKTOWER_LED_YELLOW].cycle_count,
                       stacktower_mode_name(controller->leds[STACKTOWER_LED_GREEN].mode),
                       controller->leds[STACKTOWER_LED_GREEN].brightness_percent,
                       controller->leds[STACKTOWER_LED_GREEN].cycle_count,
                       stacktower_mode_name(controller->leds[STACKTOWER_LED_BLUE].mode),
                       controller->leds[STACKTOWER_LED_BLUE].brightness_percent,
                       controller->leds[STACKTOWER_LED_BLUE].cycle_count,
                       stacktower_mode_name(controller->leds[STACKTOWER_LED_DEBUG].mode),
                       controller->leds[STACKTOWER_LED_DEBUG].brightness_percent,
                       controller->leds[STACKTOWER_LED_DEBUG].cycle_count);

    return (written > 0) && ((size_t)written < buffer_size);
}

bool stacktower_error_format_json(const char *message,
                                  int64_t timestamp,
                                  char *buffer,
                                  size_t buffer_size)
{
    int written;

    if ((message == NULL) || (buffer == NULL) || (buffer_size == 0U)) {
        return false;
    }

    written = snprintf(buffer,
                       buffer_size,
                       "{\"status\":\"error\",\"timestamp\":%lld,\"message\":\"%s\"}",
                       (long long)timestamp,
                       message);

    return (written > 0) && ((size_t)written < buffer_size);
}

const char *stacktower_led_name(stacktower_led_id_t led)
{
    if ((int)led < 0 || led >= STACKTOWER_LED_COUNT) {
        return "unknown";
    }

    return s_led_names[led];
}

const char *stacktower_mode_name(stacktower_mode_t mode)
{
    if ((int)mode < 0 || mode > STACKTOWER_MODE_FADE) {
        return "unknown";
    }

    return s_mode_names[mode];
}
