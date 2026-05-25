#include "stacktower_hw.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

typedef struct {
    gpio_num_t gpio;
    ledc_channel_t channel;
} stacktower_led_hw_t;

static const stacktower_led_hw_t s_leds[STACKTOWER_LED_COUNT] = {
    { .gpio = CONFIG_STACKTOWER_GPIO_RED, .channel = LEDC_CHANNEL_0 },
    { .gpio = CONFIG_STACKTOWER_GPIO_YELLOW, .channel = LEDC_CHANNEL_1 },
    { .gpio = CONFIG_STACKTOWER_GPIO_GREEN, .channel = LEDC_CHANNEL_2 },
    { .gpio = CONFIG_STACKTOWER_GPIO_BLUE, .channel = LEDC_CHANNEL_3 },
    { .gpio = CONFIG_STACKTOWER_GPIO_DEBUG, .channel = LEDC_CHANNEL_4 },
};

static uint32_t stacktower_percent_to_duty(uint8_t percent)
{
    return ((uint32_t)percent * 255U) / 100U;
}

esp_err_t stacktower_hw_init(void)
{
    esp_err_t result;
    size_t index;
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    result = ledc_timer_config(&timer_config);
    if (result != ESP_OK) {
        return result;
    }

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        ledc_channel_config_t channel_config = {
            .gpio_num = s_leds[index].gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = s_leds[index].channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
            .flags.output_invert = 0,
        };

        result = ledc_channel_config(&channel_config);
        if (result != ESP_OK) {
            return result;
        }
    }

    return ESP_OK;
}

esp_err_t stacktower_hw_apply(const uint8_t brightness[STACKTOWER_LED_COUNT])
{
    esp_err_t result;
    size_t index;

    for (index = 0; index < STACKTOWER_LED_COUNT; ++index) {
        result = ledc_set_duty(LEDC_LOW_SPEED_MODE,
                               s_leds[index].channel,
                               stacktower_percent_to_duty(brightness[index]));
        if (result != ESP_OK) {
            return result;
        }

        result = ledc_update_duty(LEDC_LOW_SPEED_MODE, s_leds[index].channel);
        if (result != ESP_OK) {
            return result;
        }
    }

    return ESP_OK;
}
