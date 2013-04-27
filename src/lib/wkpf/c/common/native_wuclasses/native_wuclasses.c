#include "debug.h"
#include "wkcomm.h"
#include "wkpf_config.h"
#include "wkpf_wuclasses.h"
#include "native_wuclasses.h"
#include "GENERATEDwuclass_generic.h"
#include "GENERATEDwuclass_threshold.h"
#include "GENERATEDwuclass_numeric_controller.h"
#include "GENERATEDwuclass_light_actuator.h"
#include "GENERATEDwuclass_light_sensor.h"
#include "GENERATEDwuclass_temperature_humidity_sensor.h"

uint8_t wkpf_register_wuclass_and_create_wuobject(wuclass_t *wuclass, uint8_t port_number) {
  wkpf_register_wuclass(wuclass);
  uint8_t retval = wkpf_create_wuobject(wuclass->wuclass_id, port_number, 0);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t wkpf_native_wuclasses_init() {
  uint8_t retval = WKPF_OK;

  retval = wkpf_register_wuclass_and_create_wuobject(&wuclass_generic, 0); // Always create wuobject for generic wuclass at port 0
  if (retval != WKPF_OK)
    return retval;

  DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Running wkpf native init for node id: %x\n", wkcomm_get_node_id());

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_LIGHT_SENSOR)) {
    wkpf_register_wuclass(&wuclass_light_sensor);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_sensor, 1);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_LIGHT_ACTUATOR)) {
    wkpf_register_wuclass(&wuclass_light_actuator);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_actuator, 2);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_NUMERIC_CONTROLLER)) {
    wkpf_register_wuclass(&wuclass_numeric_controller);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_numeric_controller, 3);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_NATIVE_THRESHOLD)) {
    wkpf_register_wuclass(&wuclass_threshold);
  }

  // if (wkpf_config_get_feature_enabled(WPKF_FEATURE_TEMPERATURE_HUMIDITY_SENSOR)) {
  //   wkpf_register_wuclass(&wuclass_temperature_humidity_sensor);
  //   /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_sensor, 1);*/
  //   if (retval != WKPF_OK)
  //     return retval;
  // }

  return WKPF_OK;
}
