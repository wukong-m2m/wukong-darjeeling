#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define SOUND_PIN 3

void wuclass_sound_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): init failed\n");
        return;
    }
}

void wuclass_sound_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = analogRead(SOUND_PIN);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): raw value %d\n", output);
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SOUND_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): output %d\n", output);
}
#endif