#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void wuclass_number_setup(wuobject_t *wuobject) {
}

void wuclass_number_update(wuobject_t *wuobject) {
    int16_t num;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_NUMBER_INPUT, &num);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(number): get %d\n", num);
}
