#include <debug.h>
#include "wkcomm.h"
#include "wkpf_config.h"
#include "wkpf_wuclasses.h"
#include "native_wuclasses.h"
#include "GENERATEDwuclass_generic.h"
#include "GENERATEDwuclass_threshold.h"
#include "GENERATEDwuclass_light_sensor.h"
#include "GENERATEDwuclass_temperature_humidity_sensor.h"
#include "GENERATEDwuclass_led.h"
#include "GENERATEDwuclass_light_actuator.h"
#include "GENERATEDwuclass_numeric_controller.h"

uint8_t wkpf_register_wuclass_and_create_wuobject(wuclass_t *wuclass, uint8_t port_number) {
  wkpf_register_wuclass(wuclass);
  uint8_t retval = wkpf_create_wuobject(wuclass->wuclass_id, port_number, 0);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t wkpf_native_wuclasses_init() {
  uint8_t retval;

  retval = wkpf_register_wuclass_and_create_wuobject(&wuclass_generic, 0); // Always create wuobject for generic wuclass at port 0
  if (retval != WKPF_OK)
    return retval;

  DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Running wkpf native init for node id: %x\n", wkcomm_get_node_id());

  wkpf_register_wuclass(&wuclass_threshold);
  if (retval != WKPF_OK)
    return retval;

  wkpf_register_wuclass(&wuclass_light_sensor);
  if (retval != WKPF_OK)
    return retval;

  wkpf_register_wuclass(&wuclass_light_actuator);
  if (retval != WKPF_OK)
    return retval;

  wkpf_register_wuclass(&wuclass_numeric_controller);
  if (retval != WKPF_OK)
    return retval;

  wkpf_register_wuclass(&wuclass_led);
  if (retval != WKPF_OK)
    return retval;

  return WKPF_OK;
}
