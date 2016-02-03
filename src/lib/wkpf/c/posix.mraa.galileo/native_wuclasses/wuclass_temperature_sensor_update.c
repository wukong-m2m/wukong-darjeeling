#include "config.h"
#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)

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
#include "IO_utils.h"

void wuclass_temperature_sensor_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    system("echo -n  > /sys/class/gpio/export");
    system("echo -n out > /sys/class/gpio/gpio36/direction");
    system("echo -n 0 > /sys/class/gpio/gpio36/value");
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure A2 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio53/value", F_OK ) == -1 ) {
      system("echo -n 53 > /sys/class/gpio/export");
    }
    system("echo -n out > /sys/class/gpio/gpio53/direction");
    system("echo -n strong > /sys/class/gpio/gpio53/drive");
    #endif
    #ifdef INTEL_EDISON
    //Configure A2 shield pin as an Analog input.
    if( access( "/sys/class/gpio/gpio202/value", F_OK ) == -1 ) {
      system("echo -n 202 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio234/value", F_OK ) == -1 ) {
      system("echo -n 234 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio210/value", F_OK ) == -1 ) {
      system("echo -n 210 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo high > /sys/class/gpio/gpio202/direction");
    system("echo low > /sys/class/gpio/gpio234/direction");
    system("echo in > /sys/class/gpio/gpio210/direction");
    system("echo high > /sys/class/gpio/gpio214/direction");
    #endif
}

void wuclass_temperature_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    #if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2)
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device0/in_voltage2_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    output = aio_read(buf);
    #endif
    #ifdef INTEL_EDISON
    int16_t fd=-1;
    char buf[4] = {'\\','\\','\\','\\'};
    fd = open("/sys/bus/iio/devices/iio:device1/in_voltage2_raw", O_RDONLY | O_NONBLOCK);
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 4);
    close(fd);
    system("echo high > /sys/class/gpio/gpio214/direction");
    output = aio_read(buf);
    #endif
    if(output == 0){
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Temperature): zero input\n");
    }else{
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Temperature_Sensor): Sensed raw value: %d\n", output);
      int16_t resistance = (4095 - output) * 10000 / output;
      DEBUG_LOG(DBG_WKPFUPDATE, "Resistance is %d\n", resistance);
      float temperature = 1/(((log(resistance/10000.0))/3975)+(1/298.15))-273.15;
      wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_SENSOR_CURRENT_VALUE, temperature);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Temperature): %f decree\n", temperature);
    }
}
#endif