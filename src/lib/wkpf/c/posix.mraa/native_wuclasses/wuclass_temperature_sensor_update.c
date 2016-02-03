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

mraa_aio_context adc_a2;

void wuclass_temperature_sensor_setup(wuobject_t *wuobject) {
    adc_a2 = mraa_aio_init(2);
}

void wuclass_temperature_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = mraa_aio_read(adc_a2);
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