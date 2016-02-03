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
#include "config.h"
#include <mraa.h>
#include "IO_utils.h"

mraa_aio_context adc_a1;

void wuclass_light_sensor_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    system("echo -n 36 > /sys/class/gpio/export");
    system("echo -n out > /sys/class/gpio/gpio36/direction");
    system("echo -n 0 > /sys/class/gpio/gpio36/value");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure A1 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio51/value", F_OK ) == -1 ) {
      system("echo -n 51 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio51/direction");
    system("echo -n strong > /sys/class/gpio/gpio51/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure A1 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio201/value", F_OK ) == -1 ) {
      system("echo -n 201 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio233/value", F_OK ) == -1 ) {
      system("echo -n 233 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio209/value", F_OK ) == -1 ) {
      system("echo -n 209 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo high > /sys/class/gpio/gpio201/direction");
    system("echo low > /sys/class/gpio/gpio233/direction");
    system("echo in > /sys/class/gpio/gpio209/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
    #ifdef MRAA_LIBRARY
    adc_a1 = mraa_aio_init(1);
    #endif
}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    #if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2)
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    output = aio_read(buf);
    #endif
    #ifdef INTEL_EDISON
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device1/in_voltage1_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    system("echo high > /sys/class/gpio/gpio214/direction");
    output = aio_read(buf);
    #endif
    #ifdef MRAA_LIBRARY
    output = mraa_aio_read(adc_a1);
    #endif
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light_Sensor): output %d\n", output);
}
