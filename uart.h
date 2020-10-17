#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void uart_init(void);

uint8_t uart_is_available(void);
char uart_read(void);

#endif // UART_H_
