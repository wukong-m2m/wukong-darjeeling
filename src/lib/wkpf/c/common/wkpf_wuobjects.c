#include <string.h>
#include "types.h"
#include "debug.h"
#include "heap.h"
#include "djtimer.h"
#include "wkpf.h"
#include "wkpf_wuobjects.h"
#include "wkpf_properties.h"
#include "wkpf_links.h"

wuobject_t *wuobjects_list = NULL;
uint16_t last_updated_wuobject_index = 0;
uint16_t last_propagated_property_wuobject_index = 0;

// Careful: this needs to match the IDs for the datatypes as defined in wkpf.h!
// The size is 1 for the status byte, plus the size of the property, so for instance a 16bit short takes up 3 bytes.
const uint8_t wkpf_property_datatype_size[3] = { 3, 2, 3 }; // Short, boolean, refreshrate
#define WKPF_GET_PROPERTY_DATASIZE(x)	 (wkpf_property_datatype_size[WKPF_GET_PROPERTY_DATATYPE(x)])

uint8_t wkpf_get_size_of_all_properties(wuclass_t *wuclass) {
	uint8_t size_of_properties = 0;
	for(int i=0; i<wuclass->number_of_properties; i++) {
		size_of_properties += WKPF_GET_PROPERTY_DATASIZE(wuclass->properties[i]);
	}
	return size_of_properties;
}

uint8_t wkpf_create_wuobject(uint16_t wuclass_id, uint8_t port_number, dj_object *java_instance_reference, bool called_from_wkpf_native_wuclasses_init) {
	wuobject_t *wuobject;
	if (wkpf_get_wuobject_by_port(port_number, &wuobject) != WKPF_ERR_WUOBJECT_NOT_FOUND) {
		DEBUG_LOG(DBG_WKPF, "WKPF: Port %d in use while creating wuobject for wuclass id %d: FAILED\n", port_number, wuclass_id);
		return WKPF_ERR_PORT_IN_USE;
	}

	// Find the wuclass definition for this wuclass_id
	wuclass_t *wuclass;
	uint8_t retval;
	retval = wkpf_get_wuclass_by_id(wuclass_id, &wuclass);
	if (retval != WKPF_OK)
		return retval;

	// Check if it's allowed for the application to create instances of this wuclass
	// but it is allowed when the profile framework is initialising to create wuobjects that represent HW
	// (called_from_wkpf_native_wuclasses_init==true)
	if (!(wuclass->flags & WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE) && !called_from_wkpf_native_wuclasses_init)
		return WKPF_ERR_CANT_CREATE_INSTANCE_OF_WUCLASS;

	// Check if an instance of the wuclass is provided if it's a virtual wuclass
	if (WKPF_IS_VIRTUAL_WUCLASS(wuclass) && java_instance_reference==0)
		return WKPF_ERR_NEED_VIRTUAL_WUCLASS_INSTANCE;

	// Allocate memory for the new wuobject
	uint16_t size;
	if (WKPF_IS_VIRTUAL_WUCLASS(wuclass))
		// Don't need to allocate memory for private C data for virtual wuclasses
		size = sizeof(wuobject_t) + wkpf_get_size_of_all_properties(wuclass);
	else
		size = sizeof(wuobject_t) + wkpf_get_size_of_all_properties(wuclass) + wuclass->private_c_data_size;
	dj_mem_addSafePointer((void**)&java_instance_reference); // dj_mem_alloc may cause GC to run, so the address of the wuclass and the virtual wuclass instance may change. this tells the GC to update our pointer if it does.
	dj_mem_addSafePointer((void**)&wuclass); // dj_mem_alloc may cause GC to run, so the address of the wuclass and the virtual wuclass instance may change. this tells the GC to update our pointer if it does.
	wuobject = (wuobject_t*)dj_mem_alloc(size, CHUNKID_WUCLASS);
	dj_mem_removeSafePointer((void**)&java_instance_reference);
	dj_mem_removeSafePointer((void**)&wuclass);
	if (wuobject == NULL) {
		DEBUG_LOG(DBG_WKPF, "WKPF: Out of memory while creating wuobject for wuclass %d at port %d: FAILED\n", wuclass_id, port_number);
		return WKPF_ERR_OUT_OF_MEMORY;
	}

	// Initialise memory (fixes a bug where old data corrupted the property status)
	memset(wuobject, 0, size);

	wuobject->wuclass = wuclass;
	wuobject->port_number = port_number;
	wuobject->java_instance_reference = java_instance_reference;
	wuobject->need_to_call_update = false;
	wuobject->next = wuobjects_list;
	wuobjects_list = wuobject;

	// Check if any properties need to pull their initial value from a remote node (properties that are the destination end of a link coming from another node)
	for(int i=0; i<wuclass->number_of_properties; i++) {
		if (wkpf_does_property_need_initialisation_pull(port_number, i)) {
			wuobject_property_t *property = wkpf_get_property(wuobject, i);
			wkpf_set_property_status_needs_pull(property);
			DEBUG_LOG(DBG_WKPF, "WKPF: Setting needs pull bit for property %d at port %d\n", i, port_number);
		}
	}

	if (!WKPF_IS_VIRTUAL_WUCLASS(wuclass))
		wuclass->setup(wuobject);

	DEBUG_LOG(DBG_WKPF, "WKPF: Created wuobject for wuclass id %d at port %d\n", wuclass_id, port_number);
	return WKPF_OK;
}

