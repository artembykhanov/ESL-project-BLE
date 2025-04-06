#include "pwm_control.h"

#include "nrfx_pwm.h"
#include "nrfx_gpiote.h"
#include <stdlib.h>

#include "app_timer.h"

#include "app_error.h"
#include "nrf_log.h"

#define PWM_TOP_VALUE 255

#define LED_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_B_PIN NRF_GPIO_PIN_MAP(0, 12)

#define RGB_CHANNEL_R 1
#define RGB_CHANNEL_G 2
#define RGB_CHANNEL_B 3

rgb_color_t rgb_current_color = {0, 0, 0};
bool rgb_enabled = false;

static nrfx_pwm_t rgb_instance = NRFX_PWM_INSTANCE(0);

static nrf_pwm_values_individual_t pwm_duty_cycles;
static nrf_pwm_sequence_t const pwm_sequence =
    {
        .values.p_individual = &pwm_duty_cycles,
        .length = NRF_PWM_VALUES_LENGTH(pwm_duty_cycles),
        .repeats = 0,
        .end_delay = 0};

void pwm_controller_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[1] = LED_R_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[2] = LED_G_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[3] = LED_B_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.top_value = PWM_TOP_VALUE;

    nrfx_pwm_init(&rgb_instance, &pwm_config, NULL);

    pwm_duty_cycles.channel_1 = rgb_current_color.red;
    pwm_duty_cycles.channel_2 = rgb_current_color.green;
    pwm_duty_cycles.channel_3 = rgb_current_color.blue;
}

void pwm_start_playback(void)
{
    nrfx_pwm_simple_playback(&rgb_instance, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}

static void pwm_update_duty_cycle(uint8_t channel)
{
    if (!rgb_enabled && channel != 0)
    {
        return;
    }

    switch (channel)
    {
    case 1:
        pwm_duty_cycles.channel_1 = rgb_enabled ? rgb_current_color.red : 0;
        break;
    case 2:
        pwm_duty_cycles.channel_2 = rgb_enabled ? rgb_current_color.green : 0;
        break;
    case 3:
        pwm_duty_cycles.channel_3 = rgb_enabled ? rgb_current_color.blue : 0;
        break;
    }
}

void pwm_set_rgb_color(uint8_t r, uint8_t g, uint8_t b)
{
    NRF_LOG_INFO("PWM CONTROL: Setting RGB color: R=%d, G=%d, B=%d", r, g, b);

    rgb_current_color.red = r;
    rgb_current_color.green = g;
    rgb_current_color.blue = b;

    if (rgb_enabled)
    {
        pwm_update_duty_cycle(RGB_CHANNEL_R);
        pwm_update_duty_cycle(RGB_CHANNEL_G);
        pwm_update_duty_cycle(RGB_CHANNEL_B);
    }
}

void pwm_on_rgb(void)
{
    NRF_LOG_INFO("PWM CONTROL: RGB ON: R=%d G=%d B=%d", rgb_current_color.red, rgb_current_color.green, rgb_current_color.blue);
    rgb_enabled = true;

    pwm_update_duty_cycle(RGB_CHANNEL_R);
    pwm_update_duty_cycle(RGB_CHANNEL_G);
    pwm_update_duty_cycle(RGB_CHANNEL_B);
}

void pwm_off_rgb(void)
{
    NRF_LOG_INFO("PWM CONTROL: RGB OFF (saved: R=%d G=%d B=%d)", rgb_current_color.red, rgb_current_color.green, rgb_current_color.blue);
    rgb_enabled = false;

    pwm_duty_cycles.channel_1 = 0;
    pwm_duty_cycles.channel_2 = 0;
    pwm_duty_cycles.channel_3 = 0;
}