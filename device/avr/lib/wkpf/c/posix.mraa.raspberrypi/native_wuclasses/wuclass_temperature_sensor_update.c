#include "config.h"
#ifdef GROVE_PI

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "grovepi.h"

#define TEMPERATURE_PIN 5

void wuclass_temperature_sensor_setup(wuobject_t *wuobject) {
    if(init() == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Temperature): init failed\n");
        return;
    }
}

void wuclass_temperature_sensor_update(wuobject_t *wuobject) {
    int16_t output = 0;
    output = analogRead(TEMPERATURE_PIN);
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