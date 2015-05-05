#include "debug.h"
#include "native_wuclasses.h"
#include <avr/io.h>

void wuclass_light_actuator_setup(wuobject_t *wuobject) {}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);

  // Connect light to port B, bit 4. This maps to pin 3 of JP18 on the WuNode (pin 1 is behind the power connector)
  // Port B, bit 5 (pin 4 on JP18) will reversed: low for on, high for off, for connecting LEDs
  // SETOUPUT
  DDRB |= _BV(4);
  DDRB |= _BV(5);
  DDRB |= _BV(7);
  if (onOff)
    PORTB |= _BV(4);
  else
    PORTB &= ~_BV(4);
  if (onOff)
    PORTB &= ~_BV(5);
  else
    PORTB |= _BV(5);
  if (onOff)
    PORTB |= _BV(7);
  else
    PORTB &= ~_BV(7);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}
