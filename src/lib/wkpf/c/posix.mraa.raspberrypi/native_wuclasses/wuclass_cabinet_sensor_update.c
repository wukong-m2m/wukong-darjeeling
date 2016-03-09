#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "grovepi.h"

#define SHOE0_PIN 2
#define SHOE1_PIN 3
#define SHOE2_PIN 4
#define LIGHT_PIN 5

void wuclass_cabinet_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(cabinet): init failed\n");
        return;
    }
    pinMode(SHOE0_PIN, 0);
    pinMode(SHOE1_PIN, 0);
    pinMode(SHOE2_PIN, 0);
    pinMode(LIGHT_PIN, 1);
    digitalWrite(LIGHT_PIN, 1);
}


void wuclass_cabinet_update(wuobject_t *wuobject) {
    static bool value[3];
    bool value_i[3];
    value_i[0] = digitalRead(SHOE0_PIN);
    value_i[1] = digitalRead(SHOE1_PIN);
    value_i[2] = digitalRead(SHOE2_PIN);
    if (value[0] != value_i[0] || value[1] != value_i[1] || value[2] != value_i[2]) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(cabinet): value: %d %d %d\n", value_i[0], value_i[1], value_i[2]);
        value[0] = value_i[0];
        value[1] = value_i[1];
        value[2] = value_i[2];
    }
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CABINET_SPACE1, value_i[0]);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CABINET_SPACE2, value_i[1]);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CABINET_SPACE3, value_i[2]);
}
#endif