#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_gpio_context la_gpio;

void wuclass_light_actuator_setup(wuobject_t *wuobject) {
    la_gpio = mraa_gpio_init(7);
    mraa_gpio_dir(la_gpio, MRAA_GPIO_OUT);
}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);
    if (onOff){
      mraa_gpio_write(la_gpio, 1);
    }else{
      mraa_gpio_write(la_gpio, 0);
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light): Sensed lightness: %x\n", onOff);
}
#endif