#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LOOP_DELAY_SHORT

int16_t delay_count_short=0;

void wuclass_loop_delay_short_setup(wuobject_t *wuobject) {}

void wuclass_loop_delay_short_update(wuobject_t *wuobject) {
  int16_t input;
  int16_t output;
  int16_t delay;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_INPUT, &input);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_DELAY, &delay);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_OUTPUT, &output);
  
  if(delay_count_short>=delay) {
	  delay_count_short=0;
      wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_OUTPUT, input);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: write %x to output \n", input);
  }
  else if(output!=input) {
	  delay_count_short++;
	  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: delay %x, now count to %x\n", delay, delay_count_short);
  }
}

#endif