uint8_t wkpf_remove_wuobject(uint8_t port_number) {
	wuobject_t *wuobject;

	wuobject = wuobjects_list;
	if (wuobject && wuobject->port_number == port_number) {
		// It's the first in the list
		wuobjects_list = wuobjects_list->next;
		dj_mem_free(wuobject);
		return WKPF_OK;
	}

	while (wuobject) {
		if (wuobject->next && wuobject->next->port_number == port_number) {
			wuobject_t *nextnext = wuobject->next->next;
			dj_mem_free(wuobject->next);
			wuobject->next = nextnext;
			return WKPF_OK;
		}
	}

	DEBUG_LOG(DBG_WKPF, "WKPF: No wuobject at port %d found: FAILED\n", port_number);
	return WKPF_ERR_WUOBJECT_NOT_FOUND;  
}

uint8_t wkpf_get_wuobject_by_port(uint8_t port_number, wuobject_t **wuobject) {
	*wuobject = wuobjects_list;
	while (*wuobject) {
		if ((*wuobject)->port_number == port_number) {
			return WKPF_OK;
		}
		*wuobject = (*wuobject)->next;
	}
	DEBUG_LOG(DBG_WKPF, "WKPF: No wuobject at port %d found: FAILED\n", port_number);
	return WKPF_ERR_WUOBJECT_NOT_FOUND;
}

uint8_t wkpf_get_wuobject_by_index(uint8_t index, wuobject_t **wuobject) {
	*wuobject = wuobjects_list;
	while (index > 0 && wuobject != NULL) {
		index--;
		*wuobject = (*wuobject)->next;
	}
	if (*wuobject)
		return WKPF_OK;
	else
		return WKPF_ERR_WUOBJECT_NOT_FOUND;
}

uint8_t wkpf_get_wuobject_by_java_instance_reference(dj_object *java_instance_reference, wuobject_t **wuobject) {
	*wuobject = wuobjects_list;
	while (*wuobject) {
		if ((*wuobject)->java_instance_reference == java_instance_reference) {
			return WKPF_OK;
		}
		*wuobject = (*wuobject)->next;
	}
	DEBUG_LOG(DBG_WKPF, "WKPF: no wuobject for java object at %p found: FAILED\n", java_instance_reference);
	return WKPF_ERR_WUOBJECT_NOT_FOUND;
}

uint8_t wkpf_get_number_of_wuobjects() {
	int number_of_wuobjects = 0;
	wuobject_t *wuobject = wuobjects_list;
	while (wuobject) {
		number_of_wuobjects++;
		wuobject = wuobject->next;
	}
	return number_of_wuobjects;
}

void wkpf_set_need_to_call_update_for_wuobject(wuobject_t *wuobject) {
	// TODONR: for now just call directly for native wuclasses
	// Java update should be handled by returning from the WKPF.select() function
	if (WKPF_IS_NATIVE_WUOBJECT(wuobject))
		wuobject->wuclass->update(wuobject);
	else
		wuobject->need_to_call_update = true;
}

