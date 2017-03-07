#include "config.h"
#ifdef MRAA_LIBRARY

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "MOSFET_cjq4435.h"

void wuclass_mosfet_led_setup(wuobject_t *wuobject) {
    CJQ4435_Init(5);
    setPeriodMS(10);
    enable(true);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MOSFET_LED): ready\n");
}

void wuclass_mosfet_led_update(wuobject_t *wuobject) {
    int16_t brightness;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MOSFET_LED_BRIGHTNESS, &brightness);
    setDutyCycle(brightness/255.0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MOSFET_LED): %d\n", brightness);
}
#endif