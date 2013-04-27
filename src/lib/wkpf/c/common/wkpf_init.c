#include "hooks.h"
#include "panic.h"
#include "heap.h"
#include "wkpf_gc.h"
#include "wkpf_comm.h"
#include "wkcomm.h"
#include "native_wuclasses/native_wuclasses.h"

dj_hook wkpf_markRootSetHook;
dj_hook wkpf_updatePointersHook;
dj_hook wkpf_comm_handleMessageHook;

void wkpf_init() {
	wkpf_markRootSetHook.function = wkpf_markRootSet;
	dj_hook_add(&dj_mem_markRootSetHook, &wkpf_markRootSetHook);

	wkpf_updatePointersHook.function = wkpf_updatePointers;
	dj_hook_add(&dj_mem_updateReferenceHook, &wkpf_updatePointersHook);

	wkpf_comm_handleMessageHook.function = wkpf_comm_handle_message;
	dj_hook_add(&wkcomm_handle_message_hook, &wkpf_comm_handleMessageHook);

	if (wkpf_native_wuclasses_init() != WKPF_OK)
		dj_panic(DJ_PANIC_OUT_OF_MEMORY);
}