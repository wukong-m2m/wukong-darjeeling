#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)
#include "config.h"

void wuclass_light_actuator_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    //Configure IO7 shield pin as a GPIO output.
    if( access( "/sys/class/gpio/gpio27/value", F_OK ) == -1 ) {
        system("echo -n 27 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio27/direction");
    //The drive mode is set by writing the mode string ("pullup", "pulldown", "strong", or "hiz") to /sys/class/gpio/gpioXX/drive
    //Since this pin is connected to LED and sources significant current, the pin has to be set "strong"
    system("echo -n strong > /sys/class/gpio/gpio27/drive");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure IO7 shield pin as a GPIO output.
    if( access( "/sys/class/gpio/gpio38/value", F_OK ) == -1 ) {
      system("echo -n 38 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio39/value", F_OK ) == -1 ) {
      system("echo -n 39 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio38/direction");
    system("echo -n strong > /sys/class/gpio/gpio38/drive");
    system("echo -n strong > /sys/class/gpio/gpio39/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure IO7 shield pin as a GPIO output.
    //Enable the GPIO, 48: linux gpio, 255: output enable, 223: pull-up enable, 214: tri-state for GPIO power on off
    if( access( "/sys/class/gpio/gpio48/value", F_OK ) == -1 ) {
      system("echo -n 48 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio255/value", F_OK ) == -1 ) {
      system("echo -n 255 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio223/value", F_OK ) == -1 ) {
      system("echo -n 223 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo -n in > /sys/class/gpio/gpio214/direction");
    system("echo -n out > /sys/class/gpio/gpio255/direction");
    system("echo -n in > /sys/class/gpio/gpio223/direction");
    system("echo -n out > /sys/class/gpio/gpio48/direction");
    system("echo -n out > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
    bool onOff;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);

    #ifdef INTEL_GALILEO_GEN1
    if (onOff)
            system("echo -n 1 > /sys/class/gpio/gpio27/value");//writing the value to GPIO port
    else
            system("echo -n 0 > /sys/class/gpio/gpio27/value");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    if (onOff)
	    system("echo -n 1 > /sys/class/gpio/gpio38/value");
    else
	    system("echo -n 0 > /sys/class/gpio/gpio38/value");
    #endif
    #ifdef INTEL_EDISON
    if (onOff){
            system("echo high > /sys/class/gpio/gpio214/direction");
            system("echo -n 1 > /sys/class/gpio/gpio255/value");
            system("echo -n 1 > /sys/class/gpio/gpio48/value");
    }else{
            system("echo high > /sys/class/gpio/gpio214/direction");
            system("echo -n 1 > /sys/class/gpio/gpio255/value");
            system("echo -n 0 > /sys/class/gpio/gpio48/value");
    }
    #endif 
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light): Sensed lightness: %x\n", onOff);
}
#endif