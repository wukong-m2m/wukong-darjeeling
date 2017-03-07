#include "config.h"
#ifdef MRAA_LIBRARY

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
#include <mraa.h>

mraa_aio_context adc_a3;

void wuclass_sound_sensor_setup(wuobject_t *wuobject) {
    adc_a3 = mraa_aio_init(3);
}

void wuclass_sound_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = mraa_aio_read(adc_a3);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): raw value %d\n", output);
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SOUND_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): output %d\n", output);
}
#endif