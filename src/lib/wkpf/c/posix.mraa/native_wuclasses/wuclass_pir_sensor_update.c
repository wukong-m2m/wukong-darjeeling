#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_gpio_context pir_gpio;

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
    pir_gpio = mraa_gpio_init(5);
    mraa_gpio_dir(pir_gpio, MRAA_GPIO_IN);
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = mraa_gpio_read(pir_gpio);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PIR): Sensed binary value: %d\n", value_i); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, value);
}
#endif