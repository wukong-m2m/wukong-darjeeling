#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
#include "../../common/native_wuclasses/GENERATEDwuclass_binary_sensor.h"

#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

#define DEBOUNCE_THREASHOLD 500

void wuclass_binary_sensor_setup(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(BinarySensor): setup\n");
  set_input(DDRE, 3);
  output_high(PINE, 3);
}

void wuclass_binary_sensor_update(wuobject_t *wuobject) {
  bool currentValue = 0;
  currentValue = input_get(PINE, 3);

  short *delay;
  delay = wuclass_binary_sensor_getPrivateData(wuobject);

  bool status;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_BINARY_SENSOR_STATUS, &status);

  wkpf_refresh_rate_t refresh_rate;
  wkpf_internal_read_property_refresh_rate(wuobject, WKPF_PROPERTY_BINARY_SENSOR_REFRESH_RATE, &refresh_rate);

  int debounce;
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_BINARY_SENSOR_DEBOUNCE, &debounce);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(BinarySensor): Sensed binary value: %d %d %d %d %x\n", currentValue, *delay, debounce, status, PINE);  
#if 0
  if(debounce < DEBOUNCE_THREASHOLD) { // busy waiting for short debounce
      wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_BINARY_SENSOR_CURRENT_VALUE, &currentValue);
      if(currentValue != status) {
        
      }
      
      break;
    }
  } else {  // check sensed value in every update

    if(status == currentValue) {
      *delay += refresh_rate;
    } else {
      *delay = 0;
    }

    if(*delay > debounce) {
      *delay = 0;
      status = !status;
      wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_BINARY_SENSOR_STATUS, status);
    }
  }
#endif

  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(BinarySensor): Sensed binary value: %d\n", currentValue);
  DEBUG_LOG(DBG_WKPFUPDATE, "done %d\n", currentValue);
  
}
