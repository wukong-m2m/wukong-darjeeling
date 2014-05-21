#include "types.h"
#include "program_mem.h"
#include "debug.h"
#include "djarchive.h"
#include "panic.h"
#include "wkcomm.h"
#include "wkreprog.h"
#include "wkpf.h"
#include "wkpf_wuobjects.h"
#include "wkpf_properties.h"
#include "wkpf_comm.h"
#include "wkpf_links.h"
#include "wkpf_wuclasses.h"
#include "wkpf_wuobjects.h"


dj_di_pointer wkpf_links_store = 0;
dj_di_pointer wkpf_component_map_store = 0;
uint16_t wkpf_number_of_links = 0; // To be set when we load the table
uint16_t wkpf_number_of_components = 0; // To be set when we load the map

// Link table format
// 2 bytes: number of links
// Links:
//		2 byte little endian src component id
//		1 byte src port number
//		2 byte little endian dest component id
//		1 byte dest port number
#define WKPF_LINK_ENTRY_SIZE								6
#define WKPF_LINK_SRC_COMPONENT_ID(i)						(dj_di_getU16(wkpf_links_store + 2 + WKPF_LINK_ENTRY_SIZE*i))
#define WKPF_LINK_SRC_PROPERTY(i)							(dj_di_getU8(wkpf_links_store + 2 + WKPF_LINK_ENTRY_SIZE*i + 2))
#define WKPF_LINK_DEST_COMPONENT_ID(i)						(dj_di_getU16(wkpf_links_store + 2 + WKPF_LINK_ENTRY_SIZE*i + 3))
#define WKPF_LINK_DEST_PROPERTY(i)							(dj_di_getU8(wkpf_links_store + 2 + WKPF_LINK_ENTRY_SIZE*i + 5))
// TODONR: refactor
#define WKPF_LINK_DEST_WUCLASS_ID(i)						(WKPF_COMPONENT_WUCLASS_ID(WKPF_LINK_DEST_COMPONENT_ID(i)))

// Component map format
// 2 bytes little endian number of components
// Per component:
//		2 bytes little endian offset
// Per component @ component offset:
// 		1 byte little endian number of endpoints
//		2 bytes wuclass id
//		Per endpoint
//			1 byte node address
//			1 byte port number
#define WKPF_COMPONENT_ADDRESS(i)							((dj_di_pointer)(wkpf_component_map_store + dj_di_getU16(wkpf_component_map_store + 2 + 2*i)))
#define WKPF_NUMBER_OF_ENDPOINTS(i)							(dj_di_getU8(WKPF_COMPONENT_ADDRESS(i)))
#define WKPF_COMPONENT_WUCLASS_ID(i)						(dj_di_getU16(WKPF_COMPONENT_ADDRESS(i) + 1))
#define WKPF_COMPONENT_ENDPOINT_NODE_ID(i, j)				(dj_di_getU16(WKPF_COMPONENT_ADDRESS(i) + 3 + 3*j))
#define WKPF_COMPONENT_ENDPOINT_PORT(i, j)					(dj_di_getU8(WKPF_COMPONENT_ADDRESS(i) + 3 + 3*j + 2))
#define WKPF_COMPONENT_LEADER_ENDPOINT_NODE_ID(i)			(WKPF_COMPONENT_ENDPOINT_NODE_ID(i, 0))
#define WKPF_COMPONENT_LEADER_ENDPOINT_PORT(i)				(WKPF_COMPONENT_ENDPOINT_PORT(i, 0))

bool wkpf_get_component_id(uint8_t port_number, uint16_t *component_id) {
	for(int i=0; i<wkpf_number_of_components; i++) {
		for(int j=0; j<WKPF_NUMBER_OF_ENDPOINTS(i); j++) {
			if(WKPF_COMPONENT_ENDPOINT_NODE_ID(i, j) == wkcomm_get_node_id()
					&& WKPF_COMPONENT_ENDPOINT_PORT(i, j) == port_number) {
				*component_id = i;
				return true; // Found
			}
		}
	}
	return false; // Not found. Could happen for wuobjects that aren't used in the application (unused sensors, actuators, etc).
}

bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number) {
	uint16_t component_id;
	wkpf_get_component_id(port_number, &component_id);

	for(int i=0; i<wkpf_number_of_links; i++) {
		if(WKPF_LINK_DEST_PROPERTY(i) == property_number
				&& WKPF_LINK_DEST_COMPONENT_ID(i) == component_id) {
			// The property is the destination of this link. If the source is remote, we need to ask for an initial value
			if (wkpf_node_is_leader(WKPF_LINK_SRC_COMPONENT_ID(i), wkcomm_get_node_id())) {
				DEBUG_LOG(DBG_WKPF, "%x, %x doesn't need pull: source is a local property\n", port_number, property_number);
				return false; // Source link is local, so no need to pull initial value as it will come automatically.
			} else {
				DEBUG_LOG(DBG_WKPF, "%x, %x needs initialisation pull\n", port_number, property_number);
				return true; // There is a link to this property, coming from another node. We need to ask it for the initial value.
			}
		}
	}
	DEBUG_LOG(DBG_WKPF, "%x, %x doesn't need pull: not a destination property\n", port_number, property_number);
	return false; // This wuobject isn't used in the application.
}

uint8_t wkpf_pull_property(uint8_t port_number, uint8_t property_number) {
	uint16_t component_id;
	wkpf_get_component_id(port_number, &component_id);

	for(int i=0; i<wkpf_number_of_links; i++) {
		if(WKPF_LINK_DEST_PROPERTY(i) == property_number
				&& WKPF_LINK_DEST_COMPONENT_ID(i) == component_id) {
			uint16_t src_component_id = WKPF_LINK_SRC_COMPONENT_ID(i);
			uint8_t src_property_number = WKPF_LINK_SRC_PROPERTY(i);
			wkcomm_address_t src_endpoint_node_id;
			if ((src_endpoint_node_id = WKPF_COMPONENT_LEADER_ENDPOINT_NODE_ID(src_component_id)) != wkcomm_get_node_id()) {
				uint8_t src_endpoint_port = WKPF_COMPONENT_LEADER_ENDPOINT_PORT(src_component_id);
				// Properties with local sources will be initialised eventually, so we only need to send a message
				// to ask for initial values coming from remote nodes
				return wkpf_send_request_property_init(src_endpoint_node_id, src_endpoint_port, src_property_number);      
			}
		}
	}
	return WKPF_ERR_SHOULDNT_HAPPEN;
}

uint8_t wkpf_propagate_property(wuobject_t *wuobject, uint8_t property_number, void *value) {
	uint8_t port_number = wuobject->port_number;
	uint16_t component_id;
	if (!wkpf_get_component_id(port_number, &component_id))
		return WKPF_OK; // WuObject isn't used in the application.

	wuobject_t *src_wuobject;
	uint8_t wkpf_error_code = 0;

	DEBUG_LOG(DBG_WKPF, "WKPF: propagate property number %x of component %x on port %x (value %x)\n", property_number, component_id, port_number, *((uint16_t *)value)); // TODONR: values other than 16 bit values

	wkpf_get_wuobject_by_port(port_number, &src_wuobject);
	for(int i=0; i<wkpf_number_of_links; i++) {
		if(WKPF_LINK_SRC_PROPERTY(i) == property_number
				&& WKPF_LINK_SRC_COMPONENT_ID(i) == component_id) {
			uint16_t dest_component_id = WKPF_LINK_DEST_COMPONENT_ID(i);
			uint8_t dest_property_number = WKPF_LINK_DEST_PROPERTY(i);
			uint16_t dest_wuclass_id = WKPF_LINK_DEST_WUCLASS_ID(i);
			wkcomm_address_t dest_node_id = WKPF_COMPONENT_LEADER_ENDPOINT_NODE_ID(dest_component_id);
			uint8_t dest_port_number = WKPF_COMPONENT_LEADER_ENDPOINT_PORT(dest_component_id);
			if (dest_node_id == wkcomm_get_node_id()) {
				// Local
				wuobject_t *dest_wuobject;
        uint8_t wkpf_wuobject_error_code = 0;
				wkpf_wuobject_error_code = wkpf_get_wuobject_by_port(dest_port_number, &dest_wuobject);
				if (wkpf_wuobject_error_code == WKPF_OK) {
					DEBUG_LOG(DBG_WKPF, "WKPF: propagate_property (local). (%x, %x)->(%x, %x), value %x\n", port_number, property_number, dest_port_number, dest_property_number, *((uint16_t *)value)); // TODONR: values other than 16 bit values
					if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN)
						wkpf_error_code |= wkpf_external_write_property_boolean(dest_wuobject, dest_property_number, *((bool *)value));
					else if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT)
						wkpf_error_code |= wkpf_external_write_property_int16(dest_wuobject, dest_property_number, *((uint16_t *)value));
					else
						wkpf_error_code |= wkpf_external_write_property_refresh_rate(dest_wuobject, dest_property_number, *((uint16_t *)value));
				}
			} else {
				// Remote
				DEBUG_LOG(DBG_WKPF, "WKPF: propagate_property (remote). (%x, %x)->(%x, %x, %x), value %x\n", port_number, property_number, dest_node_id, dest_port_number, dest_property_number, *((uint16_t *)value)); // TODONR: values other than 16 bit values
				if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN)
					wkpf_error_code |= wkpf_send_set_property_boolean(dest_node_id, dest_port_number, dest_property_number, dest_wuclass_id, *((bool *)value));
				else if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT)
					wkpf_error_code |= wkpf_send_set_property_int16(dest_node_id, dest_port_number, dest_property_number, dest_wuclass_id, *((uint16_t *)value));
				else
					wkpf_error_code |= wkpf_send_set_property_refresh_rate(dest_node_id, dest_port_number, dest_property_number, dest_wuclass_id, *((uint16_t *)value));
			}
			/*if (wkpf_error_code != WKPF_OK)*/
				/*return wkpf_error_code;*/
		}
	}
	return wkpf_error_code;
}

