#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <stdio.h>
#include <sys/io.h>

#define EBOX_PORT0 0x78
#define EBOX_PORT1 0x79
#define EBOX_PORT2 0x7a
#define EBOX_DDR0  0x98
#define EBOX_DDR1  0x99
#define EBOX_DDR2  0x9a

#define EBOX_LIGHT_ACTUATOR_PIN 0

#define setbit(port, bit) (outb(port, inb(port) | (1<<bit)))
#define clearbit(port, bit) (outb(port, inb(port) & ~(1<<bit)))

void wuclass_light_actuator_setup(wuobject_t *wuobject) {}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
	bool onOff;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);

	iopl(3); // Allow IO
	setbit(EBOX_DDR0, EBOX_LIGHT_ACTUATOR_PIN);
	if (onOff)
		setbit(EBOX_PORT0, EBOX_LIGHT_ACTUATOR_PIN);
	else
		clearbit(EBOX_PORT0, EBOX_LIGHT_ACTUATOR_PIN);

	printf("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}
