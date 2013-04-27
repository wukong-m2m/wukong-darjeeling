#include "uart.h"
#include "msp430f1611.h"

#define UART_COUNT 2

const uint8_t P3SEL_ENABLE_UART[UART_COUNT] = {, BIT6+BIT7};
const uint8_t ME1_ENABLE_UART[UART_COUNT] = {, UTXE1+URXE1};

void uart_inituart(uint8_t uart, uint32_t baudrate) {
	if (uart==0) {
		P3SEL |= BIT4+BIT5; // Configure pins for UART use (instead of GPIO)
		ME1 |= UTXE0+URXE0; // Enable RX and TX on the UART
		U1TCTL 
	} else if (uart==1) {
		P3SEL |= BIT6+BIT7; // Configure pins for UART use (instead of GPIO)
		ME2 |= UTXE1+URXE1; // Enable RX and TX on the UART
	}
}

void uart_write_byte(uint8_t uart, uint8_t byte) {

}

bool uart_available(uint8_t uart, uint16_t wait_ms) {

}

uint8_t uart_read_byte(uint8_t uart) {

}