bool wkpf_get_next_wuobject_to_update(wuobject_t **virtual_wuobject) {
	if (wuobjects_list == NULL)
		return false;
	wuobject_t *wuobject;
	if (wkpf_get_wuobject_by_index(last_updated_wuobject_index, &wuobject) == WKPF_ERR_WUOBJECT_NOT_FOUND) { // Could happen if objects were deleted
		wuobject = wuobjects_list;
		last_updated_wuobject_index = 0;
	}
	uint16_t current_index = last_updated_wuobject_index;

	// wuobject is now pointing to the last updated object
	do {
		// Find the first next object that needs to be updated, or the same if there's no other.
		// Find next object
		wuobject = wuobject->next;
		current_index++;
		if (wuobject == NULL) {
			wuobject = wuobjects_list; // Wrap around to the first object.
			current_index = 0;
		}

		// Does it need to be updated?
		if ((wuobject->next_scheduled_update > 0 && wuobject->next_scheduled_update < dj_timer_getTimeMillis())
				|| wuobject->need_to_call_update) {
			// Update this object
			// Clear the flag if it was set
			wuobject->need_to_call_update = false;
			// If update has to be called because it's scheduled, schedule the next call
			if (wuobject->next_scheduled_update > 0 && wuobject->next_scheduled_update < dj_timer_getTimeMillis())
				wkpf_schedule_next_update_for_wuobject(wuobject);
			if (WKPF_IS_NATIVE_WUOBJECT(wuobject)) { // For native wuobjects: call update() directly
				// Mark wuobject as safe just in case the wuclass does something to trigger GC
				dj_mem_addSafePointer((void**)&wuobject);
				DEBUG_LOG(DBG_WKPF, "WKPF: Update native wuobject at port %d\n", wuobject->port_number);
				wuobject->wuclass->update(wuobject);
				dj_mem_removeSafePointer((void**)&wuobject);
			} else { // For virtual wuobject: return it so WKPF.select() can return it to Java
				*virtual_wuobject = wuobject;
				last_updated_wuobject_index = current_index;
				DEBUG_LOG(DBG_WKPF, "WKPF: Update virtual wuobject at port %d\n", wuobject->port_number);
				return true;
			}
		}
	} while(current_index != last_updated_wuobject_index);
	return false; // No Java wuobjects need to be updated
}

void wkpf_schedule_next_update_for_wuobject(wuobject_t *wuobject) {
	for (int i=0; i<wuobject->wuclass->number_of_properties; i++) {
		if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[i]) == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
			wkpf_refresh_rate_t refresh_rate;
			wkpf_internal_read_property_refresh_rate(wuobject, i, &refresh_rate);
			if (refresh_rate == 0) // 0 means turned off
				wuobject->next_scheduled_update = 0;
			else
				wuobject->next_scheduled_update = dj_timer_getTimeMillis() + refresh_rate;
			DEBUG_LOG(DBG_WKPF, "WKPF: Scheduled next update for object at port %d. Refresh rate:%d Current time:%lu Next update at:%lu\n", wuobject->port_number, refresh_rate, (unsigned long)dj_timer_getTimeMillis(), (unsigned long)wuobject->next_scheduled_update);
			return;
		}
	}
}

// This is here instead of in wkpf_properties so we can directly access the wuobjects list.
bool wkpf_get_next_dirty_property(wuobject_t **dirty_wuobject, uint8_t *dirty_property_number) {
	if (wuobjects_list == NULL)
		return false;
	wuobject_t *wuobject;
	if (wkpf_get_wuobject_by_index(last_propagated_property_wuobject_index, &wuobject) == WKPF_ERR_WUOBJECT_NOT_FOUND) { // Could happen if objects were deleted
		wuobject = wuobjects_list;
		last_propagated_property_wuobject_index = 0;
	}
	uint16_t current_index = last_propagated_property_wuobject_index;

	// wuobject is now pointing to the last checked object
	do {
		// Find the first next object that needs to be checked for dirty properties, or the same if there's no other.
		// Find next object
		wuobject = wuobject->next;
		current_index++;
		if (wuobject == NULL) {
			wuobject = wuobjects_list; // Wrap around to the first object.
			current_index = 0;
		}

		// Check if any property is dirty for this wuobject
		wuclass_t *wuclass = wuobject->wuclass;
		uint8_t offset = 0;
		for (int i=0; i<wuclass->number_of_properties; i++) {
			wuobject_property_t *property = (wuobject_property_t *)&(wuobject->properties_store[offset]);
			if (wkpf_property_status_is_dirty(property->status)) {
				// Found a dirty property. Return it.
				DEBUG_LOG(DBG_WKPF, "WKPF: wkpf_get_next_dirty_property DIRTY: port %d property %d status %d\n", wuobject->port_number, i, property->status);
				last_propagated_property_wuobject_index = current_index; // Next time continue from the next wuobject
				*dirty_wuobject = wuobject;
				*dirty_property_number = i;
				return true;
			}
			offset += WKPF_GET_PROPERTY_DATASIZE(wuclass->properties[i]);
		}
	} while(current_index != last_propagated_property_wuobject_index);
	return false; // No dirty properties found
}

wuobject_property_t* wkpf_get_property(wuobject_t *wuobject, uint8_t property_number) {
	wuclass_t *wuclass = wuobject->wuclass;
	uint8_t offset = 0;
	while(property_number > 0) {
		offset += WKPF_GET_PROPERTY_DATASIZE(wuclass->properties[--property_number]);
	}
	return (wuobject_property_t *)&(wuobject->properties_store[offset]);
}

void *wkpf_get_private_wuobject_data(wuobject_t *wuobject) {
	return ((void *)wuobject->properties_store) + wkpf_get_size_of_all_properties(wuobject->wuclass);
}
