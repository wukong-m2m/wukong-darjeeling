#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <math.h>
#include "config.h"

#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_aio_context adc_a1;

void wuclass_light_sensor_setup(wuobject_t *wuobject) {
    adc_a1 = mraa_aio_init(1);
}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = mraa_aio_read(adc_a1);
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light_Sensor): output %d\n", output);
}
#endif