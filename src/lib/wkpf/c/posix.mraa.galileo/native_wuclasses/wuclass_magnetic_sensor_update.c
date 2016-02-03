#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure IO4 shield pin as a GPIO Input.
    if( access( "/sys/class/gpio/gpio6/value", F_OK ) == -1 ) {
	system("echo -n 6 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio36/value", F_OK ) == -1 ) {
	system("echo -n 36 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio37/value", F_OK ) == -1 ) {
	system("echo -n 37 > /sys/class/gpio/export");
    }
    system("echo in > /sys/class/gpio/gpio6/direction");
    system("echo strong > /sys/class/gpio/gpio37/drive");
    system("echo in > /sys/class/gpio/gpio36/direction");
    #endif
    #ifdef INTEL_EDISON
    //Configure IO4 shield pin as a GPIO Input.
    if( access( "/sys/class/gpio/gpio129/value", F_OK ) == -1 ) {
	system("echo -n 129 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio252/value", F_OK ) == -1 ) {
	system("echo -n 252 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio220/value", F_OK ) == -1 ) {
	system("echo -n 220 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
	system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo low > /sys/class/gpio/gpio252/direction");
    system("echo in > /sys/class/gpio/gpio220/direction");
    system("echo mode0 > /sys/kernel/debug/gpio_debug/gpio129/current_pinmux");
    system("echo in > /sys/class/gpio/gpio129/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
    bool value;
    int16_t value_i;
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    char this_gpio[10]={"gpio6"};
    value_i = gpio_read(this_gpio);
    #endif
    #ifdef INTEL_EDISON
    char this_gpio[10]={"gpio129"};
    value_i = gpio_read(this_gpio);
    #endif
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Magnetic Sensor): Sensed binary value: %d\n", value); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_TOUCH_SENSOR_CURRENT_VALUE, value);
}
#endif