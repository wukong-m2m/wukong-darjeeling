#include <stdio.h>
#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_led_setup(wuobject_t *wuobject) {
	// Just put a dummy value to make sure the file is created even if the object's not used in the FBP
	posix_property_put(wuobject, "led1", 0);
	posix_property_put(wuobject, "led2", 0);
	posix_property_put(wuobject, "led3", 0);
	posix_property_put(wuobject, "led4", 0);
}

void wuclass_led_update(wuobject_t *wuobject) {
	bool port1, port2, port3, port4;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT1, &port1);
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT2, &port2);
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT3, &port3);
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT4, &port4);
	posix_property_put(wuobject, "led1", port1);
	posix_property_put(wuobject, "led2", port2);
	posix_property_put(wuobject, "led3", port3);
	posix_property_put(wuobject, "led4", port4);
	printf("WKPFUPDATE(Fan): Setting leds to: %x %x %x %x\n", port1, port2, port3, port4);
}
