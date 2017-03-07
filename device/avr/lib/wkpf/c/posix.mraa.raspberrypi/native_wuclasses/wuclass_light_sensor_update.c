#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define LIGHT_SENSOR_PIN 0 // A0

void wuclass_light_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light_Sensor): init failed\n");
        return;
    }
}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = analogRead(LIGHT_SENSOR_PIN);
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light_Sensor): output %d\n", output);
}
#endif