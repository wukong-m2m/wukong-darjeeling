#include <avr/io.h>

void wunode_init()
{
	set_output(DDRK, 0);
	set_output(DDRK, 1);
	set_output(DDRK, 2);
	set_output(DDRK, 3);
	output_low(PORTK, 0);
	output_low(PORTK, 1);
	output_low(PORTK, 2);
	output_low(PORTK, 3);
sadsa
}
