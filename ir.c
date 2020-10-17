#include "ir.h"
#include "main.h"

enum {
    S_IDLE  = 0,
    S_START = 1,
    S_RX    = 2,
    S_PAUSE = 3,
    S_NOISE = 4
};

typedef struct {
    uint8_t     was_overflow;
    uint8_t     state;
    uint16_t    prev_icr;
    uint8_t     bit_cnt;
    uint32_t    word;
    uint16_t    acc_1s;

    volatile uint8_t command;
    volatile uint8_t pulse_1s;
} IcState;

static IcState ic_state;

void ir_init(void)
{
    TCCR1B = 0x03; // IC on falling edge, clk/64
    TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
}

IrButton ir_read(void)
{
    IcState* ps = &ic_state;

    uint8_t cmd = ps->command;
    if (cmd) {
        ps->command = 0;
        printf("%02x ", cmd);
    }
    return (IrButton)cmd;
}

uint8_t is_pulse_1s(void)
{
    IcState* ps = &ic_state;
    uint8_t v = ps->pulse_1s;
    if (v)
        ps->pulse_1s = 0;
    return v;
}

SIGNAL(TIMER1_CAPT_vect)
{
    IcState* ps = &ic_state;

    uint16_t icr = ICR1;
    uint16_t diff = icr - ps->prev_icr;
    ps->prev_icr = icr;
    ps->was_overflow = 0;

    switch (ps->state) {
        case S_IDLE:
            ps->state = S_START;
            break;

        case S_START:
            if (diff > 12500/4 && diff < 14500/4) {         // 13420
                ps->bit_cnt = 32;
                ps->state = S_RX;
            } else if (diff > 10400/4 && diff < 12000/4) {  // 11210
                ps->state = S_PAUSE;
            } else {
                ps->state = S_NOISE;
            }
            break;

        case S_RX:
            ps->word <<= 1;
            if (diff > 950/4 && diff < 1300/4) {            // 1085-1176
            } else if (diff > 2000/4 && diff < 2500/4) {    // 2233-2288
                ps->word |= 1;
            } else {
                ps->state = S_NOISE;
                break;
            }

            if (--ps->bit_cnt == 0) {
                uint8_t b3 = ps->word >> 24;
                uint8_t b2 = (ps->word >> 16) & 0xff;
                uint8_t b1 = (ps->word >> 8) & 0xff;
                uint8_t b0 = ps->word & 0xff;
                if (b3 == (uint8_t)(~b2) && b1 == (uint8_t)(~b0))
                    ps->command = b1;
                ps->state = S_PAUSE;
                //ps->prev_icr += (uint16_t)(65536 - 55860/4); // 107400 start-start
            }
            break;

        case S_PAUSE:
            //if (diff > 94000/4 && diff < 98500/4)
            ps->state = S_START;
            break;

        case S_NOISE:
            break;
    }
}

SIGNAL(TIMER1_OVF_vect)
{
    IcState* ps = &ic_state;
    if (ps->was_overflow)
        ps->state = S_IDLE;
    ps->was_overflow = 1;

    uint16_t acc = ps->acc_1s + 4096;
    if (acc >= 15625) {
        acc -= 15625;
        ps->pulse_1s = 1;
    }
    ps->acc_1s = acc;
}
