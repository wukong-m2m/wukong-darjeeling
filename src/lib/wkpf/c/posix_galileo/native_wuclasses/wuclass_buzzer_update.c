#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wuclass_buzzer_setup(wuobject_t *wuobject) {
	//gpio24 corresponds to arduino port IO6
	if( access( "/sys/class/gpio/gpio24/value", F_OK ) == -1 ) {
		system("echo -n 24 > /sys/class/gpio/export");
	}
	system("echo -n out > /sys/class/gpio/gpio24/direction");
	system("echo -n strong > /sys/class/gpio/gpio24/drive");
}

void wuclass_buzzer_update(wuobject_t *wuobject) {
	bool onOff;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_BUZZER_ON_OFF, &onOff);

	if (onOff)
		system("echo -n 1 > /sys/class/gpio/gpio24/value");
	else
		system("echo -n 0 > /sys/class/gpio/gpio24/value");

	printf("WKPFUPDATE(Buzzer): Setting buzzer to: %x\n", onOff);
}
