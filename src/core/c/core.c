#include "core.h"
#include "types.h"
#include "hooks.h"
#include "heap.h"
#include "djtimer.h"

// Runlevel. Used to pause the VM when reprogramming and reset it afterwards.
uint8_t dj_exec_runlevel;

// For libraries that need frequent polling. Currently just for radios, but maybe there are other uses. Should be fast.
dj_hook *dj_core_pollingHook = NULL;

extern void dj_libraries_init(); // Generated during build based on the included libraries

void core_init(void *mem, uint32_t memsize) {
	// initialise timer
	dj_timer_init();

	// initialise memory managerw
	dj_mem_init(mem, memsize);

	// initialise libraries
	dj_libraries_init();
}