uint8_t wkpf_propagate_dirty_properties() {
	uint8_t wkpf_error_code;
	wuobject_t *dirty_wuobject;
	uint8_t dirty_property_number;

	while (wkpf_get_next_dirty_property(&dirty_wuobject, &dirty_property_number)) {
		// TODONR: comm
		// nvmcomm_poll(); // Process incoming messages
		wuobject_property_t *dirty_property = wkpf_get_property(dirty_wuobject, dirty_property_number);
		if (dirty_property->status & PROPERTY_STATUS_NEEDS_PUSH) {
			wkpf_error_code = wkpf_propagate_property(dirty_wuobject, dirty_property_number, &(dirty_property->value));
		} else { // PROPERTY_STATUS_NEEDS_PULL
			DEBUG_LOG(DBG_WKPF, "WKPF: (pull) requesting initial value for property %x at port %x\n", dirty_property_number, dirty_wuobject->port_number);
			wkpf_error_code = wkpf_pull_property(dirty_wuobject->port_number, dirty_property_number);
		}
		if (wkpf_error_code == WKPF_OK) {
			wkpf_propagating_dirty_property_succeeded(dirty_property);
		} else { // TODONR: need better retry mechanism
			DEBUG_LOG(DBG_WKPF, "WKPF: ------!!!------ Propagating property failed: port %x property %x error %x\n", dirty_wuobject->port_number, dirty_property_number, wkpf_error_code);
			wkpf_propagating_dirty_property_failed(dirty_property);
			return wkpf_error_code;
		}
	}
	return WKPF_OK;
}

// TODONR: proper definition for this function.
uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, wkcomm_address_t *node_id, uint8_t *port_number) {
	if (component_id > wkpf_number_of_components)
		return WKPF_ERR_COMPONENT_NOT_FOUND;
	*node_id = WKPF_COMPONENT_ENDPOINT_NODE_ID(component_id, 0);
	*port_number = WKPF_COMPONENT_ENDPOINT_PORT(component_id, 0);
	return WKPF_OK;
}

bool wkpf_node_is_leader(uint16_t component_id, wkcomm_address_t node_id) {
	return WKPF_COMPONENT_LEADER_ENDPOINT_NODE_ID(component_id) == node_id;
}


