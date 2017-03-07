#include "config.h" // To get RADIO_USE_XBEE

#ifdef RADIO_USE_XBEE

#include "types.h"
#include "panic.h"
#include "radio_xbee.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../routing/routing.h"

void radio_xbee_init(void) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
}

radio_xbee_address_t radio_xbee_get_node_id() {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	return 1; // To keep the compiler happy.
}

void radio_xbee_poll(void) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
}

uint8_t radio_xbee_send(radio_xbee_address_t dest, uint8_t *payload, uint8_t length) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	return 1; // To keep the compiler happy.
}


#endif
