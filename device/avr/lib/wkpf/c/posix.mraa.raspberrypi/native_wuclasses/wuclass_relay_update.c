#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define RELAY_PIN 8

void wuclass_relay_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Relay): init failed\n");
        return;
    }
    pinMode(RELAY_PIN, OUTPUT);
}

void wuclass_relay_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_RELAY_ON_OFF, &onOff);
    if (onOff){
      digitalWrite(RELAY_PIN, 1);
    }else{
      digitalWrite(RELAY_PIN, 0);
    }

    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Relay): Sensed value: %x\n", onOff);
}
#endif