// Initialisation code called from WKPF.appInit().
uint8_t wkpf_load_component_to_wuobject_map(dj_di_pointer map) {
	wkpf_component_map_store = map;
	wkpf_number_of_components = dj_di_getU16(wkpf_component_map_store);

	// After storing the reference, only use the constants defined above to access it so that we may change the storage implementation later
	DEBUG_LOG(DBG_WKPF, "WKPF: Registering %x components\n", wkpf_number_of_components);
	for (uint16_t i=0; i<wkpf_number_of_components; i++) {
		DEBUG_LOG(DBG_WKPF, "WKPF: Component %d, %d endpoints -> ", i, WKPF_NUMBER_OF_ENDPOINTS(i));
		for (uint8_t j=0; j<WKPF_NUMBER_OF_ENDPOINTS(i); j++) {
			DEBUG_LOG(DBG_WKPF, "  (node %d, port %d)", WKPF_COMPONENT_ENDPOINT_NODE_ID(i, j), WKPF_COMPONENT_ENDPOINT_PORT(i, j));
			if (WKPF_COMPONENT_ENDPOINT_NODE_ID(i, j) == wkcomm_get_node_id()) {
				wuobject_t *wuobject;
				if (wkpf_get_wuobject_by_port(WKPF_COMPONENT_ENDPOINT_PORT(i, j), &wuobject) == WKPF_OK) {
					// This is a local component, and it's wuobject already exists. This means it's a hardware device, and the
					// application won't need to create an object for it. It may have an incoming link from another device though,
					// so in that case we need to set the pull bit to ask the remote device for the initial value.
					wkpf_set_request_property_init_where_necessary(wuobject);
				}
			}
		}
		DEBUG_LOG(DBG_WKPF, "\n");
	}

// // TODONR: nieuwe constante bedenken en implementatie van group_add_node_to_watch en wkcomm_get_node_id
// #ifdef NVM_USE_GROUP
// 	for (int i=0; i<wkpf_number_of_components; i++) {
// 		wkpf_component_t *component = wkpf_get_component(i);
// 		for (int j=0; j<WKPF_NUMBER_OF_ENDPOINTS(component); j++) {
// 			wkpf_endpoint_t *endpoint = wkpf_get_endpoint_for_component(component, j);
// 			if (endpoint->node_id == wkcomm_get_node_id()) {
// 				if (j == 0) {
// 					// I'm the leader, so watch everyone
// 					for (int k=1; k<WKPF_NUMBER_OF_ENDPOINTS(component); k++)
// 						group_add_node_to_watch(wkpf_get_endpoint_for_component(component, k)->node_id);
// 				} else {
// 					// Just watch the leader
// 					group_add_node_to_watch(wkpf_get_endpoint_for_component(component, 0)->node_id);
// 				}
// 			}
// 		}
// 	}
// #endif // NVM_USE_GROUP
	return WKPF_OK;
}

uint8_t wkpf_load_links(dj_di_pointer links) {
	// This works on AVR and x86 since they're both little endian. To port WKPF to a big endian
	// platform we would need to do some swapping.
	wkpf_links_store = links;
	wkpf_number_of_links =  dj_di_getU16(wkpf_links_store);
	// After storing the reference, only use the constants defined above to access it so that we may change the storage implementation later

	DEBUG_LOG(DBG_WKPF, "WKPF: Registering %d links\n", (int)wkpf_number_of_links); // Need a cast here because the type may differ depending on architecture.
#ifdef DARJEELING_DEBUG
	for (int i=0; i<wkpf_number_of_links; i++) {
		DEBUG_LOG(DBG_WKPF, "WKPF: Link from (%d, %d) to (%d, %d)\n", WKPF_LINK_SRC_COMPONENT_ID(i), WKPF_LINK_SRC_PROPERTY(i), WKPF_LINK_DEST_COMPONENT_ID(i), WKPF_LINK_DEST_PROPERTY(i));
	}
#endif // DARJEELING_DEBUG
	return WKPF_OK;
}

uint8_t wkpf_create_local_wuobjects_from_app_tables() {
	uint8_t wkpf_error_code;
	for (uint16_t i=0; i<wkpf_number_of_components; i++) {
		for (uint8_t j=0; j<WKPF_NUMBER_OF_ENDPOINTS(i); j++) {
			if (WKPF_COMPONENT_ENDPOINT_NODE_ID(i, j) == wkcomm_get_node_id()) {
				// This is a local component, so we need to create an instance if it's a native wuclass
				// I'm still letting the virtual wuclasses be created by the Java code, since this won't
				// necessary for picokong
				wuclass_t *wuclass;
				wkpf_error_code = wkpf_get_wuclass_by_id(WKPF_COMPONENT_WUCLASS_ID(i), &wuclass);
				if (wkpf_error_code != WKPF_OK)
					return wkpf_error_code;
				if (WKPF_IS_VIRTUAL_WUCLASS(wuclass)) {
					// These will be created in Java.
					continue;
				}
				wuobject_t *wuobject;
				if (wkpf_get_wuobject_by_port(WKPF_COMPONENT_ENDPOINT_PORT(i, j), &wuobject) != WKPF_ERR_WUOBJECT_NOT_FOUND) {
					// There's already an object here. This is the case for hard wuobjects that have
					// their instance created in native_wuclasses_init.
					// In that case we can skip it here, but let's check the wuclass id just to be safe
					if (wuobject->wuclass == wuclass)
						continue; // Ok, this is just a hardware component, so we don't need to create it
					else
						return WKPF_ERR_PORT_IN_USE; // This is bad: the port is already in use by an object of different type
				}
				wkpf_error_code = wkpf_create_wuobject(wuclass->wuclass_id, WKPF_COMPONENT_ENDPOINT_PORT(i, j), NULL, false);
				if (wkpf_error_code != WKPF_OK)
					return wkpf_error_code;
			}
		}
	}
	return WKPF_OK;
}

