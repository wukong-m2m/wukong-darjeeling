#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_CONDITION_SELECTOR_SHORT

void wuclass_condition_selector_short_setup(wuobject_t *wuobject) {}

void wuclass_condition_selector_short_update(wuobject_t *wuobject) {

  int16_t input;
  bool control;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_INPUT, &input);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_CONTROL, &control);
  
	if (control==false) {
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT1, input);	
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT2, 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(condition_selector_short): Native condition_selector_short: input %x control %x\n", input, control);
  }
	else {
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT1, 0);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT2, input);	
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(condition_selector_short): Native condition_selector_short: input %x control %x\n", input, control);
  }
}

#endif
