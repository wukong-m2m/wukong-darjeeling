#ifndef VM_GCH
#define VM_GCH

extern void vm_mem_setPanicExceptionObject(dj_object *obj);
extern dj_object * vm_mem_getPanicExceptionObject();

// Hooks
extern void vm_mem_markRootSet(void *data);
extern void vm_mem_updatePointers(void *data);
extern void vm_mem_markObject(void *data);
extern void vm_mem_postGC(void *data);

#endif // VM_GCH