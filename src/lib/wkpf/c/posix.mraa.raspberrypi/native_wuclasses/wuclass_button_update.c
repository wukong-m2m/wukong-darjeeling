#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define BUTTON_PIN 5

void wuclass_button_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NButton): init failed\n");
        return;
    }
    pinMode(BUTTON_PIN, 0);
}

void wuclass_button_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = digitalRead(BUTTON_PIN);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NButton): Sensed binary value: %d\n", value_i);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_BUTTON_CURRENT_VALUE, value);
}
#endif