#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_gpio_context touch_gpio;

void wuclass_touch_sensor_setup(wuobject_t *wuobject) {
    touch_gpio = mraa_gpio_init(4);
    mraa_gpio_dir(touch_gpio, MRAA_GPIO_IN);
}

void wuclass_touch_sensor_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = mraa_gpio_read(touch_gpio);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Touch Sensor): Sensed binary value: %d\n", value); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_TOUCH_SENSOR_CURRENT_VALUE, value);
}
#endif