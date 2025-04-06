#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_color_t;

void pwm_controller_init(void);
void pwm_timer_start(void);
void pwm_start_playback(void);
void pwm_set_rgb_color(uint8_t r, uint8_t g, uint8_t b);

void pwm_on_rgb(void);
void pwm_off_rgb(void);

#endif // PWM_CONTROL_H
