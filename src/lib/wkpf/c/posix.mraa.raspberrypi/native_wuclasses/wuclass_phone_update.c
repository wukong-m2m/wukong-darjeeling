#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define PHONE_PIN 5

void wuclass_phone_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Phone): init failed\n");
        return;
    }
    pinMode(PHONE_PIN, 0);
}

void wuclass_phone_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = digitalRead(PHONE_PIN);
    value = (value_i != 1);  //because we use magnetic sensor which will output "1" when it's turned off
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Phone): Send value: %d\n", value);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PHONE_TALKING, value);

}
#endif