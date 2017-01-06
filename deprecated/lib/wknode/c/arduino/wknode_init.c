#include <avr/io.h>

#define output_low(port, pin) port &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define set_output(portdir, pin) portdir |= (1<<pin)

void wknode_init()
{
	set_output(DDRK, 0);
	set_output(DDRK, 1);
	set_output(DDRK, 2);
	set_output(DDRK, 3);
	output_low(PORTK, 0);
	output_low(PORTK, 1);
	output_low(PORTK, 2);
	output_low(PORTK, 3);
}
