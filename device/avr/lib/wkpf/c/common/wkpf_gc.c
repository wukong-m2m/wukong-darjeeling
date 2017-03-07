#include "heap.h"
#include "debug.h"
#include "wkpf_wuclasses.h"
#include "wkpf_wuobjects.h"

extern wuclass_t *wuclasses_list;
extern wuobject_t *wuobjects_list;

void wkpf_markRootSet(void *data) {
#ifdef DARJEELING_DEBUG
	dj_mem_dump();
#endif // DARJEELING_DEBUG
	// WuClasses
	DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Marking wuclasses black.\n");
	wuclass_t *wuclass = wuclasses_list;
	while (wuclass) {
		if (wuclass->update == NULL) { // Only for virtual classes, since native classes are global variables that aren't on the heap.
			dj_mem_setChunkColor(wuclass, TCM_BLACK);
		}
		wuclass = wuclass->next;
	}

	// WuObjects
	DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Marking wuobjects black, and java instances gray.\n");
	wuobject_t *wuobject = wuobjects_list;
	while (wuobject) {
		dj_mem_setChunkColor(wuobject, TCM_BLACK);
		if (wuobject->java_instance_reference)
			dj_mem_setChunkColor(wuobject->java_instance_reference, TCM_GRAY);
		// WuObject also containts a pointer to the wuclass, but that's already been taken care of above.
		wuobject = wuobject->next;
	}
}

void wkpf_updatePointers(void *data) {
	DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Updating references\n");

	// WuClasses
	wuclass_t **wuclass = &wuclasses_list;
	while (*wuclass) {
		wuclass_t **next = &(*wuclass)->next; // Store a pointer to this wuclass' next pointer
		if ((*wuclass)->update == NULL) { // Only for virtual classes, since native classes are global variables that aren't on the heap.
			DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Updating pointer for wuclass %d from %p to %p\n", (*wuclass)->wuclass_id, *wuclass, dj_mem_getUpdatedPointer(*wuclass));
			*wuclass = dj_mem_getUpdatedPointer(*wuclass); // Then update the pointer to this wuclass
		}
		wuclass = next; // Continue from the previously stored next pointer, since we can't access the wuclass itself anymore
	}

	wuobject_t **wuobject = &wuobjects_list;
	while (*wuobject) {
		// Print some debug output
		DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Updating pointer for wuobject on port %d from %p to %p\n", (*wuobject)->port_number, *wuobject, dj_mem_getUpdatedPointer(*wuobject));
		DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Updating pointer to wuclass for wuobject on port %d from %p to %p\n", (*wuobject)->port_number, (*wuobject)->wuclass, dj_mem_getUpdatedPointer((*wuobject)->wuclass));
		if ((*wuobject)->java_instance_reference) {
			DEBUG_LOG(DBG_WKPFGC, "WKPF: (GC) Updating pointer to java instance for wuobject on port %d from %p to %p\n", (*wuobject)->port_number, (*wuobject)->java_instance_reference, dj_mem_getUpdatedPointer((*wuobject)->java_instance_reference));
		}
		// Store a pointer to this wuobject' next pointer
		wuobject_t **next = &(*wuobject)->next;
		// Then update the pointers
		if ((*wuobject)->wuclass->update == NULL) { // Only for virtual classes, since native classes are global variables that aren't on the heap.
			(*wuobject)->wuclass = dj_mem_getUpdatedPointer((*wuobject)->wuclass);
		}
		(*wuobject)->java_instance_reference = dj_mem_getUpdatedPointer((*wuobject)->java_instance_reference);

		*wuobject = dj_mem_getUpdatedPointer(*wuobject);
		// Continue from the previously stored next pointer, since we can't access the wuobject itself anymore
		wuobject = next;
	}
}
