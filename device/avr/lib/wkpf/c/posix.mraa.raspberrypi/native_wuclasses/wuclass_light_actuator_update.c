#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define LIGHT_ACTUATOR_PIN 7

void wuclass_light_actuator_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light): init failed\n");
        return;
    }
    pinMode(LIGHT_ACTUATOR_PIN, OUTPUT);
}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);
    if (onOff){
      digitalWrite(LIGHT_ACTUATOR_PIN, 1);
    }else{
      digitalWrite(LIGHT_ACTUATOR_PIN, 0);
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light): Sensed lightness: %x\n", onOff);
}
#endif