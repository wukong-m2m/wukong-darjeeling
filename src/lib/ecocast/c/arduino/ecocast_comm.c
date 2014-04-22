#include <stddef.h>
#include <stdint.h>
#include "djarchive.h"
#include "debug.h"
#include "program_mem.h"
#include "wkcomm.h"
#include "wkreprog.h"
#include "ecocast_comm.h"
#include "ecocast_capsules.h"

uint16_t offset_in_capsule_file;

// Message format:
//    Request:  [ 0x88, 0x20 (ECOCOMMAND),   seq_LSB, seq_MSB, fragmentnr, lastfragmentnr, retval_size, payload.... ]
//    Response: [ 0x88, 0x21 (ECOCOMMAND_R), seq_LSB, seq_MSB, retval.... ]
//
// Buffer organisation:
// Each capsule starts with a 6 byte capsule header:
//  2 bytes length (little endian)
//  4 bytes hash
//  length-6 bytes capsule data (header size is included in length)
//
// Capsules are required to have an even number of bytes to
// maintain word alignment (python side will take care of this)
//
// Two 0 bytes after the last capsule signal the end of the buffer
// We will assume the capsule is the same if the length and hash
// value match. Given the right hash function on the Python side, 
// the chance of collisions is very low.
// Alternatively, we could make the hash larger, or still receive
// the whole capsule and check it really matches. This would cost
// more communication overhead though, for something that won't
// happen in practice.
void ecocast_comm_handle_message(void *data) {
	wkcomm_received_msg *msg = (wkcomm_received_msg *)data;
	uint8_t *payload = msg->payload;
	uint8_t response_size, response_cmd = 0;

	switch (msg->command) {
		case ECOCAST_COMM_ECOCOMMAND: {
			uint8_t fragmentnr = payload[0];
			uint8_t lastfragmentnr = payload[1];
			uint8_t *capsule_data = payload+3;
			bool capsule_found_in_buffer = false;

			DEBUG_LOG(DBG_ECO, "[ECO] Received packet nr %d, last packet nr %d\n", fragmentnr, lastfragmentnr);
			if (fragmentnr == 0) {
				// FIRST fragment of a capsule: check if we have the capsule already
				uint16_t length = *((uint16_t *)capsule_data);
				uint8_t retval = ecocast_find_capsule_padding_or_empty_space(length, capsule_data+2, &offset_in_capsule_file);
				if (retval == ECOCAST_CAPSULE_FOUND) {
					// Found! Execute it and return the result immediately.
					DEBUG_LOG(DBG_ECO, "[ECO] Found, so executing directly\n");
					capsule_found_in_buffer = true;
				} else if (retval == ECOCAST_CAPSULE_NOT_FOUND) {
					// Not found, but enough free space. Open the file for writing at the first free position.
					wkreprog_open(ecocast_find_capsule_filenr(), offset_in_capsule_file);
					wkreprog_write(msg->length-3, payload+3);
				} else {
					// retval == ECOCAST_BUFFER_TOO_SMALL
					// The capsule file is too small to receive this capsule. Send error code.
					response_cmd = ECOCAST_COMM_ECOCOMMAND_R;
					response_size = 1;
					payload[0] = ECOCAST_REPLY_TOO_BIG;
					break; // Breaking here is a bit ugly. Refactor later.
				}
			} else {
				// Not the first capsule, so we always write it
				wkreprog_write(msg->length-3, payload+3);
			}

			if (fragmentnr == lastfragmentnr) {
				// This is the last part of the capsule, we need to close the flash
				// Unless we're going to execute a capsule already in flash, because then we never opened it.
				if (!capsule_found_in_buffer) {
					// First write two 0 bytes to signal this is currently the last capsule in the file.
					uint8_t zeros[] = { 0, 0 };
					wkreprog_write(2, zeros);
					// Then close the file.
					wkreprog_close();
				}
			}
			
			if (fragmentnr == lastfragmentnr
					|| capsule_found_in_buffer) {
				ecocast_execute_code_capsule_at_offset(offset_in_capsule_file, payload[2], payload + 1);
				response_size = payload[2] + 1;
				payload[0] = ECOCAST_REPLY_EXECUTED;
			} else {
				response_size = 1;
				payload[0] = ECOCAST_REPLY_OK;				
			}
			response_cmd = ECOCAST_COMM_ECOCOMMAND_R;
		}
		break;
	}
	if (response_cmd != 0)
		wkcomm_send_reply(msg, response_cmd, payload, response_size);
}


