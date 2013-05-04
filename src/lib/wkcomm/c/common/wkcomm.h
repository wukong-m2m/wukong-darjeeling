#ifndef WKCOMMH
#define WKCOMMH

#include "types.h"
#include "hooks.h"

// Size of the frame that will be sent to the radio by wkcomm is WKCOMM_MESSAGE_PAYLOAD_SIZE+3
// 1 byte: command
// 2 bytes: seqnr
// TODONR: higher payload sizes seem to cause problems in the uart.
#define WKCOMM_MESSAGE_PAYLOAD_SIZE     40

#define WKCOMM_SEND_OK					 0
#define WKCOMM_SEND_ERR_TOO_LONG		 3
#define WKCOMM_SEND_ERR_NO_REPLY		 4

// Need to make sure these codes don't overlap with other libs or the definitions in panic.h
#define WKCOMM_PANIC_INIT_FAILED 100

// WuKong address. For now it's just a byte, but this will probably change.
// When it does, we need to change the component-node map as well
typedef uint8_t wkcomm_address_t;

typedef struct wkcomm_received_msg {
	wkcomm_address_t src;
	uint16_t seqnr;
	uint8_t command;
	uint8_t *payload;
	uint8_t length;
} wkcomm_received_msg;

// To allow other libraries to listen to received messages
extern dj_hook *wkcomm_handle_message_hook;

// Message handling. This function is called from the routing library, checks for replies we may be waiting for, or passes on the handling to one of the other libs.
extern void wkcomm_handle_message(wkcomm_address_t addr, uint8_t *payload, uint8_t length);

// Get my own node id, directly from routing
wkcomm_address_t wkcomm_get_node_id();

// Call this periodically to receive data
extern void wkcomm_poll(void *);

// Send length bytes to dest
extern int wkcomm_send(wkcomm_address_t dest, uint8_t command, uint8_t *payload, uint8_t length);

// Send length bytes to dest
extern int wkcomm_send_reply(wkcomm_received_msg *received_msg, uint8_t command, uint8_t *payload, uint8_t length);

// Send length bytes to dest and wait for a specific reply (and matching sequence nr)
extern int wkcomm_send_and_wait_for_reply(wkcomm_address_t dest, uint8_t command, uint8_t *payload, uint8_t length, uint16_t wait_msec, uint8_t *commands, uint8_t number_of_commands, wkcomm_received_msg **reply);

#endif // WKCOMMH
