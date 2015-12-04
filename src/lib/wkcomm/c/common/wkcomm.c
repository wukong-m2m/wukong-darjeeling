#include <string.h>
#include "panic.h"
#include "debug.h"
#include "config.h"
#include "hooks.h"
#include "djtimer.h"

#include "routing/routing.h"
#include "wkcomm.h"

// Periodically call wkcomm_poll to receive messages
dj_hook wkcomm_pollingHook;

// Keep track of sequence numbers
uint16_t wkcomm_last_seqnr = 0;

// Some variables to wait for a reply
uint8_t *wkcomm_wait_reply_commands;
uint8_t wkcomm_wait_reply_number_of_commands;
uint16_t wkcomm_wait_reply_seqnr;
wkcomm_received_msg wkcomm_received_reply;

// To allow other libraries to listen to received messages
dj_hook *wkcomm_handle_message_hook = NULL;

// Send length bytes to dest
int wkcomm_send_raw(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	return routing_send_raw(dest, payload, length);
}
int wkcomm_do_send(wkcomm_address_t dest, uint8_t command, uint8_t *payload, uint8_t length, uint16_t seqnr) {
	DEBUG_LOG(DBG_WKCOMM, "wkcomm_send\n");
	if (length > WKCOMM_MESSAGE_PAYLOAD_SIZE) {
		DEBUG_LOG(DBG_WKCOMM, "message oversized\n");
		return WKCOMM_SEND_ERR_TOO_LONG; // Message too large
	}

	uint8_t buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // 2 bytes for the seq nr, 1 for the command
	buffer[0] = command;
    buffer[1] = seqnr & 0xFF;
    buffer[2] = seqnr >> 8;
	memcpy (buffer+3, payload, length);
	return routing_send(dest, buffer, length+3);
}

int wkcomm_send(wkcomm_address_t dest, uint8_t command, uint8_t *payload, uint8_t length) {
	return wkcomm_do_send(dest, command, payload, length, ++wkcomm_last_seqnr);
}

int wkcomm_send_reply(wkcomm_received_msg *received_msg, uint8_t command, uint8_t *payload, uint8_t length) {
	return wkcomm_do_send(received_msg->src, command, payload, length, received_msg->seqnr);
}

// Send length bytes to dest and wait for a specific reply (and matching sequence nr)
int wkcomm_send_and_wait_for_reply(wkcomm_address_t dest, uint8_t command, uint8_t *payload, uint8_t length,
							uint16_t wait_msec, uint8_t *reply_commands, uint8_t number_of_reply_commands, wkcomm_received_msg **reply) {

	// Set global variables to wait for the required message types
	wkcomm_wait_reply_commands = reply_commands;
	wkcomm_wait_reply_number_of_commands = number_of_reply_commands;
	wkcomm_received_reply.command = 0; // command will be != 0 once a reply has been received.
	// Need to store the current sequence nr because other messages might be sent while
	// waiting for the reply, so we can't use wkcomm_last_seqnr to check the incoming replies.
	wkcomm_wait_reply_seqnr = ++wkcomm_last_seqnr;

	// Do the send, and store the seqnr so wkcomm_handle_message can check for a match
	int8_t retval = wkcomm_do_send(dest, command, payload, length, wkcomm_last_seqnr);

	if (retval != 0)
		return retval; // Something went wrong during send.

	dj_time_t deadline = dj_timer_getTimeMillis() + wait_msec;
	do {
		wkcomm_poll(NULL);
		if (wkcomm_received_reply.command != 0) {
			// Reply received
			*reply = &wkcomm_received_reply;
			wkcomm_wait_reply_number_of_commands = 0;
			return WKCOMM_SEND_OK;
		}
	} while(deadline > dj_timer_getTimeMillis());
	return WKCOMM_SEND_ERR_NO_REPLY;
}

// Message handling. This function is called from the radio code (radio_zwave_poll or radio_xbee_poll), checks for replies we may be waiting for, or passes on the handling to one of the other libs.
void wkcomm_handle_message(wkcomm_address_t addr, uint8_t *payload, uint8_t length) {
#ifdef DARJEELING_DEBUG
	DEBUG_LOG(DBG_WKCOMM, "Handling command %d from %d, length %d:\n", payload[0], addr, length);
	for (int8_t i=0; i<length; ++i) {
		DEBUG_LOG(DBG_WKCOMM, " %d", payload[i]);
	}
	DEBUG_LOG(DBG_WKCOMM, "\n");
#endif // DARJEELING_DEBUG

	wkcomm_received_msg msg;
	msg.src = addr;
	msg.command = 	payload[0];
	msg.seqnr = payload[1] + (((uint16_t)payload[2]) << 8);
	msg.payload = payload+3;
	msg.length = length - 3;

	if (wkcomm_wait_reply_number_of_commands > 0) {
		// nvmcomm_wait is waiting for a particular type of message. probably a response to a message sent earlier.
		// if this message is of that type, store it in nvmcomm_wait_received_message so nvmcomm_wait can return it.
		// if not, handle it as a normal message
		if (wkcomm_wait_reply_number_of_commands != 0
				&& msg.seqnr == wkcomm_last_seqnr) {
			for (int i=0; i<wkcomm_wait_reply_number_of_commands; i++) {
				if (msg.command == wkcomm_wait_reply_commands[i]) {
					wkcomm_received_reply = msg; // Struct, so values are copied. Radio libs need to provide a pointer to a global payload buffer.
					wkcomm_wait_reply_number_of_commands = 0; // Signal we're no longer waiting for the reply.
				}
			}
		}
	}

	// Pass on to other libs. Could have a system here were libraries register for specific commands, but this seems simpler, and only a bit slower if handlers return quickly when the message isn't meant for them.
	dj_hook_call(wkcomm_handle_message_hook, &msg);
}

void wkcomm_poll(void *dummy) {
	routing_poll();
}

wkcomm_address_t wkcomm_get_node_id() {
	return routing_get_node_id();
}

wkcomm_address_t wkcomm_get_gateway_id() {
	return routing_get_gateway_id();
}