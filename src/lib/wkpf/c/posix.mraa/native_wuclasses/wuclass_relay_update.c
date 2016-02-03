#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_gpio_context relay_gpio;

void wuclass_relay_setup(wuobject_t *wuobject) {
    relay_gpio = mraa_gpio_init(8);
    mraa_gpio_dir(relay_gpio, MRAA_GPIO_OUT);
}

void wuclass_relay_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_RELAY_ON_OFF, &onOff);
    if (onOff){
      mraa_gpio_write(relay_gpio, 1);
    }else{
      mraa_gpio_write(relay_gpio, 0);
    }

    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Relay): Sensed value: %x\n", onOff);
}
#endif