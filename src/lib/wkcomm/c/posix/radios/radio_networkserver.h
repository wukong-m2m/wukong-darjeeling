#ifndef RADIO_LOCALH
#define RADIO_LOCALH

#include "types.h"

typedef uint16_t radio_networkserver_address_t;

extern void radio_networkserver_init(void);
extern radio_networkserver_address_t radio_networkserver_get_node_id();
extern void radio_networkserver_poll(void);
extern uint8_t radio_networkserver_send(radio_networkserver_address_t dest, uint8_t *payload, uint8_t length);

#endif // RADIO_LOCALH
