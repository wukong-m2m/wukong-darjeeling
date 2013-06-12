#ifndef COREH
#define COREH

#include "types.h"
#include "hooks.h"

#define RUNLEVEL_RUNNING           1
#define RUNLEVEL_REPROGRAMMING     2
#define RUNLEVEL_PANIC             3 // All runlevels higher than this, as defined in panic.h, are panic runlevels.

// For libraries that need frequent polling. Currently just for radios, but maybe there are other uses. Should be fast.
extern dj_hook *dj_core_pollingHook;

extern uint8_t dj_exec_runlevel;
#define dj_exec_setRunlevel(runlevel)			(dj_exec_runlevel = runlevel)
#define dj_exec_getRunlevel()					(dj_exec_runlevel)

extern void core_init(void *mem, uint32_t memsize);

#endif // COREH