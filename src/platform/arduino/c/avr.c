/*
 * avr.c
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "avr.h"

// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW ((64 * 256) / (F_CPU / 1000000L))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

SIGNAL(TIMER0_OVF_vect)
{

	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

unsigned long avr_millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}

void avr_delay(unsigned long ms)
{
	unsigned long start = avr_millis();

	while (avr_millis() - start <= ms)
		;
}


void avr_timerInit()
{
	// this needs to be called before setup() or some functions won't
	// work there
	sei();

	// set timer 0 prescale factor to 64
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);

	// enable timer 0 overflow interrupt
	sbi(TIMSK0, TOIE0);

	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	/*
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);
	*/

	// enable a2d conversions
	sbi(ADCSRA, ADEN);

}

// adapted from the wiring stuff
void avr_serialInit(uint32_t baud)
{
	uint16_t baud_setting;

	UCSR0A = 0;

	baud_setting = (F_CPU / 8 / baud - 1) / 2;
    // baud_setting = (CLOCKSPEED + (baud * 8L)) / (baud * 16L) - 1;

    // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;

    sbi(UCSR0B, RXEN0);
    sbi(UCSR0B, TXEN0);
    sbi(UCSR0B, RXCIE0);
}

void avr_serialPrint(char * str)
{
	int i;
	for (i=0; str[i]!=0; i++) {
		if (str[i] == '\n')
			avr_serialWrite('\r');
		avr_serialWrite(str[i]);
	}
}
void avr_serialVPrint(char * format, va_list arg)
{
	static char temp[128];
	vsnprintf(temp, 128, format, arg);
	avr_serialPrint(temp);
}

void avr_serialPrintf(char * format, ...)
{
	va_list arg;

	va_start(arg, format);
	avr_serialVPrint(format, arg);
	va_end(arg);

}

void avr_serialWrite(unsigned char value)
{
	while (!((UCSR0A) & (1 << UDRE0)));
	UDR0 = value;
}
