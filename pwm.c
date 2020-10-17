#include "pwm.h"
#include <string.h>
#include "main.h"

#define PWM_PHASES_T0       8
#define PWM_PHASES_T2       16

typedef struct {
    volatile uint8_t t0_index;
    volatile uint8_t t2_index;
    volatile uint8_t t0a_values[PWM_PHASES_T0];
    volatile uint8_t t2a_values[PWM_PHASES_T2];
    volatile uint8_t t2b_values[PWM_PHASES_T2];
} PwmState;

static PwmState pwm_state;

void pwm_init(void)
{
    memset((void*)pwm_state.t0a_values, 0xff, sizeof(pwm_state.t0a_values));
    memset((void*)pwm_state.t2a_values, 0xff, sizeof(pwm_state.t2a_values));
    memset((void*)pwm_state.t2b_values, 0xff, sizeof(pwm_state.t2b_values));

    TCCR2A = 0xf3; // Fast PWM for A and B, inverting mode
    TCCR2B = 0x03; // clk/32
    OCR2A  = 0xff;
    OCR2B  = 0xff;

    _delay_us(200);

    TCCR0A = 0xc3; // Fast PWM for A, inverting mode
    TCCR0B = 0x03; // clk/64
    OCR0A  = 0xff;

    DDRB |= 0x08;
    DDRD |= 0x48;

    TIMSK0 = (1 << TOIE0);
    TIMSK2 = (1 << TOIE2);
}

void pwm_set(uint16_t red, uint16_t green, uint16_t blue)
{
    printf_P(PSTR("%u.%u.%u\n"), red, green, blue);

    if (red > LED_MAX)
        red = LED_MAX;
    for (uint8_t i = 0; i < PWM_PHASES_T2; i++) {
        uint8_t v = red / (PWM_PHASES_T2 - i);
        red -= v;
        pwm_state.t2b_values[i] = 255 - v;
    }

    if (green > LED_MAX)
        green = LED_MAX;
    for (uint8_t i = 0; i < PWM_PHASES_T2; i++) {
        uint8_t v = green / (PWM_PHASES_T2 - i);
        green -= v;
        pwm_state.t2a_values[i] = 255 - v;
    }

    if (blue > LED_MAX)
        blue = LED_MAX;
    blue >>= 1;
    for (uint8_t i = 0; i < PWM_PHASES_T0; i++) {
        uint8_t v = blue / (PWM_PHASES_T0 - i);
        blue -= v;
        pwm_state.t0a_values[i] = 255 - v;
    }
}

SIGNAL(TIMER0_OVF_vect)
{
    uint8_t idx = pwm_state.t0_index;
    OCR0A = pwm_state.t0a_values[idx];
    pwm_state.t0_index = (idx + 1) & (PWM_PHASES_T0 - 1);
}

SIGNAL(TIMER2_OVF_vect)
{
    uint8_t idx = pwm_state.t2_index;
    OCR2A = pwm_state.t2a_values[idx];
    OCR2B = pwm_state.t2b_values[idx];
    pwm_state.t2_index = (idx + 1) & (PWM_PHASES_T2 - 1);
}
