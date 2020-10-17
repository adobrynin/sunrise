#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU           16000000UL

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define COUNTOF(arr)                (sizeof(arr)/sizeof(arr[0]))

#define LED_ON()                    (PORTB |= 0x20)
#define LED_OFF()                   (PORTB &= ~0x20)

#define LIMIT(v,vmin,vmax)          (((v) < (vmin)) ? (vmin) : ((v) > (vmax)) ? (vmax) : (v))

#define LED_MAX                     4080

void rgb_set(uint16_t red, uint16_t green, uint16_t blue);

#endif // MAIN_H_
