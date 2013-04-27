#include "hooks.h"
#include "heap.h"
#include "vm_gc.h"

dj_hook vm_markRootSetHook;
dj_hook vm_markObjectHook;
dj_hook vm_updatePointersHook;
dj_hook vm_postGCHook;

void vm_init() {
	vm_markRootSetHook.function = vm_mem_markRootSet;
	dj_hook_add(&dj_mem_markRootSetHook, &vm_markRootSetHook);

	vm_markObjectHook.function = vm_mem_markObject;
	dj_hook_add(&dj_mem_markObjectHook, &vm_markObjectHook);

	vm_updatePointersHook.function = vm_mem_updatePointers;
	dj_hook_add(&dj_mem_updateReferenceHook, &vm_updatePointersHook);

	vm_postGCHook.function = vm_mem_postGC;
	dj_hook_add(&dj_mem_postGCHook, &vm_postGCHook);
}

