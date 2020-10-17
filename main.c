#include "main.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "fsm.h"
#include "ir.h"
#include "pwm.h"
#include "uart.h"

enum {
    NO_TIME_SET = 0,
    SET_CLOCK   = 1,
    SET_ALARM   = 2
};

typedef struct {
    uint8_t     state;
    uint8_t     entered;
    uint8_t     digits[4];
    uint16_t    prev_red;
    uint16_t    prev_green;
    uint16_t    prev_blue;
} TimeSetState;
static TimeSetState time_set;

char pc_cmd[32];
uint8_t pc_filled;

uint32_t wall_clock;
uint32_t alarm_time = 100000ul;

uint16_t g_red;
uint16_t g_green;
uint16_t g_blue;

static inline uint16_t inc16(uint16_t v) {
    if (v < LED_MAX)
        v++;
    return v;
}
static inline uint16_t dec16(uint16_t v) {
    if (v > 0)
        v--;
    return v;
}

void rgb_set(uint16_t red, uint16_t green, uint16_t blue)
{
    g_red   = red;
    g_green = green;
    g_blue  = blue;
    pwm_set(red, green, blue);
}

void parse_pc_command(const char* cmd)
{
    char* pend;
    uint16_t red = strtoul(cmd, &pend, 10);
    if (*pend == '\0')
        return;
    cmd = pend + 1;
    uint16_t green = strtoul(cmd, &pend, 10);
    if (*pend == '\0')
        return;
    cmd = pend + 1;
    uint16_t blue = strtoul(cmd, &pend, 10);
    if (*pend != '\0')
        return;

    rgb_set(red, green, blue);
}

void pc_process(void)
{
    if (uart_is_available()) {
        char c = uart_read();
        putchar(c);
        if (c == '\r')
            putchar('\n');
        //printf_P(PSTR(" %02x"), c);

        if (c == '\r' || c == '\n') {
            pc_cmd[pc_filled] = '\0';
            parse_pc_command(pc_cmd);
            pc_filled = 0;
        } else if (pc_filled < COUNTOF(pc_cmd) - 1) {
            pc_cmd[pc_filled++] = c;
        }
    }
}

void load_settings(void)
{
    alarm_time = eeprom_read_dword((const uint32_t*)2);
    EEAR = 0;

    if (alarm_time < 86400) {
        uint32_t v = alarm_time / 60ul;
        uint16_t hours = v / 60ul;
        uint16_t minutes = v % 60ul;
        printf_P(PSTR("alarm set: %02u:%02u\n"), hours, minutes);
    } else {
        printf_P(PSTR("alarm not set\n"));
    }
}

void save_settings(void)
{
    eeprom_write_dword((uint32_t*)2, alarm_time);
    EEAR = 0;
}

void time_set_process(IrButton btn)
{
    static const uint16_t levels[4] = { 80, 300, 800, 2500 };
    TimeSetState* ps = &time_set;
    switch (btn) {
        case NO_BUTTON:
        default:
            return;
        case B_POWER:
        case B_DELETE:
            ps->state = NO_TIME_SET;
            rgb_set(ps->prev_red, ps->prev_green, ps->prev_blue);
            return;

        case B_SET_CLOCK:
        case B_SET_ALARM:
            ps->state = (btn == B_SET_CLOCK) ? SET_CLOCK : SET_ALARM;
            ps->entered = 0;
            ps->prev_red   = g_red;
            ps->prev_green = g_green;
            ps->prev_blue  = g_blue;
            break;
        case B_0: ps->digits[ps->entered++] = 0; break;
        case B_1: ps->digits[ps->entered++] = 1; break;
        case B_2: ps->digits[ps->entered++] = 2; break;
        case B_3: ps->digits[ps->entered++] = 3; break;
        case B_4: ps->digits[ps->entered++] = 4; break;
        case B_5: ps->digits[ps->entered++] = 5; break;
        case B_6: ps->digits[ps->entered++] = 6; break;
        case B_7: ps->digits[ps->entered++] = 7; break;
        case B_8: ps->digits[ps->entered++] = 8; break;
        case B_9: ps->digits[ps->entered++] = 9; break;
    }

    if (ps->entered < 4) {
        uint16_t red   = (ps->state == SET_ALARM) ? levels[ps->entered] : 0;
        uint16_t green = (ps->state == SET_CLOCK) ? levels[ps->entered] : 0;
        rgb_set(red, green, 0);
        return;
    }

    uint8_t hours   = ps->digits[0] * 10 + ps->digits[1];
    uint8_t minutes = ps->digits[2] * 10 + ps->digits[3];
    if (hours >= 24 || minutes >= 60) {
        rgb_set(LED_MAX, LED_MAX, LED_MAX);
        _delay_ms(50);
        rgb_set(0, 0, 0);
        _delay_ms(50);
        rgb_set(LED_MAX, LED_MAX, LED_MAX);
        _delay_ms(50);
        rgb_set(0, 0, 0);
        ps->state = NO_TIME_SET;
        rgb_set(ps->prev_red, ps->prev_green, ps->prev_blue);
        return;
    }

    rgb_set(0, 0, 0);
    _delay_ms(100);
    if (ps->state == SET_CLOCK) {
        printf_P(PSTR("clock set: %02u:%02u\n"), hours, minutes);
        wall_clock = ((hours * 60u) + minutes) * 60ul;
    } else {
        printf_P(PSTR("alarm set: %02u:%02u\n"), hours, minutes);
        alarm_time = ((hours * 60u) + minutes) * 60ul;
        if (alarm_time == 0)
            alarm_time = 100000ul;
        save_settings();
    }

    ps->state = NO_TIME_SET;
    rgb_set(ps->prev_red, ps->prev_green, ps->prev_blue);
}

