#include "types.h"
#include "hooks.h"
#include "heap.h"
#include "wkcomm.h"
#include "wkreprog_comm.h"
#include "wkreprog_gc.h"

dj_hook wkreprog_comm_handleMessageHook;
dj_hook wkreprog_gc_markRootSetHook;
dj_hook wkreprog_gc_updatePointersHook;

void wkreprog_init() {
	wkreprog_comm_handleMessageHook.function = wkreprog_comm_handle_message;
	dj_hook_add(&wkcomm_handle_message_hook, &wkreprog_comm_handleMessageHook);

	wkreprog_gc_markRootSetHook.function = wkreprog_mem_markRootSet;
	dj_hook_add(&dj_mem_markRootSetHook, &wkreprog_gc_markRootSetHook);

	wkreprog_gc_updatePointersHook.function = wkreprog_mem_updatePointers;
	dj_hook_add(&dj_mem_updateReferenceHook, &wkreprog_gc_updatePointersHook);
}
