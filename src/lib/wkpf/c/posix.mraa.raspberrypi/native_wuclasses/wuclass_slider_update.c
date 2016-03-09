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

mraa_aio_context adc_a0;

void wuclass_slider_setup(wuobject_t *wuobject) {
    adc_a0 = mraa_aio_init(0);
    DEBUG_LOG(true, "WKPFUPDATE(Slider): Slider\n");
}

void wuclass_slider_update(wuobject_t *wuobject) {
    int16_t num=0;
    num = mraa_aio_read(adc_a0);
    int16_t low;
    int16_t high;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_LOW_VALUE, &low);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_HIGH_VALUE, &high);
    int16_t range = high-low;
    int16_t output = (int16_t)((num/4095.0)*range+low);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SLIDER_OUTPUT, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): Sensed %d, low %d, high %d, output %d\n", num, low, high, output);
}
#endif