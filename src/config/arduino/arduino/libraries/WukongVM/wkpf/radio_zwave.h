#ifndef RADIO_ZWAVEH
#define RADIO_ZWAVEH

#include "types.h"

typedef uint8_t radio_zwave_address_t;

extern void radio_zwave_init(void);
extern radio_zwave_address_t radio_zwave_get_node_id();
extern void radio_zwave_poll(void);
extern uint8_t radio_zwave_send(radio_zwave_address_t dest, uint8_t *payload, uint8_t length);
extern uint8_t radio_zwave_send_raw(radio_zwave_address_t dest, uint8_t *payload, uint8_t length);
extern void radio_zwave_set_node_info(uint8_t devmask,uint8_t generic, uint8_t specific);

#endif // RADIO_ZWAVEH
