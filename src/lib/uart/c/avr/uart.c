#include "uart.h"
#include "types.h"
#include "djtimer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
sdf
#ifdef WUTINY
#define CLOCK 11059200
#define UART_BAUDRATE_DOUBLE_SPEED 1
#else
// Mostly copied from NanoVM's uart.c
#define CLOCK 16000000 // Not sure if there's a better way to get the clock speed, but right now all our devices are at 16MHz
#define UART_BAUDRATE_DOUBLE_SPEED 0
#endif

#define TXEN TXEN0
#define RXEN RXEN0
#define RXCIE RXCIE0
#define UDRE UDRE0
#define USBS USBS0
#define UCPHA UCSZ00
#define UDORD UCSZ01
#define UCPOL UCPOL0
#define U2X U2X0

#define UART_COUNT 4
#define UART_BUFFER_SIZE  (1<<(UART_BUFFER_BITS))
#define UART_BUFFER_MASK  ((UART_BUFFER_SIZE)-1)
#define UART_BUFFER_BITS 6       // Old comment from NanoVM: "32 bytes buffer (min. req for loader)" Not sure if we can lower this, since we're not using their bootloader

#ifdef WUTINY
volatile uint8_t *UBRRH[] = { &UBRR0H, &UBRR1H };
volatile uint8_t *UBRRL[] = { &UBRR0L, &UBRR1L };
volatile uint8_t *UCSRA[] = { &UCSR0A, &UCSR1A };
volatile uint8_t *UCSRB[] = { &UCSR0B, &UCSR1B };
volatile uint8_t *UCSRC[] = { &UCSR0C, &UCSR1C };
volatile uint8_t *UDR[] = { &UDR0, &UDR1 };
volatile uint16_t *UBRR[] = { &UBRR0, &UBRR1 };
#else
volatile uint8_t *UBRRH[] = { &UBRR0H, &UBRR1H, &UBRR2H, &UBRR3H };
volatile uint8_t *UBRRL[] = { &UBRR0L, &UBRR1L, &UBRR2L, &UBRR3L };
volatile uint8_t *UCSRA[] = { &UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A };
volatile uint8_t *UCSRB[] = { &UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B };
volatile uint8_t *UCSRC[] = { &UCSR0C, &UCSR1C, &UCSR2C, &UCSR3C };
volatile uint8_t *UDR[] = { &UDR0, &UDR1, &UDR2, &UDR3 };
volatile uint16_t *UBRR[] = { &UBRR0, &UBRR1, &UBRR2, &UBRR3 };
volatile uint8_t *XCKDDR[] = { &DDRE, &DDRD, &DDRH, &DDRJ};
volatile uint8_t XCKn[] = {DDE2, DDD5, DDH2, DDJ2};
#endif

uint8_t uart_rd[UART_COUNT], uart_wr[UART_COUNT];
uint8_t uart_buf[UART_COUNT][UART_BUFFER_SIZE];
uint8_t uart_openlog = 0;

// Interrupt handlers for receiving data
// Store byte and increase write pointer
#if defined(USART0_RX_vect)
SIGNAL(USART0_RX_vect) {
  uart_buf[0][uart_wr[0]] = *UDR[0];
  uart_wr[0] = ((uart_wr[0]+1) & UART_BUFFER_MASK);
}
#endif
#if defined(USART1_RX_vect)
SIGNAL(USART1_RX_vect) {
  uart_buf[1][uart_wr[1]] = *UDR[1];
  uart_wr[1] = ((uart_wr[1]+1) & UART_BUFFER_MASK);
}
#endif
#if defined(USART2_RX_vect)
SIGNAL(USART2_RX_vect) {
  uart_buf[2][uart_wr[2]] = *UDR[2];
  uart_wr[2] = ((uart_wr[2]+1) & UART_BUFFER_MASK);
}
#endif
#if defined(USART3_RX_vect)
SIGNAL(USART3_RX_vect) {
  uart_buf[3][uart_wr[3]] = *UDR[3];
  uart_wr[3] = ((uart_wr[3]+1) & UART_BUFFER_MASK);
}
#endif

