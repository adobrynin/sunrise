#include "uart.h"
#include "main.h"

typedef struct {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t buf[32];
} Circular32;

typedef struct {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t buf[256];
} Circular256;

static Circular32 rx_fifo;
static Circular256 tx_fifo;

SIGNAL(USART_UDRE_vect)
{
    uint8_t head = tx_fifo.head;
    uint8_t tail = tx_fifo.tail;
    if (head != tail) {
        UDR0 = tx_fifo.buf[tail];
        tx_fifo.tail = (tail + 1) & (COUNTOF(tx_fifo.buf) - 1);
    } else {
        UCSR0B &= ~(1 << UDRIE0);   // disable UDRE interrupt
    }
}

SIGNAL(USART_RX_vect)
{
    uint8_t sr_a = UCSR0A;
    uint8_t byte = UDR0;
    if ((sr_a & (1 << FE0)) == 0) {
        uint8_t head = rx_fifo.head;
        rx_fifo.buf[head] = byte;
        rx_fifo.head = (head + 1) & (COUNTOF(rx_fifo.buf) - 1);
    }
}

static int uart_putchar(char c, FILE* stream)
{
    uint8_t head = tx_fifo.head;
    uint8_t tail = tx_fifo.tail;
    uint8_t new_head = (head + 1) & (COUNTOF(tx_fifo.buf) - 1);
    if (new_head != tail) {
        tx_fifo.buf[head] = c;
        tx_fifo.head = new_head;
        UCSR0B |= (1 << UDRIE0);  // enable UDRE interrupt
    }
    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void)
{
    // 115200 8N1 @ 16 MHz, normal speed
    UBRR0 = 8;
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = 3 << UCSZ00;
    stdout = &uart_str;
}

uint8_t uart_is_available(void)
{
    return rx_fifo.head != rx_fifo.tail;
}

char uart_read(void)
{
    uint8_t head = rx_fifo.head;
    uint8_t tail = rx_fifo.tail;
    uint8_t c = 0;
    if (head != tail) {
        c = rx_fifo.buf[tail];
        rx_fifo.tail = (tail + 1) & (COUNTOF(rx_fifo.buf) - 1);
    }
    return c;
}
