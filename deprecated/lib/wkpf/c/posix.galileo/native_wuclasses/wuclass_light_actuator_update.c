#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wuclass_light_actuator_setup(wuobject_t *wuobject) {
	if( access( "/sys/class/gpio/gpio26/value", F_OK ) == -1 ) {
		system("echo -n 26 > /sys/class/gpio/export");
	}
	system("echo -n out > /sys/class/gpio/gpio26/direction");
}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
	bool onOff;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);

	if (onOff)
		system("echo -n 1 > /sys/class/gpio/gpio26/value");
	else
		system("echo -n 0 > /sys/class/gpio/gpio26/value");

	printf("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}
