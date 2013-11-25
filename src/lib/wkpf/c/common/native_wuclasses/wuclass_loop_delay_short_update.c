#include "debug.h"
#include "native_wuclasses.h"

int16_t delay_count_short=0;
bool over_short=false;

void wuclass_loop_delay_short_setup(wuobject_t *wuobject) {}

void wuclass_loop_delay_short_update(wuobject_t *wuobject) {
  int16_t input;
  int16_t delay;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_INPUT, &input);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_DELAY, &delay);
  
  if(delay_count_short>=delay && over_short==false) {
      over_short=true;
      wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_SHORT_OUTPUT, input);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: write %d to output \n", input);
  }
  else if(over_short==false) {
	  delay_count_short++;
	  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: delay %d, now count to %d\n", delay, delay_count_short);
  }
}
