#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>

void wuclass_fan_setup(wuobject_t *wuobject) {}

void wuclass_fan_update(wuobject_t *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_FAN_ON_OFF, &onOff);

  // Connect relay for the fan to port H bit 3, which is connected to JP6
  // SETOUPUT
  DDRH |= _BV(4);
  if (onOff)
    PORTH |= _BV(4);
  else
    PORTH &= ~_BV(4);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Fan): Setting fan to: %x\n", onOff);
}
