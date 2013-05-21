#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"

#include <avr/io.h>
#include "../../../../uart/c/common/uart.h"
#include "wkcomm.h"

#ifdef ENABLE_WUCLASS_GH_SENSOR

void wuclass_gh_sensor_setup(wuobject_t *wuobject) {
	uart_inituart(3, 9600);
      DEBUG_LOG(DBG_WKPFGH, "transimit wunode id: %d\n", wkcomm_get_node_id());
        uart_write_byte(3, 4);
	uart_write_byte(3, wkcomm_get_node_id());//send wunode id to oct
}

void wuclass_gh_sensor_update(wuobject_t *wuobject) {

  bool enable;
  uint8_t state;

    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_GH_SENSOR_ENABLE, &enable);
    if(enable == true){
      uart_write_byte(3, 2);
    }
    else
      uart_write_byte(3, 3);
    
    //use get state command to check the state has been changed or not
    uart_write_byte(3, 1);
    while(true){
      state = uart_read_byte(3);
      if (state != 0) break;
    }

    if((state == 5 && enable == true) || (state == 6 && enable != true)){
      DEBUG_LOG(DBG_WKPFGH, "change state success\n");
    }
    else
      DEBUG_LOG(DBG_WKPFGH, "change state fail\n");
}

#endif
