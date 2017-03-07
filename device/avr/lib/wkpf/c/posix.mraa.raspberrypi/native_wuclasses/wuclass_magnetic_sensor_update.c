#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define MAGNETIC_PIN 4

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Magnetic): init failed\n");
        return;
    }
    pinMode(MAGNETIC_PIN, INPUT);
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
    bool value;
    int16_t value_i;
    value_i = digitalRead(MAGNETIC_PIN);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Magnetic): Sensed binary value: %d\n", value);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_TOUCH_SENSOR_CURRENT_VALUE, value);
}
#endif