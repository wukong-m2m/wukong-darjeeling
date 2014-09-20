#include "types.h"
#include "debug.h"
#include "panic.h"
#include "djarchive.h"
#include "hooks.h"
#include "core.h"
#include "wkpf.h"
#include "wkpf_wuobjects.h"
#include "wkpf_links.h"

#include "config.h"
#ifndef HAS_WDT
#define platform_wdt_init()
#define platform_wdt_reset()
#endif
void wkpf_initLinkTableAndComponentMap(dj_di_pointer archive) {
	bool found_linktable = false, found_componentmap = false;
	for (uint8_t i=0; i<dj_archive_number_of_files(archive); i++) {
		dj_di_pointer file = dj_archive_get_file(archive, i);
		if (dj_archive_filetype(file) == DJ_FILETYPE_WKPF_LINK_TABLE) {
			DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Loading link table....\n");
			wkpf_load_links(file);
			found_linktable = true;
		}
		if (dj_archive_filetype(file) == DJ_FILETYPE_WKPF_COMPONENT_MAP) {
			DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Loading component map....\n");
			wkpf_load_component_to_wuobject_map(file);
			found_componentmap = true;
		}
	}
	if (!found_linktable || !found_componentmap)
		dj_panic(WKPF_PANIC_MISSING_BINARY_FILE);
}

void wkpf_initLocalObjectAndInitValues(dj_di_pointer archive) {
	DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Creating local native wuobjects....\n");
	if (wkpf_create_local_wuobjects_from_app_tables() != WKPF_OK)
		dj_panic(WKPF_PANIC_ERROR_CREATING_LOCAL_OBJECTS);

	bool found_initvalues = false;
	for (uint8_t i=0; i<dj_archive_number_of_files(archive); i++) {
		dj_di_pointer file = dj_archive_get_file(archive, i);
		if (dj_archive_filetype(file) == DJ_FILETYPE_WKPF_INITVALUES_TABLE) {
			DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Processing initvalues....\n");
			wkpf_process_initvalues_list(file);
			found_initvalues = true;
		}
	}
	if (!found_initvalues)
		dj_panic(WKPF_PANIC_MISSING_BINARY_FILE);
}

wuobject_t *wkpf_mainloop() {
	wuobject_t *wuobject = NULL;
	platform_wdt_init();
	while(true) {
		// Process any incoming messages
		platform_wdt_reset();
		dj_hook_call(dj_core_pollingHook, NULL);
		if (dj_exec_getRunlevel() == RUNLEVEL_RUNNING) {
			// Propagate any dirty properties
			wkpf_propagate_dirty_properties();
			// Check if any wuobjects need updates
			// Will call update() for native profiles directly,
			// and return only true for virtual profiles requiring an update.
			if(wkpf_get_next_wuobject_to_update(&wuobject)) {
				return wuobject;
			}
		}
	}
}

void picokong_init(dj_di_pointer archive) {
	wkpf_initLinkTableAndComponentMap(archive);
	wkpf_initLocalObjectAndInitValues(archive);
}
void wkpf_picokong(dj_di_pointer archive) {
	wkpf_initLinkTableAndComponentMap(archive);
	wkpf_initLocalObjectAndInitValues(archive);
	wkpf_mainloop(); // This will never return since picokong won't contain virtual wuobjects.
}
