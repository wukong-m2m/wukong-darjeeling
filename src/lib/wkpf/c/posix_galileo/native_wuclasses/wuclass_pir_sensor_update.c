#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
    if( access( "/sys/class/gpio/gpio27/value", F_OK ) == -1 ) {
        system("echo -n 27 > /sys/class/gpio/export");
    }
    system("echo -n in > /sys/class/gpio/gpio27/direction"); //gpio27-pin7, gpio26-pin8
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
	bool value;
	int value_i;
	FILE *fp = NULL;
	while (fp == NULL)
		fp = fopen("/sys/class/gpio/gpio27/value", "r");
	fscanf(fp, "%d", &value_i);
	fclose(fp);
	value = (value_i != 0);
	printf("WKPFUPDATE(PirSensor): Sensed binary value: %d\n", value);
	//system("echo \"value\" > /dev/ttyGS0"); //Serial (IDE Serial Monitor)
  	//delay(500);
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, value);
}
