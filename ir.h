#ifndef IR_H_
#define IR_H_

#include <stdint.h>

typedef enum {
    NO_BUTTON       = 0,
    B_BRIGHT_UP     = 0x3a,
    B_BRIGHT_DOWN   = 0xba,
    B_PLAY          = 0x82,
    B_POWER         = 0x02,
    B_RED           = 0x1a,
    B_GREEN         = 0x9a,
    B_BLUE          = 0xa2,
    B_WHITE         = 0x22,
    B_7             = 0x2a,
    B_8             = 0xaa,
    B_9             = 0x92,
    B_4             = 0x0a,
    B_5             = 0x8a,
    B_6             = 0xb2,
    B_1             = 0x38,
    B_2             = 0xb8,
    B_3             = 0x78,
    B_YELLOW        = 0x18,
    B_0             = 0x98,
    B_DELETE        = 0x58,
    B_PINK_2        = 0x12,
    B_PINK_1        = 0x32,
    B_GRAY_2        = 0xf8,
    B_GRAY_1        = 0xd8,
    B_RED_UP        = 0x28,
    B_RED_DOWN      = 0x08,
    B_GREEN_UP      = 0xa8,
    B_GREEN_DOWN    = 0x88,
    B_BLUE_UP       = 0x68,
    B_BLUE_DOWN     = 0x48,
    B_SET_CLOCK     = 0x30, // DIY1
    B_SET_ALARM     = 0xb0, // DIY2
    B_DIY3          = 0x70,
    B_DIY4          = 0x10,
    B_DIY5          = 0x90,
    B_DIY6          = 0x50,
    B_QUICK         = 0xe8,
    B_SLOW          = 0xc8,
    B_AUTO          = 0xf0,
    B_FLASH         = 0xd0,
    B_FADE7         = 0xe0,
    B_FADE3         = 0x60,
    B_JUMP7         = 0xa0,
    B_JUMP3         = 0x20,
} IrButton ;

void ir_init(void);
IrButton ir_read(void);

uint8_t is_pulse_1s(void);

#endif // IR_H_
