#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define SLIDER_PIN 0 // A0

void wuclass_slider_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): init failed\n");
        return;
    }
}

void wuclass_slider_update(wuobject_t *wuobject) {
    int16_t num=0;
    num = analogRead(SLIDER_PIN);
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