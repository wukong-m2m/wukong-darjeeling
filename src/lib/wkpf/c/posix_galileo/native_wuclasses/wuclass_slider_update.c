#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
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
#include "../../../../../config/galileo/c/config.h"

void wuclass_slider_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    system("echo -n 37 > /sys/class/gpio/export");
    system("echo -n out > /sys/class/gpio/gpio37/direction");
    system("echo -n 0 > /sys/class/gpio/gpio37/value");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure A0 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio49/value", F_OK ) == -1 ) {
      system("echo -n 49 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio49/direction");
    system("echo -n strong > /sys/class/gpio/gpio49/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure A0 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio200/value", F_OK ) == -1 ) {
      system("echo -n 200 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio232/value", F_OK ) == -1 ) {
      system("echo -n 232 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio208/value", F_OK ) == -1 ) {
      system("echo -n 208 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo high > /sys/class/gpio/gpio200/direction");
    system("echo low > /sys/class/gpio/gpio232/direction");
    system("echo in > /sys/class/gpio/gpio208/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_slider_update(wuobject_t *wuobject) {
	
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    #ifdef INTEL_GALILEO_GEN1
    fd = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    #endif
    #ifdef INTEL_GALILEO_GEN2
    fd = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    #endif
    #ifdef INTEL_EDISON
    fd = open("/sys/bus/iio/devices/iio:device1/in_voltage0_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    system("echo high > /sys/class/gpio/gpio214/direction"); // this line is nesseccesry but have no idea how to explain it
    #endif 
    int16_t num=0;
    int16_t i;
    //use this loop to convert char to int
    //at first, we use atoi (e.g. num=atoi(buf)) but quickly realize that atoi is not reliable
    for(i=0;i<4;i++){
        if(buf[i]>='0' && buf[i] <='9')
            num = num*10 + (buf[i] - '0');
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): Sensed value: %d\n", num);
    
    int16_t low;
    int16_t high;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_LOW_VALUE, &low);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_HIGH_VALUE, &high);
    int16_t range = high-low;
    int16_t output = (int16_t)((num/4095.0)*range+low);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SLIDER_OUTPUT, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): Sensed %d, low %d, high %d, output %d\n", num, low, high, output);
}
