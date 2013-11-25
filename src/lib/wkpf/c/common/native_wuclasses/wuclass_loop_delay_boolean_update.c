#include "debug.h"
#include "native_wuclasses.h"

int16_t delay_count_boolean=0;
bool over_bool=false; 

void wuclass_loop_delay_boolean_setup(wuobject_t *wuobject) {}

void wuclass_loop_delay_boolean_update(wuobject_t *wuobject) {
  bool input;
  int16_t delay;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_INPUT, &input);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_DELAY, &delay);
  
  if(delay_count_boolean>=delay && over_bool==false ) {
      over_bool=true;
      wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_LOOP_DELAY_BOOLEAN_OUTPUT, input);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: write %d to output \n", input);
  }
  else if(over_bool==false) {
	  delay_count_boolean++;
	  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(loop_delay): Native loop_delay: delay %d, now count to %d\n", delay, delay_count_boolean);
  }
}
