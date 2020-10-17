#ifndef FSM_H_
#define FSM_H_

#include <stdint.h>

void fsm_start(uint8_t speedup);
void fsm_stop(void);
void fsm_next(void);

#endif // FSM_H_
