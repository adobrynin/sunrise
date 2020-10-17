#include "fsm.h"
#include "main.h"

typedef struct {
    uint16_t    time;
    uint16_t    red;
    uint16_t    green;
    uint16_t    blue;
} Point;

static const Point points[] = { // TODO: to progmem
    {    0,    0,    0,    0 },
    {  100,   60,    0,    0 },
    {  100,   55,    1,    0 },
    {  200,  100,   10,    0 },
    {  300,  200,   25,    0 },
    {  400,  400,   60,    0 },
    {  500,  800,  150,    8 },
    {  600, 1600,  400,   20 },
    {  700, 3200,  800,   40 }
};

typedef struct {
    uint16_t    elapsed;
    uint8_t     speedup;
} Fsm;
static Fsm fsm;

static uint16_t interpolate(uint16_t x, uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1)
{
    return (int32_t)y0 + (((int32_t)y1 - (int32_t)y0) * (x - x0) + (x1 - x0)/2) / (x1 - x0);
}

void fsm_start(uint8_t speedup)
{
    fsm.elapsed = 0;
    fsm.speedup = speedup;
}

void fsm_stop(void)
{
    fsm.elapsed = 0xffff;
}

void fsm_next(void)
{
    if (fsm.elapsed == 0xffff)
        return;

    uint16_t elapsed = ++fsm.elapsed * fsm.speedup;

    const Point* p1 = NULL;
    for (uint8_t i = 1; i < (uint8_t)COUNTOF(points); i++) {
        if (points[i].time > elapsed) {
            p1 = &points[i];
            break;
        }
    }
    if (!p1) {
        fsm_stop();
        return;
    }

    const Point* p0 = p1 - 1;
    uint16_t red   = interpolate(elapsed, p0->time, p1->time, p0->red,   p1->red);
    uint16_t green = interpolate(elapsed, p0->time, p1->time, p0->green, p1->green);
    uint16_t blue  = interpolate(elapsed, p0->time, p1->time, p0->blue,  p1->blue);

    printf_P(PSTR("%3u - "), elapsed);
    printf_P(PSTR("%u:%u.%u.%u .. "), p0->time, p0->red, p0->green, p0->blue);
    printf_P(PSTR("%u:%u.%u.%u => "), p1->time, p1->red, p1->green, p1->blue);
    rgb_set(red, green, blue);
}
