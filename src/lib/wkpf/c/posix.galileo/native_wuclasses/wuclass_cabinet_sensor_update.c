#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include <mraa.h>

mraa_gpio_context shoe_gpio[3];
mraa_gpio_context light_gpio;

void wuclass_cabinet_setup(wuobject_t *wuobject) {
    shoe_gpio[0] = mraa_gpio_init(2);
    mraa_gpio_dir(shoe_gpio[0], MRAA_GPIO_IN);
    shoe_gpio[1] = mraa_gpio_init(3);
    mraa_gpio_dir(shoe_gpio[1], MRAA_GPIO_IN);
    shoe_gpio[2] = mraa_gpio_init(4);
    mraa_gpio_dir(shoe_gpio[2], MRAA_GPIO_IN);
    light_gpio = mraa_gpio_init(5);
    mraa_gpio_dir(light_gpio, MRAA_GPIO_OUT);
    mraa_gpio_write(light_gpio, 1);
}

void wuclass_cabinet_update(wuobject_t *wuobject) {
    static bool value[3];
    bool value_i[3];
    value_i[0] = mraa_gpio_read(shoe_gpio[0]);
    value_i[1] = mraa_gpio_read(shoe_gpio[1]);
    value_i[2] = mraa_gpio_read(shoe_gpio[2]);
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
