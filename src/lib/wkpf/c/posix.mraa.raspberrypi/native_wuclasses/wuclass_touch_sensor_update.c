#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define TOUCH_PIN 4

void wuclass_touch_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Touch Sensor): init failed\n");
        return;
    }
    pinMode(TOUCH_PIN, 0);
}

void wuclass_touch_sensor_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = digitalRead(TOUCH_PIN);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Touch Sensor): Sensed binary value: %d\n", value);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_TOUCH_SENSOR_CURRENT_VALUE, value);
}
#endif