#include "debug.h"
#include "panic.h"
#include "native_wuclasses.h"
#include <avr/io.h>
#include "GENERATEDwuclass_magnetic_sensor.h"

#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

// PIN1: JP5. port E, pin 3
// PIN2: JP6. port H, pin 3

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor): setup\n");
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
  bool currentValue = true;

  DDRB &= ~(1<<3);
  PORTB |= (1<<3);
  //PORTD = 0xff;
  currentValue = (PINB&_BV(3));
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor): Sensed binary value: %d %d %d\n", currentValue, PORTB, DDRB);  
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, currentValue);  
}