void uart_close(uint8_t uart){
  *UCSRB[uart] &=
    ~(_BV(RXEN) | _BV(RXCIE) |          // enable receiver and irq
    _BV(TXEN));                        // enable transmitter
  uart_openlog &= ~_BV(uart+1);
}

static void uart_set_baudrate(uint8_t uart, uint32_t baudrate, uint8_t factor){
  /* set baud rate by rounding */
    *UBRR[uart] = (CLOCK / (factor>>1) / baudrate - 1) / 2;
}

void uart_inituart(uint8_t uart, uint32_t baudrate) {
	uint8_t stopbit = 1;
	uint8_t parity = 0;
  // WARNING:
  //    We cannot use more than 15-bit int which is at most 16383
  //    So Baudrate is needed to be considered.
  if (uart_openlog & _BV(uart+1)) uart_close(uart);  // if initialized, then close it
  uart_rd[uart] = uart_wr[uart] = 0;   // init buffers
#ifdef WUTINY
  bool u2x_flag = true;//(baudrate != 57600)?TRUE:FALSE;  // speed up
#else
  bool u2x_flag = true;//(baudrate != 57600)?TRUE:FALSE;  // speed up
#endif
  uint8_t factor = (u2x_flag) ? 8 : 16;

  *UCSRA[uart] = (u2x_flag) ? _BV(U2X) : 0;
  *UCSRB[uart] =
    _BV(RXEN) | _BV(RXCIE) |          // enable receiver and irq
    _BV(TXEN);                        // enable transmitter

  if (stopbit == 2){
    *UCSRC[uart] |= _BV(USBS);
  } else { // default: one stop bit
    *UCSRC[uart] &= ~_BV(USBS);
  }

  switch (parity){
    case 3: // Odd Parity
      *UCSRC[uart] |= _BV(UPM01) | _BV(UPM00);
      break;
    case 2: // Even Parity
      *UCSRC[uart] &= ~_BV(UPM00);
      *UCSRC[uart] |= _BV(UPM01);
      break;
    case 0: // default: No Parity
    default:
      *UCSRC[uart] &= ~(_BV(UPM00) | _BV(UPM01));
      break;
  }

  uart_set_baudrate(uart, baudrate, factor);

#ifdef URSEL // UCSRC shared with UBRRH in nibo
  *UCSRC[uart] |= _BV(URSEL) | _BV(UCSZ00) | _BV(UCSZ01);
#else
  *UCSRC[uart] |= _BV(UCSZ00) | _BV(UCSZ01);  // 8 bit data
#endif // URSEL

  sei();
  uart_openlog |= _BV(uart+1);
  if (uart == 0) {
  	while (true) { PORTA ^= 0x1;}
	while(true)
	uart_write_byte(0, 'a');
  }
}

void uart_write_byte(uint8_t uart, uint8_t byte) {
  /* Wait for empty transmit buffer */
  while(!(*UCSRA[uart] & _BV(UDRE)));

  // start transmission
  *UDR[uart] = byte;
}

bool uart_available(uint8_t uart, uint16_t wait_ms) {
  if (wait_ms == 0)
    return(UART_BUFFER_MASK & (uart_wr[uart] - uart_rd[uart]));

  dj_time_t timeout = dj_timer_getTimeMillis() + wait_ms;
  while (dj_timer_getTimeMillis() < timeout) {
    if (UART_BUFFER_MASK & (uart_wr[uart] - uart_rd[uart]))
      return true;
  }
  return false;
}

uint8_t uart_read_byte(uint8_t uart) {
  uint8_t ret = uart_buf[uart][uart_rd[uart]];

  /* and increase read pointer */
  uart_rd[uart] = ((uart_rd[uart]+1) & UART_BUFFER_MASK);

  return ret;
}
