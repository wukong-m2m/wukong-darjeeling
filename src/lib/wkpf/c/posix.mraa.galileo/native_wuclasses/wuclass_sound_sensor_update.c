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

#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)
#include "IO_utils.h"
void wuclass_sound_sensor_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    system("echo -n 22 > /sys/class/gpio/export");
    system("echo -n out > /sys/class/gpio/gpio22/direction");
    system("echo -n 0 > /sys/class/gpio/gpio22/value");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure A3 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio55/value", F_OK ) == -1 ) {
      system("echo -n 55 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio55/direction");
    system("echo -n strong > /sys/class/gpio/gpio55/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure A3 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio203/value", F_OK ) == -1 ) {
      system("echo -n 203 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio235/value", F_OK ) == -1 ) {
      system("echo -n 235 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio211/value", F_OK ) == -1 ) {
      system("echo -n 211 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo high > /sys/class/gpio/gpio203/direction");
    system("echo low > /sys/class/gpio/gpio235/direction");
    system("echo in > /sys/class/gpio/gpio211/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_sound_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    #if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2)
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    output = aio_read(buf);
    #endif
    #ifdef INTEL_EDISON
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device1/in_voltage3_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    system("echo high > /sys/class/gpio/gpio214/direction");
    output = aio_read(buf);
    #endif
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): raw value %d\n", output);
    output = (int16_t)((output/4095.0)*255);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SOUND_SENSOR_CURRENT_VALUE, output);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Sound_Sensor): output %d\n", output);
}
#endif