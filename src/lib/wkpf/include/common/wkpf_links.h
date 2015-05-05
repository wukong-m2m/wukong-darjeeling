#ifndef WKPF_LINKSH
#define WKPF_LINKSH

#include "wkcomm.h"
#include "program_mem.h"

extern bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number);
extern uint8_t wkpf_propagate_dirty_properties();
extern uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, wkcomm_address_t *node_id, uint8_t *port_number);

extern bool wkpf_node_is_leader(uint16_t component_id, wkcomm_address_t node_id);

uint8_t wkpf_load_links(dj_di_pointer links);
uint8_t wkpf_load_component_to_wuobject_map(dj_di_pointer map);
uint8_t wkpf_create_local_wuobjects_from_app_tables();
uint8_t wkpf_process_initvalues_list(dj_di_pointer initvalues);

// Updates the current value of this property to be an initvalue
// (if it exists in the initvalue list in the first place,
// if there's no initvalue, this function won't create an entry,
// so it's a noop in that case)
void wkpf_update_initvalue_in_flash(wuobject_t *wuobject, uint8_t object_property_number);

#endif // WKPF_LINKSH
