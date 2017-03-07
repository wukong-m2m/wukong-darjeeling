#include "hooks.h"
#include "core.h"
#include "wkcomm.h"
#include "routing/routing.h"

dj_hook wkcomm_pollingHook;

void wkcomm_init() {
	wkcomm_pollingHook.function = wkcomm_poll;
	dj_hook_add(&dj_core_pollingHook, &wkcomm_pollingHook);
	routing_init();
}
