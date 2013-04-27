#include <stdio.h>
#include "execution.h"

void javax_wukong_wknode_WuNodeLeds_void__setLed_int_boolean()
{
	unsigned char on = dj_exec_stackPopShort();
	unsigned int nr = dj_exec_stackPopInt();

	printf("Turning LED %d %s\n", nr, on ? "on" : "off");
}
