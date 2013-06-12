#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_THRESHOLD

void wuclass_threshold_setup(wuobject_t *wuobject) {}

void wuclass_threshold_update(wuobject_t *wuobject) {
  int16_t operator;
  int16_t threshold;
  int16_t value;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_OPERATOR, &operator);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_THRESHOLD, &threshold);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, &value);

	if (((operator == WKPF_ENUM_THRESHOLD_OPERATOR_GT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_GTE) && value > threshold)
	 || ((operator == WKPF_ENUM_THRESHOLD_OPERATOR_LT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value < threshold)
	 || ((operator == WKPF_ENUM_THRESHOLD_OPERATOR_GTE || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value == threshold)) {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, true);
    DEBUG_LOG(DBG_WKCOMM, "thr: operator %x thr %x value %x -> TRUE\n", operator, threshold, value);
  }
	else {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, false);
    DEBUG_LOG(DBG_WKCOMM, "thr: operator %x thr %x value %x -> FALSE\n", operator, threshold, value);
  }
}

#endif // ENABLE_WUCLASS_THRESHOLD
