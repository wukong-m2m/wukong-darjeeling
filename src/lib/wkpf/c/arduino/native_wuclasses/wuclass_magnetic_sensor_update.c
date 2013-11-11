#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
#include "../../common/native_wuclasses/GENERATEDwuclass_magnetic_sensor.h"

#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

#ifdef ENABLE_WUCLASS_MAGNETIC_SENSOR

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
  DEBUG_LOG(true, "WKPFUPDATE(MagneticSensor): setup\n");
  set_input(DDRE, 3);
  output_high(PINE, 3);
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
  bool currentValue = input_get(PINE, 3);
  DEBUG_LOG(true, "WKPFUPDATE(MagneticSensor): Sensed binary value: %d\n", currentValue);  
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, currentValue);  
}

#endif // ENABLE_WUCLASS_MAGNETIC_SENSOR
