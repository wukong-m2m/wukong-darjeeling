#include "core.h"
#include "debug.h"
#include "types.h"
#include "hooks.h"
#include "wkcomm.h"
#include "wkreprog_comm.h"
#include "wkreprog.h"

static uint16_t wkreprog_pos;

void wkreprog_comm_handle_message(void *data) {
	wkcomm_received_msg *msg = (wkcomm_received_msg *)data;
	uint8_t *payload = msg->payload;
	uint8_t response_size = 0, response_cmd = 0;

	// TODONR: check on file size
	// TODONR: add checksum
	switch (msg->command) {
		case WKREPROG_COMM_CMD_REPROG_OPEN: {
			DEBUG_LOG(DBG_WKREPROG, "Initialise reprogramming.\n");
			// uint16_t size_to_upload = (uint16_t)payload[0] + (((uint16_t)payload[1]) << 8);
			if (wkreprog_impl_open_app_archive(0)) {
				// TODONR: DEBUG_LOG(DBG_WKREPROG, "Setting master address to %x", src);
			    // wkpf_config_set_master_node_id(src);
				DEBUG_LOG(DBG_WKREPROG, "Going to runlevel RUNLEVEL_REPROGRAMMING.\n");
				dj_exec_setRunlevel(RUNLEVEL_REPROGRAMMING);
				DEBUG_LOG(DBG_WKREPROG, "Initialise reprogramming code.\n");
				wkreprog_pos = 0;
				DEBUG_LOG(DBG_WKREPROG, "Send WKREPROG_COMM_CMD_REPROG_OPEN_R containing page size.\n");
				uint16_t pagesize = wkreprog_get_page_size();
				payload[0] = WKREPROG_OK;
				payload[1] = (uint8_t)(pagesize);
				payload[2] = (uint8_t)(pagesize>>8);
				response_size = 3;
			} else {
				payload[0] = WKREPROG_TOOLARGE;
				response_size = 1;
			}
			response_cmd = WKREPROG_COMM_CMD_REPROG_OPEN_R;
		}
		break;
		case WKREPROG_COMM_CMD_REPROG_WRITE: {
			uint16_t pos_in_message = (uint16_t)payload[0] + (((uint16_t)payload[1]) << 8);
			DEBUG_LOG(DBG_WKREPROG, "Received program packet for address 0x%x, current position: 0x%x.\n", pos_in_message, wkreprog_pos);
			uint8_t codelength = msg->length - 2;
			uint8_t *codepayload = payload + 2;
			uint16_t pagesize = wkreprog_get_page_size();
			if (pos_in_message/(uint16_t)pagesize != (pos_in_message+(uint16_t)codelength)/(uint16_t)pagesize) {
				// Crossing page boundary, send a reply with OK or REQUEST_RETRANSMIT
				if (pos_in_message == wkreprog_pos) {
					DEBUG_LOG(DBG_WKREPROG, "Page boundary reached. Sending OK.\n");
					payload[0] = WKREPROG_OK;
					response_size = 1;
				} else {
					DEBUG_LOG(DBG_WKREPROG, "Page boundary reached, positions don't match. Sending REQUEST_RETRANSMIT.\n");
					payload[0] = WKREPROG_REQUEST_RETRANSMIT;
					payload[1] = (uint8_t)(wkreprog_pos);
					payload[2] = (uint8_t)(wkreprog_pos>>8);
					response_size = 3;
				}
				response_cmd = WKREPROG_COMM_CMD_REPROG_WRITE_R;
			}
			if (pos_in_message == wkreprog_pos) {
				DEBUG_LOG(DBG_WKREPROG, "Write %d bytes at position 0x%x.\n", codelength, wkreprog_pos);
				wkreprog_impl_write(codelength, codepayload);
				wkreprog_pos += codelength;
			}
		}
		break;
		case WKREPROG_COMM_CMD_REPROG_COMMIT: {
			uint16_t pos_in_message = (uint16_t)payload[0] + (((uint16_t)payload[1]) << 8);
			DEBUG_LOG(DBG_WKREPROG, "Received commit request for code up to address 0x%x, current position: 0x%x.\n", pos_in_message, wkreprog_pos);
			bool reprogramming_ok = false;
			if (pos_in_message != wkreprog_pos) {
				DEBUG_LOG(DBG_WKREPROG, "Positions don't match. Sending REQUEST_RETRANSMIT.");
				payload[0] = WKREPROG_REQUEST_RETRANSMIT;
				payload[1] = (uint8_t)(wkreprog_pos);
				payload[2] = (uint8_t)(wkreprog_pos>>8);
				response_size = 3;
			} else if (0==1) {
				// TODO: add checksum, send NVMCOMM_CMD_REPRG_COMMIT_R_FAILED if they don't match.
				payload[0] = WKREPROG_FAILED;
				response_size = 1;
			} else {
				payload[0] = WKREPROG_OK;
				response_size = 1;
				reprogramming_ok = true;
				DEBUG_LOG(DBG_WKREPROG, "Committing new code.\n");
				DEBUG_LOG(DBG_WKREPROG, "Flushing pending writes to flash.\n");
			}
			response_cmd = WKREPROG_COMM_CMD_REPROG_COMMIT_R;

			if (reprogramming_ok)
				wkreprog_impl_close();
		}
		break;
		case WKREPROG_COMM_CMD_REPROG_REBOOT: {
			DEBUG_LOG(DBG_WKREPROG, "Reboot the VM.\n");
			dj_hook_call(dj_core_shutdownHook, NULL);
			wkreprog_impl_reboot();
		}
	}
	if (response_cmd != 0)
		wkcomm_send_reply(msg, response_cmd, payload, response_size);
}