#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void wuclass_button_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure IO3 shield pin as a GPIO Input.
    if( access( "/sys/class/gpio/gpio62/value", F_OK ) == -1 ) {
      system("echo -n 62 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio17/value", F_OK ) == -1 ) {
      system("echo -n 17 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio76/value", F_OK ) == -1 ) {
      system("echo -n 76 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio64/value", F_OK ) == -1 ) {
      system("echo -n 64 > /sys/class/gpio/export");
    }
    system("echo -n strong > /sys/class/gpio/gpio17/drive");
    system("echo 0 > /sys/class/gpio/gpio76/value");
    system("echo 0 > /sys/class/gpio/gpio64/value");
    system("echo in > /sys/class/gpio/gpio62/direction");
    #endif
    #ifdef INTEL_EDISON
    //Configure IO3 shield pin as a GPIO Input.
    if( access( "/sys/class/gpio/gpio12/value", F_OK ) == -1 ) {
      system("echo -n 12 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio251/value", F_OK ) == -1 ) {
      system("echo -n 251 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio219/value", F_OK ) == -1 ) {
      system("echo -n 219 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo low > /sys/class/gpio/gpio251/direction");
    system("echo in > /sys/class/gpio/gpio219/direction");
    system("echo mode0 > /sys/kernel/debug/gpio_debug/gpio12/current_pinmux");
    system("echo in > /sys/class/gpio/gpio12/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_button_update(wuobject_t *wuobject) {
    bool value;
    int value_i;
    FILE *fp = NULL;
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2 
    while (fp == NULL)
      fp = fopen("/sys/class/gpio/gpio62/value", "r");
    #endif
    #ifdef INTEL_EDISON
    while (fp == NULL)
      fp = fopen("/sys/class/gpio/gpio12/value", "r");
    #endif
    fscanf(fp, "%d", &value_i);
    fclose(fp);
    value = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Button): Sensed binary value: %d\n", value); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_BUTTON_CURRENT_VALUE, value);
}
