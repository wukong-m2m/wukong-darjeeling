#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
#include "../../common/native_wuclasses/GENERATEDwuclass_pir_sensor.h"

#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PirSensor): setup\n");
  set_input(DDRH, 4);
  output_high(PINH, 4);
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
  bool currentValue = input_get(PINH, 4);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PirSensor): Sensed pir value: %d\n", currentValue);  
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, currentValue);  
}
