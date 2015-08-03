#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include <mraa.h>

mraa_gpio_context button_gpio;

void wuclass_button_setup(wuobject_t *wuobject) {
    button_gpio = mraa_gpio_init(3);
    mraa_gpio_dir(button_gpio, MRAA_GPIO_IN);
}

void wuclass_button_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = mraa_gpio_read(button_gpio);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NButton): Sensed binary value: %d\n", value_i); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_BUTTON_CURRENT_VALUE, value);
}