void brightness_up(void)
{
    if (g_red < 100 && g_green < 100 && g_blue < 100) {
        g_red   += 50;
        g_green += 50;
        g_blue  += 50;
        return;
    }

    uint16_t red   = (g_red   * 11 + 5) / 10;
    uint16_t green = (g_green * 11 + 5) / 10;
    uint16_t blue  = (g_blue  * 11 + 5) / 10;
    if (red > LED_MAX || green > LED_MAX || blue > LED_MAX)
        return;

    g_red   = red;
    g_green = green;
    g_blue  = blue;
}

void brightness_down(void)
{
    g_red   = (g_red   * 9 + 5) / 10;
    g_green = (g_green * 9 + 5) / 10;
    g_blue  = (g_blue  * 9 + 5) / 10;
}

void ir_process(void)
{
    IrButton btn = ir_read();
    if (time_set.state != NO_TIME_SET) {
        time_set_process(btn);
        return;
    }
    if (btn != NO_BUTTON)
        fsm_stop();

    switch (btn) {
        case NO_BUTTON:
        default:
            return;
        case B_POWER:
            printf_P(PSTR("power off\n"));
            g_red   = 0;
            g_green = 0;
            g_blue  = 0;
            break;
        case B_BRIGHT_UP:
            brightness_up();
            break;
        case B_BRIGHT_DOWN:
            brightness_down();
            break;
        case B_RED:
            g_red   = LED_MAX;
            g_green = 0;
            g_blue  = 0;
            break;
        case B_GREEN:
            g_red   = 0;
            g_green = LED_MAX;
            g_blue  = 0;
            break;
        case B_BLUE:
            g_red   = 0;
            g_green = 0;
            g_blue  = LED_MAX;
            break;
        case B_WHITE:
            g_red   = LED_MAX;
            g_green = LED_MAX;
            g_blue  = LED_MAX;
            break;
        case B_PINK_2:
            g_red   = LED_MAX;
            g_green = LED_MAX/2;
            g_blue  = LED_MAX/10;
            break;
        case B_PINK_1:
            g_red   = LED_MAX;
            g_green = (LED_MAX*2)/5;
            g_blue  = (LED_MAX*2)/25;
            break;
        case B_GRAY_2:
            g_red   = LED_MAX;
            g_green = (LED_MAX*3)/10;
            g_blue  = LED_MAX/25;
            break;
        case B_GRAY_1:
            g_red   = LED_MAX;
            g_green = LED_MAX/4;
            g_blue  = LED_MAX/50;
            break;
        case B_RED_UP:
            g_red = inc16(g_red);
            break;
        case B_RED_DOWN:
            g_red = dec16(g_red);
            break;
        case B_GREEN_UP:
            g_green = inc16(g_green);
            break;
        case B_GREEN_DOWN:
            g_green = dec16(g_green);
            break;
        case B_BLUE_UP:
            g_blue = inc16(g_blue);
            break;
        case B_BLUE_DOWN:
            g_blue = dec16(g_blue);
            break;
        case B_SET_CLOCK:
        case B_SET_ALARM:
            time_set_process(btn);
            return;
        case B_QUICK:
            fsm_start(5);
            break;
        case B_SLOW:
            fsm_start(1);
            break;
    }
    pwm_set(g_red, g_green, g_blue);
}

int main() {
    uart_init();
    pwm_init();
    ir_init();
    fsm_stop();
    DDRB |= 0x20;
    LED_OFF();
    sei();

    printf_P(PSTR("\nHello world!\n"));
    load_settings();

    rgb_set(LED_MAX/3, LED_MAX/3, LED_MAX/3);
    _delay_ms(50);
    rgb_set(0, 0, 0);

    while (1) {
        pc_process();
        ir_process();
        if (is_pulse_1s()) {
            if (++wall_clock >= 86400)
                wall_clock = 0;
            if (wall_clock == alarm_time)
                fsm_start(1);
            fsm_next();
        }
    }
}
