#ifndef WKPF_PROPERTIESH
#define WKPF_PROPERTIESH

#include "types.h"
#include "wkpf.h"
#include "wkpf_wuobjects.h"

// Property state:
// Bit 0: retry timer bit: retry when current time bit (failure count*2)+1 equals retry timer bit
// Bits 1,2,3: failure count
// Bits 4,5,6,7: status [clean|needs_push|needs_initialisation_push|needs_pull|needs_pull_waiting]
// Schedule:
// Failure count 1: retry immediately
// Failure count 2: retry after 0 to 2^5 = 32ms
// Failure count 3: retry after 0 to 2^7 = 128ms
// Failure count 4: retry after 0 to 2^9 = 512ms
// Failure count 5: retry after 0 to 2^11 = 2s
// Failure count 6: retry after 0 to 2^13 = 8s
// Failure count 7+: retry after 0 to 2^15 = 32s
#define PROPERTY_STATUS_NEEDS_PUSH                  0x10 // Property has changed and needs to be propagated
#define PROPERTY_STATUS_FORCE_NEXT_PUSH             0x20 // Received initialisation request from remote node. Propagate this property regardless of any filters (which we don't have yet, but may implement at some later stage)
#define PROPERTY_STATUS_NEEDS_PULL                  0x40 // Property has incoming link and hasn't been initialised yet. Need to fetch value from source WuObject
#define PROPERTY_STATUS_NEEDS_PULL_WAITING          0x80 // Uninit message accepted by remote node. Waiting to receive value through normal WRITE_PROPERTY message
#define PROPERTY_STATUS_FAILURE_COUNT_TIMES2_MASK   0x0E // Times two since the failure count is stored in bits 1,2,3

// Access functions that check r/w access permission, used for external access
#define wkpf_external_read_property_int16(wuobject, property_number, value)           wkpf_read_property_int16(wuobject, property_number, true, value)
#define wkpf_external_write_property_int16(wuobject, property_number, value)          wkpf_write_property_int16(wuobject, property_number, true, value)
#define wkpf_external_read_property_boolean(wuobject, property_number, value)         wkpf_read_property_boolean(wuobject, property_number, true, value)
#define wkpf_external_write_property_boolean(wuobject, property_number, value)        wkpf_write_property_boolean(wuobject, property_number, true, value)
#define wkpf_external_read_property_refresh_rate(wuobject, property_number, value)    wkpf_read_property_refresh_rate(wuobject, property_number, true, value)
#define wkpf_external_write_property_refresh_rate(wuobject, property_number, value)   wkpf_write_property_refresh_rate(wuobject, property_number, true, value)

// Access functions that don't check r/w access permission, used by the wuclasses to access their own properties
#define wkpf_internal_read_property_int16(wuobject, property_number, value)           wkpf_read_property_int16(wuobject, property_number, false, value)
#define wkpf_internal_write_property_int16(wuobject, property_number, value)          wkpf_write_property_int16(wuobject, property_number, false, value)
#define wkpf_internal_read_property_boolean(wuobject, property_number, value)         wkpf_read_property_boolean(wuobject, property_number, false, value)
#define wkpf_internal_write_property_boolean(wuobject, property_number, value)        wkpf_write_property_boolean(wuobject, property_number, false, value)
#define wkpf_internal_read_property_refresh_rate(wuobject, property_number, value)    wkpf_read_property_refresh_rate(wuobject, property_number, false, value)
#define wkpf_internal_write_property_refresh_rate(wuobject, property_number, value)   wkpf_write_property_refresh_rate(wuobject, property_number, false, value)

extern uint8_t wkpf_read_property_int16(wuobject_t *wuobject, uint8_t property_number, bool external_access, int16_t *value);
extern uint8_t wkpf_write_property_int16(wuobject_t *wuobject, uint8_t property_number, bool external_access, int16_t value);
extern uint8_t wkpf_read_property_boolean(wuobject_t *wuobject, uint8_t property_number, bool external_access, bool *value);
extern uint8_t wkpf_write_property_boolean(wuobject_t *wuobject, uint8_t property_number, bool external_access, bool value);
extern uint8_t wkpf_read_property_refresh_rate(wuobject_t *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t *value);
extern uint8_t wkpf_write_property_refresh_rate(wuobject_t *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t value);
extern uint8_t wkpf_get_property_status(wuobject_t *wuobject, uint8_t property_number, uint8_t *status);

extern uint8_t wkpf_property_needs_initialisation_push(wuobject_t *wuobject, uint8_t property_number);

extern uint8_t wkpf_update_map_in_flash(uint16_t component_id, uint16_t orig_node_id, uint8_t orig_port_number, uint16_t new_node_id, uint8_t new_port_number);
extern uint8_t wkpf_update_link_in_flash(uint8_t* orig_link, uint8_t* new_link);

extern bool wkpf_property_status_is_dirty(uint8_t status);
extern void wkpf_propagating_dirty_property_failed(wuobject_property_t *property);
extern void wkpf_propagating_dirty_property_succeeded(wuobject_property_t *property);
extern void wkpf_set_property_status_needs_pull(wuobject_property_t *property);

#endif // WKPF_PROPERTIESH
