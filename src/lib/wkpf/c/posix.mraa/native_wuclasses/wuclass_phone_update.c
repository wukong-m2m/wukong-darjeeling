#include "config.h"
#ifdef MRAA_LIBRARY

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <math.h>
#include <mraa.h>

mraa_gpio_context phone_gpio;

void wuclass_phone_setup(wuobject_t *wuobject) {
    phone_gpio = mraa_gpio_init(5);
    mraa_gpio_dir(phone_gpio, MRAA_GPIO_IN);
    DEBUG_LOG(true, "WKPFUPDATE(Phone): start\n");
}

void wuclass_phone_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    value_i = mraa_gpio_read(phone_gpio);
    value = (value_i != 1);  //because we use magnetic sensor which will output "1" when it's turned off
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Phone): Send value: %d\n", value); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PHONE_TALKING, value);

}
#endif