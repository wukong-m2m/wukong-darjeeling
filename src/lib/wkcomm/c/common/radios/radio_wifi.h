#ifndef RADIO_LOCALH
#define RADIO_LOCALH

#include "types.h"

typedef uint32_t radio_wifi_address_t;

extern void radio_wifi_init(void);
extern radio_wifi_address_t radio_wifi_get_node_id();
extern radio_wifi_address_t radio_wifi_get_prefix_mask();
extern void radio_wifi_poll(void);
extern uint8_t radio_wifi_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length);
extern uint8_t radio_wifi_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length);

#endif // RADIO_LOCALH
