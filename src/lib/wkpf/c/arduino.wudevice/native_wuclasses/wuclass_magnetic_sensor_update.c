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
  int16_t pin;
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor): setup\n");

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR___PIN, &pin);
  switch (pin) {
    case WKPF_ENUM_PIN_PIN1:
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor,%d): setup using pin JP5\n", pin);
      set_input(DDRE, 3);
      output_high(PINE, 3);
    break;
    case WKPF_ENUM_PIN_PIN2:
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor,%d): setup using pin JP6\n", pin);
      set_input(DDRH, 3);
      output_high(PINH, 3);
    break;
    default:
      dj_panic(WKPF_PANIC_PIN_NOT_SUPPORTED);
  }
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
  int16_t pin;
  bool currentValue;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR___PIN, &pin);
  switch (pin) {
    case WKPF_ENUM_PIN_PIN1:
      currentValue = input_get(PINE, 3);
    break;
    case WKPF_ENUM_PIN_PIN2:
      currentValue = input_get(PINH, 3);
    break;
    default:
      currentValue = 0; // keeps the compiler happy
      dj_panic(WKPF_PANIC_PIN_NOT_SUPPORTED);
  }

  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor,%d): Sensed binary value: %d\n", pin, currentValue);  
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, currentValue);  
}
