#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

void pwm_init(void);
void pwm_set(uint16_t red, uint16_t green, uint16_t blue);

#endif // PWM_H_