uint8_t wkpf_process_initvalues_list(dj_di_pointer initvalues) {
	uint8_t wkpf_error_code;
	uint16_t number_of_initvalues = dj_di_getU16(initvalues);
	initvalues += 2; // Skip number of values
	for (uint16_t i=0; i<number_of_initvalues; i++) {
		uint16_t component_id = dj_di_getU16(initvalues);
		initvalues += 2;
		uint8_t property_number = dj_di_getU8(initvalues);
		initvalues += 1;
		uint8_t value_size = dj_di_getU8(initvalues);
		initvalues += 1;

		for (uint8_t j=0; j<WKPF_NUMBER_OF_ENDPOINTS(component_id); j++) {
			if (WKPF_COMPONENT_ENDPOINT_NODE_ID(component_id, j) == wkcomm_get_node_id()) {
				// This initvalue is for a component hosted on this node
				// Find the wuboject
				wuobject_t *wuobject;
				wkpf_error_code = wkpf_get_wuobject_by_port(WKPF_COMPONENT_ENDPOINT_PORT(component_id, j), &wuobject);
				if (wkpf_error_code != WKPF_OK)
					return wkpf_error_code;
				uint8_t datatype = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
				switch (datatype) {
					case WKPF_PROPERTY_TYPE_SHORT: {
						int16_t value = dj_di_getU16(initvalues);
						wkpf_error_code = wkpf_external_write_property_int16(wuobject, property_number, value);
						break;
					}
					case WKPF_PROPERTY_TYPE_BOOLEAN: {
						uint8_t value = dj_di_getU8(initvalues);
						wkpf_error_code = wkpf_external_write_property_boolean(wuobject, property_number, value);
						break;
					}
					case WKPF_PROPERTY_TYPE_REFRESH_RATE: {
						int16_t value = dj_di_getU16(initvalues);
						wkpf_error_code = wkpf_external_write_property_refresh_rate(wuobject, property_number, value);
						break;
					}
				}
				if (wkpf_error_code != WKPF_OK) {
					DEBUG_LOG(DBG_WKPF, "------ INITVALUES ERROR: %d\n", wkpf_error_code);
					return wkpf_error_code;
				}
			}
		}
		initvalues += value_size;
	}
	return WKPF_OK;
}

// Updates the current value of this property to be an initvalue
// (if it exists in the initvalue list in the first place)
void wkpf_update_initvalue_in_flash(wuobject_t *wuobject, uint8_t object_property_number) {
	// !!!!!!!!!!!!
	// Note that this will only work as long as the format in the table is the same
	// as in memory. Since WuKong message format and the native and wunode platforms
	// are both little endian, this works fine for now.
	// If we add a big endian platform, the value in the table would still be little
	// endian since it needs to be platform independent. But the value in memory would
	// be big endian, so we need to do a conversion here before storing it back in the
	// table.
	// !!!!!!!!!!!!

	// Find initvalues file
	int filenumber = -1;
	for (int i=0; i<dj_archive_number_of_files(di_app_archive); i++) {
		if (dj_archive_filetype(dj_archive_get_file(di_app_archive, i))==DJ_FILETYPE_WKPF_INITVALUES_TABLE) {
			filenumber = i;
			break;
		}
	}
	// Find component id for wuobject
	uint16_t object_component_id;
	wkpf_get_component_id(wuobject->port_number, &object_component_id);

	dj_di_pointer initvalues = dj_archive_get_file(di_app_archive, filenumber);
	uint16_t offset = 0;

	uint16_t number_of_initvalues = dj_di_getU16(initvalues+offset);
	offset += 2; // Skip number of values
	for (uint16_t i=0; i<number_of_initvalues; i++) {
		uint16_t value_component_id = dj_di_getU16(initvalues+offset);
		offset += 2;
		uint8_t value_property_number = dj_di_getU8(initvalues+offset);
		offset += 1;
		uint8_t value_size = dj_di_getU8(initvalues+offset);
		offset += 1;
		if (object_component_id == value_component_id
				&& object_property_number == value_property_number) {
			wuobject_property_t *property = wkpf_get_property(wuobject, value_property_number);
			wkreprog_open_file(filenumber, offset);
			wkreprog_write(value_size, property->value);
			wkreprog_close();
			return;
		}
		offset += value_size;		
	}
}

