#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define PIR_PIN 5

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PIR): init failed\n");
        return;
    }
    pinMode(PIR_PIN, INPUT);
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = digitalRead(PIR_PIN);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PIR): Sensed binary value: %d\n", value_i);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, value);
}
#endif