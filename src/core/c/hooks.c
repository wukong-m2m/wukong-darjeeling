#include "hooks.h"
#include "types.h"
#include "debug.h"

void dj_hook_add(dj_hook **list, dj_hook *callback) {
	// Add this callback at the front of the linked list
	dj_hook *tmp = *list;
	while (tmp) {
		if (tmp == callback) {
			DEBUG_LOG(true, "dj_hook_add: %d already in list!", callback);
			return;
		}
		tmp = tmp->next;
	}

	callback->next = *list;
	*list = callback;
}

void inline dj_hook_call(dj_hook *list, void *data) {
	// Call each callback on the list
	dj_hook *callback = list;
	while (callback) {
		callback->function(data);
		callback = callback->next;
	}
}

