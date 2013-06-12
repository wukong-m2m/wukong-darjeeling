#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LOOP_DELAY_BOOLEAN

int16_t delay_count_boolean=0;

void wuclass_loop_delay_boolean_setup(wuobject_t *wuobject) {}

void wuclass_loop_delay_boolean_update(wuobject_t *wuobject) {
  bool input;
  bool output;
  int16_t delay;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_INPUT, &input);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_DELAY, &delay);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_OUTPUT, &output);
  
  if(delay_count_boolean>=delay) {
	  delay_count_boolean=0;
      wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_OUTPUT, input);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: write %x to output \n", input);
  }
  else if(output!=input) {
	  delay_count_boolean++;
	  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: delay %x, now count to %x\n", delay, delay_count_boolean);
  }
}


#endif
