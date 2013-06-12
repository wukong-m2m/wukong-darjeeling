#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_CONDITION_SELECTOR_BOOLEAN

void wuclass_condition_selector_boolean_setup(wuobject_t *wuobject) {}

void wuclass_condition_selector_boolean_update(wuobject_t *wuobject) {

  bool input;
  bool control;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_INPUT, &input);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_CONTROL, &control);
  
	if (control==false) {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT1, input);	
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT2, false);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(condition_selector_boolean): Native condition_selector_boolean: input %x control %x\n", input, control);
  }
	else {
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT1, false);
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT2, input);	
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(condition_selector_boolean): Native condition_selector_boolean: input %x control %x\n", input, control);
  }
}

#endif
