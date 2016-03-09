#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define TV_ONOFF_IN_PIN 3
#define TV_ONOFF_OUT_PIN 4
#define TV_MUTE_OUT_PIN 8

void wuclass_irremote_tv_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(IRremote TV): init failed\n");
        return;
    }
    pinMode(TV_ONOFF_IN_PIN, 0);
    pinMode(TV_ONOFF_OUT_PIN, 1);
    pinMode(TV_MUTE_OUT_PIN, 1);
}

void wuclass_irremote_tv_update(wuobject_t *wuobject) {
    bool on_off;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_ON_OFF, &on_off);
    digitalWrite(TV_ONOFF_OUT_PIN, on_off);

    bool mute;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_MUTE, &mute);
    digitalWrite(TV_MUTE_OUT_PIN, mute);

    bool on_off_state;
    int value_i;
    value_i = digitalRead(TV_ONOFF_IN_PIN);
    on_off_state = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(IRremote TV): Sensed on_off_state: %d\n", value_i);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_ON_OFF_STATE, on_off_state);
}
#endif