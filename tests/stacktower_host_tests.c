#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "stacktower_core.h"

static void test_parse_single_led_command(void)
{
    stacktower_command_t command;
    char error[64];

    assert(stacktower_command_parse("Red on", &command, error, sizeof(error)));
    assert(!command.applies_to_all);
    assert(command.led == STACKTOWER_LED_RED);
    assert(command.mode == STACKTOWER_MODE_ON);
}

static void test_parse_all_command_with_spacing(void)
{
    stacktower_command_t command;
    char error[64];

    assert(stacktower_command_parse("  all   fade  ", &command, error, sizeof(error)));
    assert(command.applies_to_all);
    assert(command.mode == STACKTOWER_MODE_FADE);
}

static void test_reject_invalid_command(void)
{
    stacktower_command_t command;
    char error[64];

    assert(!stacktower_command_parse("purple on", &command, error, sizeof(error)));
    assert(strstr(error, "unknown target") != NULL);
}

static void test_controller_defaults_to_off(void)
{
    stacktower_controller_t controller;
    uint8_t brightness[STACKTOWER_LED_COUNT];
    size_t index;

    stacktower_controller_init(&controller);
    stacktower_controller_copy_brightness(&controller, brightness);

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        assert(controller.leds[index].mode == STACKTOWER_MODE_OFF);
        assert(brightness[index] == 0);
    }
}

static void test_fast_flash_timing(void)
{
    stacktower_controller_t controller;
    stacktower_command_t command = {
        .applies_to_all = false,
        .led = STACKTOWER_LED_RED,
        .mode = STACKTOWER_MODE_FFLASH,
    };

    stacktower_controller_init(&controller);
    stacktower_controller_apply_command(&controller, &command);

    assert(controller.leds[STACKTOWER_LED_RED].brightness_percent == 100);

    stacktower_controller_tick(&controller, 100);
    assert(controller.leds[STACKTOWER_LED_RED].brightness_percent == 0);

    stacktower_controller_tick(&controller, 100);
    assert(controller.leds[STACKTOWER_LED_RED].brightness_percent == 100);
    assert(controller.leds[STACKTOWER_LED_RED].cycle_count == 1);
}

static void test_slow_flash_timing(void)
{
    stacktower_controller_t controller;
    stacktower_command_t command = {
        .applies_to_all = false,
        .led = STACKTOWER_LED_BLUE,
        .mode = STACKTOWER_MODE_SFLASH,
    };

    stacktower_controller_init(&controller);
    stacktower_controller_apply_command(&controller, &command);

    stacktower_controller_tick(&controller, 499);
    assert(controller.leds[STACKTOWER_LED_BLUE].brightness_percent == 100);

    stacktower_controller_tick(&controller, 1);
    assert(controller.leds[STACKTOWER_LED_BLUE].brightness_percent == 0);
}

static void test_fade_cycles(void)
{
    stacktower_controller_t controller;
    stacktower_command_t command = {
        .applies_to_all = false,
        .led = STACKTOWER_LED_GREEN,
        .mode = STACKTOWER_MODE_FADE,
    };
    int step;

    stacktower_controller_init(&controller);
    stacktower_controller_apply_command(&controller, &command);

    for (step = 0; step < 20; ++step) {
        stacktower_controller_tick(&controller, STACKTOWER_FADE_STEP_MS);
    }
    assert(controller.leds[STACKTOWER_LED_GREEN].brightness_percent == 100);

    for (step = 0; step < 20; ++step) {
        stacktower_controller_tick(&controller, STACKTOWER_FADE_STEP_MS);
    }
    assert(controller.leds[STACKTOWER_LED_GREEN].brightness_percent == 0);
    assert(controller.leds[STACKTOWER_LED_GREEN].cycle_count == 1);
}

static void test_state_json_contains_expected_fields(void)
{
    stacktower_controller_t controller;
    stacktower_command_t command = {
        .applies_to_all = true,
        .mode = STACKTOWER_MODE_ON,
    };
    char json[STACKTOWER_STATE_JSON_MAX_LEN];

    stacktower_controller_init(&controller);
    stacktower_controller_apply_command(&controller, &command);

    assert(stacktower_state_format_json(&controller, 12345, json, sizeof(json)));
    assert(strstr(json, "\"timestamp\":12345") != NULL);
    assert(strstr(json, "\"red\":{\"mode\":\"on\",\"brightness\":100") != NULL);
    assert(strstr(json, "\"debug\":{\"mode\":\"on\",\"brightness\":100") != NULL);
}

int main(void)
{
    test_parse_single_led_command();
    test_parse_all_command_with_spacing();
    test_reject_invalid_command();
    test_controller_defaults_to_off();
    test_fast_flash_timing();
    test_slow_flash_timing();
    test_fade_cycles();
    test_state_json_contains_expected_fields();

    printf("stacktower host tests passed\n");
    return 0;
}
