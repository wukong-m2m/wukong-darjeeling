#include "debug.h"
#include "native_wuclasses.h"
#include <avr/io.h>
#include "GENERATEDwuclass_binary_sensor.h"

#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

void wuclass_binary_sensor_setup(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(BinarySensor): setup\n");
  set_input(DDRE, 3);
  output_high(PINE, 3);
}

void wuclass_binary_sensor_update(wuobject_t *wuobject) {
  bool currentValue = 0;
  currentValue = input_get(PINE, 3);

  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_BINARY_SENSOR_CURRENT_VALUE, currentValue);

  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(BinarySensor): Sensed binary value: %d\n", currentValue);
  
}
