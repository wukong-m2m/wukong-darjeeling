#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>
#include <stdint.h>
#include "djarchive.h"
#include "wkcomm.h"
#include "wkreprog.h"
#include "ecocast_comm.h"
#include "program_mem.h"

// Format:
//    Request:  [ 0x88, 0x20 (ECOCOMMAND),   seq_LSB, seq_MSB, packetnr, lastpacketnr, retval_size, payload.... ]
//    Response: [ 0x88, 0x21 (ECOCOMMAND_R), seq_LSB, seq_MSB, retval.... ]

dj_di_pointer ecocast_code_capsule_start = 0;

void ecocast_execute_code_capsule(dj_di_pointer capsule, uint8_t retval_size, uint8_t* retval_buffer) {
	// According to http://gcc.gnu.org/wiki/avr-gcc
	//  Z (R30, R31) is call used and could be destroyed by the function in the capsule, but since we already did the ICALL then this doesn't matter
	//  Y (R28, R29) is call saved. This means it's still safe after ICALL, but we need to restore the contents if we modify it (for 16 and 32 bit retval)

	// TODONR: this doesn't work if capsule is above 64K
	uint16_t programme_counter_address = capsule/2; // PC is in words, not bytes.
	switch (retval_size) {
		case 1:
			asm("ICALL\n" \
				"ST Y, R24\n" \
				: \
				: "z"(programme_counter_address), "y"(retval_buffer));
		break;
		case 2:
			asm("ICALL\n" \
				"PUSH R28\n" \
				"PUSH R29\n" \
				"ST Y+, R24\n" \
				"ST Y+, R25\n" \
				"POP R29\n" \
				"POP R28\n" \
				: \
				: "z"(programme_counter_address), "y"(retval_buffer));
		break;
		case 4:
			asm("ICALL\n" \
				"PUSH R28\n" \
				"PUSH R29\n" \
				"ST Y+, R22\n" \
				"ST Y+, R23\n" \
				"ST Y+, R24\n" \
				"ST Y+, R25\n" \
				"POP R29\n" \
				"POP R28\n" \
				: \
				: "z"(programme_counter_address), "y"(retval_buffer));
		break;
	}
}

void ecocast_comm_handle_message(void *data) {
	wkcomm_received_msg *msg = (wkcomm_received_msg *)data;
	uint8_t *payload = msg->payload;
	uint8_t response_size = 0, response_cmd = 0;

	switch (msg->command) {
		case ECOCAST_COMM_ECOCOMMAND: {
			uint8_t packetnr = payload[0];
			uint8_t lastpacketnr = payload[1];

			if (packetnr == 0) {
				ecocast_code_capsule_start = dj_archive_get_file(di_app_archive, 0);
				if ((ecocast_code_capsule_start & 1) == 1) {
					// Need to start writing at a word boundary
					wkreprog_open(0, 1);
					ecocast_code_capsule_start++;
				} else {
					wkreprog_open(0, 0);
				}
			}

			wkreprog_write(msg->length-3, payload+3);

			if (packetnr == lastpacketnr) {
				// Flush current page to flash
				wkreprog_close();
				// Execute the code
				ecocast_execute_code_capsule(ecocast_code_capsule_start, payload[2], payload);
			} else {
				response_size = 0;
			}
			response_cmd = ECOCAST_COMM_ECOCOMMAND_R;
		}
		break;
	}
	if (response_cmd != 0)
		wkcomm_send_reply(msg, response_cmd, payload, response_size);
}


