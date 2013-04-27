#include "types.h"
#include "debug.h"
#include "heap.h"
#include "wkpf.h"
#include "wkpf_wuclasses.h"

wuclass_t *wuclasses_list = NULL;

void wkpf_register_wuclass(wuclass_t *wuclass) {
  DEBUG_LOG(DBG_WKPF, "WKPF: Registering wuclass id %d at index %d\n", wuclass->wuclass_id, wkpf_get_number_of_wuclasses());
  wuclass->next = wuclasses_list;
  wuclasses_list = wuclass;
}

uint8_t wkpf_get_wuclass_by_id(uint16_t wuclass_id, wuclass_t **wuclass) {
	*wuclass = wuclasses_list;
	while (*wuclass) {
		if ((*wuclass)->wuclass_id == wuclass_id)
			return WKPF_OK;
		*wuclass = (*wuclass)->next;
	}
	DEBUG_LOG(DBG_WKPF, "WKPF: No wuclass with id %d found: FAILED\n", wuclass_id);
	return WKPF_ERR_WUCLASS_NOT_FOUND;
}

uint8_t wkpf_get_wuclass_by_index(uint8_t index, wuclass_t **wuclass) {
	*wuclass = wuclasses_list;
	while (index > 0 && wuclass != NULL) {
		index--;
		*wuclass = (*wuclass)->next;
	}
	if (*wuclass)
		return WKPF_OK;
	else
		return WKPF_ERR_WUCLASS_NOT_FOUND;
}

uint8_t wkpf_get_number_of_wuclasses() {
	int number_of_wuclasses = 0;
	wuclass_t *wuclass = wuclasses_list;
	while (wuclass) {
		number_of_wuclasses++;
		wuclass = wuclass->next;
	}
	return number_of_wuclasses;
}
