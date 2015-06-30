#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "core.h"
#include "hooks.h"
#include "types.h"
#include "config.h"
#include "debug.h"
#include "djtimer.h"
#include "routing/routing.h"
#include "radio_wifi.h"
#include "posix_utils.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"

#ifdef RADIO_USE_WIFI

extern void radio_wifi_platform_dependent_init(void);
extern radio_wifi_address_t radio_wifi_platform_dependent_get_node_id();
extern radio_wifi_address_t radio_wifi_platform_dependent_get_prefix_mask();
extern void radio_wifi_platform_dependent_poll();
extern uint8_t radio_wifi_platform_dependent_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length);
extern uint8_t radio_wifi_platform_dependent_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length);

void radio_wifi_init(void) {
    radio_wifi_platform_dependent_init();
}

radio_wifi_address_t radio_wifi_get_node_id() {
    return radio_wifi_platform_dependent_get_node_id();
}

radio_wifi_address_t radio_wifi_get_prefix_mask() {
    return radio_wifi_platform_dependent_get_prefix_mask();
}

void radio_wifi_poll(void) {
    radio_wifi_platform_dependent_poll();
}

uint8_t radio_wifi_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    return radio_wifi_platform_dependent_send(dest, payload, length);
}

uint8_t radio_wifi_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    return radio_wifi_platform_dependent_send_raw(dest, payload, length);
}

#endif // RADIO_USE_WIFI
