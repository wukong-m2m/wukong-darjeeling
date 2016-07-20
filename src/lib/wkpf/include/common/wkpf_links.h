#ifndef WKPF_LINKSH
#define WKPF_LINKSH

#include "wkcomm.h"
#include "program_mem.h"

extern bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number);
extern uint8_t wkpf_propagate_dirty_properties();
extern uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, wkcomm_address_t *node_id, uint8_t *port_number);

extern bool wkpf_node_is_leader(uint16_t component_id, wkcomm_address_t node_id);

uint8_t wkpf_init_token();
uint8_t wkpf_set_token (uint16_t lock_component_id, uint16_t src_component_id, uint16_t dest_component_id);
uint8_t wkpf_release_token(uint16_t token_id);
uint8_t wkpf_update_token_table (uint16_t* component_ids, int length, uint16_t src_component, uint16_t dest_component);
uint8_t wkpf_update_token_table_with_piggyback (uint8_t* piggyback_message);
int wkpf_find_token(uint16_t dest_component_id);
bool wkpf_component_is_locked(uint16_t dest_component_id);

uint8_t wkpf_load_links(dj_di_pointer links);
uint8_t wkpf_load_component_to_wuobject_map(dj_di_pointer map);
uint8_t wkpf_create_local_wuobjects_from_app_tables();
uint8_t wkpf_process_initvalues_list(dj_di_pointer initvalues);

// Updates the current value of this property to be an initvalue
// (if it exists in the initvalue list in the first place,
// if there's no initvalue, this function won't create an entry,
// so it's a noop in that case)
void wkpf_update_initvalue_in_flash(wuobject_t *wuobject, uint8_t object_property_number);
uint8_t wkpf_update_link(uint16_t orig_src_component_id, uint8_t orig_src_property_id, uint16_t orig_dest_component_id, uint8_t orig_dest_property_id,
                                  uint16_t new_src_component_id, uint8_t new_src_property_id, uint16_t new_dest_component_id, uint8_t new_dest_property_id);
uint8_t wkpf_propagate_link_change( uint16_t orig_src_component_id, uint8_t orig_src_property_id,
                                                                uint16_t orig_dest_component_id, uint8_t orig_dest_property_id, uint16_t new_src_component_id,
                                                                uint8_t new_src_property_id, uint16_t new_dest_component_id, uint8_t new_dest_property_id);
uint8_t wkpf_update_map_in_flash(uint16_t component_id, uint32_t orig_node_id, uint8_t orig_port_number,
                                uint32_t new_node_id, uint8_t new_port_number);


#endif // WKPF_LINKSH
