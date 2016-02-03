#include "config.h"
#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wuclass_relay_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure IO8 shield pin as an GPIO output
    if( access( "/sys/class/gpio/gpio40/value", F_OK ) == -1 ) {
      system("echo -n 40 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio41/value", F_OK ) == -1 ) {
      system("echo -n 41 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio40/direction");
    system("echo -n strong > /sys/class/gpio/gpio40/drive");
    system("echo -n strong > /sys/class/gpio/gpio41/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure IO8 shield pin as an GPIO output
    if( access( "/sys/class/gpio/gpio49/value", F_OK ) == -1 ) {
      system("echo -n 49 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio256/value", F_OK ) == -1 ) {
      system("echo -n 256 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio224/value", F_OK ) == -1 ) {
      system("echo -n 224 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo -n in > /sys/class/gpio/gpio214/direction");
    system("echo -n out > /sys/class/gpio/gpio256/direction");
    system("echo -n in > /sys/class/gpio/gpio224/direction");
    system("echo -n out > /sys/class/gpio/gpio49/direction");
    system("echo -n out > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_relay_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_RELAY_ON_OFF, &onOff);

    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    if (onOff)
            system("echo -n 1 > /sys/class/gpio/gpio40/value");
    else
            system("echo -n 0 > /sys/class/gpio/gpio40/value");
    #endif
    #ifdef INTEL_EDISON
    if (onOff){
            system("echo high > /sys/class/gpio/gpio214/direction");
            system("echo -n 1 > /sys/class/gpio/gpio256/value");
            system("echo -n 1 > /sys/class/gpio/gpio49/value");
    }else{
            system("echo high > /sys/class/gpio/gpio214/direction");
            system("echo -n 1 > /sys/class/gpio/gpio256/value");
            system("echo -n 0 > /sys/class/gpio/gpio49/value");
    }
    #endif 
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Relay): Sensed value: %x\n", onOff);
}
#endif