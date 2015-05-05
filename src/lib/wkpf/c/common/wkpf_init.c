#include "hooks.h"
#include "panic.h"
#include "heap.h"
#include "wkpf_gc.h"
#include "wkpf_comm.h"
#include "wkcomm.h"
#include "native_wuclasses.h"
#ifdef LOAD_ENABLED_WUCLASSES_AT_STARTUP
#include "posix_utils.h"
#include "../posix/wkpf_process_enabled_wuclasses_xml.h"
#endif
// see issue 115 #include <avr/io.h>

dj_hook wkpf_markRootSetHook;
dj_hook wkpf_updatePointersHook;
dj_hook wkpf_comm_handleMessageHook;

#define output_low(port, pin) port &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define set_output(portdir, pin) portdir |= (1<<pin)

void led_init()
{
// see issue 115 	set_output(DDRK, 0);
// see issue 115 	set_output(DDRK, 1);
// see issue 115 	set_output(DDRK, 2);
// see issue 115 	set_output(DDRK, 3);
// see issue 115 	output_low(PORTK, 0);
// see issue 115 	output_low(PORTK, 1);
// see issue 115 	output_low(PORTK, 2);
// see issue 115 	output_low(PORTK, 3);
}

void wkpf_init() {
	led_init();
	wkpf_markRootSetHook.function = wkpf_markRootSet;
	dj_hook_add(&dj_mem_markRootSetHook, &wkpf_markRootSetHook);

	wkpf_updatePointersHook.function = wkpf_updatePointers;
	dj_hook_add(&dj_mem_updateReferenceHook, &wkpf_updatePointersHook);

	wkpf_comm_handleMessageHook.function = wkpf_comm_handle_message;
	dj_hook_add(&wkcomm_handle_message_hook, &wkpf_comm_handleMessageHook);

#ifdef LOAD_ENABLED_WUCLASSES_AT_STARTUP
	if (posix_enabled_wuclasses_xml != NULL) {
		if (wkpf_process_enabled_wuclasses_xml() != WKPF_OK)
			dj_panic(DJ_PANIC_OUT_OF_MEMORY);
	} else {
		if (wkpf_native_wuclasses_init() != WKPF_OK)
			dj_panic(DJ_PANIC_OUT_OF_MEMORY);		
	}
#else
	if (wkpf_native_wuclasses_init() != WKPF_OK)
		dj_panic(DJ_PANIC_OUT_OF_MEMORY);
#endif
}
