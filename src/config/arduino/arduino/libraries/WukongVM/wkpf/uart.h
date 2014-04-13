#ifndef UARTH
#define UARTH

#include "types.h"

extern void uart_inituart(uint8_t uart, uint32_t baudrate);
extern void uart_write_byte(uint8_t uart, uint8_t byte);
extern bool uart_available(uint8_t uart, uint16_t wait_ms);
extern uint8_t uart_read_byte(uint8_t uart);

#endif // UARTH