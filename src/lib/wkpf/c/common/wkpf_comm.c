#include "wkcomm.h"
#include "panic.h"
#include "debug.h"
#include "core.h"

#include "wkpf.h"
#include "wkpf_comm.h"
#include "wkpf_config.h"
#include "wkpf_wuclasses.h"
#include "wkpf_wuobjects.h"
#include "wkpf_properties.h"

#define NUMBER_OF_WUCLASSES_PER_MESSAGE 9
#define NUMBER_OF_WUOBJECTS_PER_MESSAGE 9

uint8_t send_message(wkcomm_address_t dest_node_id, uint8_t command, uint8_t *payload, uint8_t length) {
	// Print some debug info
#ifdef DARJEELING_DEBUG
	DEBUG_LOG(DBG_WKPF, "WKPF: sending property set command to %d:", dest_node_id);
	for(int i=0; i<length; i++) {
		DEBUG_LOG(DBG_WKPF, "[%x] ", payload[i]);
	}
	DEBUG_LOG(DBG_WKPF, "\n");
#endif

	// Send
	wkcomm_received_msg *reply;
	uint8_t retval = wkcomm_send_and_wait_for_reply(dest_node_id, command, payload, length,
													1000 /* 1000ms timeout */,  (uint8_t[]){command+1 /* the reply to this command */, WKPF_COMM_CMD_ERROR_R}, 2, &reply);
	if (retval == WKCOMM_SEND_OK) {
		if (reply->command != WKPF_COMM_CMD_ERROR_R)
			return WKPF_OK;
		else
			return reply->payload[0]; // Contains a WKPF_ERR code.
	} else if (retval == WKCOMM_SEND_ERR_NO_REPLY) {
		return WKPF_ERR_NVMCOMM_NO_REPLY;
	} else {
		return WKPF_ERR_NVMCOMM_SEND_ERROR;
	}
}

void send_message_withou_reply(wkcomm_address_t dest_node_id, uint8_t command, uint8_t *payload, uint8_t length) {
    // Print some debug info
    #ifdef DARJEELING_DEBUG
    	DEBUG_LOG(true, "WKPF: sending property set command to %d:", dest_node_id);
    	for(int i=0; i<length; i++) {
    		DEBUG_LOG(true, "[%x] ", payload[i]);
    	}
    	DEBUG_LOG(true, "\n");
    #endif

    wkcomm_send(dest_node_id, command, payload, length);
}

uint8_t wkpf_call_adaptor(wkcomm_address_t dest_node_id, uint16_t wuclass_id, uint8_t property_number, uint16_t value)
{
	uint8_t buf[6];
	uint8_t r;

	DEBUG_LOG(DBG_WKPF, "Send value %d to node %d\n", value, dest_node_id);
	buf[0] = 0x20;		// COMMAND_CLASS_BASIC
	buf[1] = 1;			// BASIC_SET
	buf[2] = value;		// level
	r =  wkcomm_send_raw(dest_node_id,buf,3);
	DEBUG_LOG(DBG_WKPF,"send raw done\n");
	return r;

}


uint8_t wkpf_send_set_property_int16(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, int16_t value) {
	uint8_t message_buffer[7];
	if (port_number >= DEVICE_NATIVE_ZWAVE_SWITCH) {
		return wkpf_call_adaptor(dest_node_id, wuclass_id, property_number, value);
	} else {
		message_buffer[0] = port_number;
		message_buffer[1] = (uint8_t)(wuclass_id >> 8);
		message_buffer[2] = (uint8_t)(wuclass_id);
		message_buffer[3] = property_number;
		message_buffer[4] = WKPF_PROPERTY_TYPE_SHORT;
		message_buffer[5] = (uint8_t)(value >> 8);
		message_buffer[6] = (uint8_t)(value);
		return send_message(dest_node_id, WKPF_COMM_CMD_WRITE_PROPERTY, message_buffer, 7);
	}
}


uint8_t wkpf_send_set_property_boolean(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, bool value) {
	uint8_t message_buffer[6];
	if (port_number >= DEVICE_NATIVE_ZWAVE_SWITCH) {
		return wkpf_call_adaptor(dest_node_id, wuclass_id, property_number, value? 255:0);
	} else {
		message_buffer[0] = port_number;
		message_buffer[1] = (uint8_t)(wuclass_id >> 8);
		message_buffer[2] = (uint8_t)(wuclass_id);
		message_buffer[3] = property_number;
		message_buffer[4] = WKPF_PROPERTY_TYPE_BOOLEAN;
		message_buffer[5] = (uint8_t)(value);
		return send_message(dest_node_id, WKPF_COMM_CMD_WRITE_PROPERTY, message_buffer, 6);
	}
}

uint8_t wkpf_send_set_property_refresh_rate(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, wkpf_refresh_rate_t value) {
	uint8_t message_buffer[8];
	if (port_number >= DEVICE_NATIVE_ZWAVE_SWITCH) {
		return WKPF_COMM_CMD_ERROR_R;
	} else {
		message_buffer[0] = port_number;
		message_buffer[1] = (uint8_t)(wuclass_id >> 8);
		message_buffer[2] = (uint8_t)(wuclass_id);
		message_buffer[3] = property_number;
		message_buffer[4] = WKPF_PROPERTY_TYPE_REFRESH_RATE;
		message_buffer[5] = (uint8_t)(value >> 8);
		message_buffer[6] = (uint8_t)(value);
		return send_message(dest_node_id, WKPF_COMM_CMD_WRITE_PROPERTY, message_buffer, 7);
	}
}

uint8_t wkpf_send_monitor_property_int16(wkcomm_address_t progression_server_id, uint16_t wuclass_id, uint8_t port_number, int16_t value) {
    uint8_t message_buffer[6];
    if (progression_server_id >= DEVICE_NATIVE_ZWAVE_SWITCH) {
        return WKPF_COMM_CMD_ERROR_R;
    } else {
        message_buffer[0] = (uint8_t)(wuclass_id >> 8);
        message_buffer[1] = (uint8_t)(wuclass_id);
        message_buffer[2] = port_number;
        message_buffer[3] = WKPF_PROPERTY_TYPE_SHORT;
        message_buffer[4] = (uint8_t)(value >> 8);
        message_buffer[5] = (uint8_t)(value);
        send_message_withou_reply(progression_server_id, WUKONG_MONITOR_PROPERTY, message_buffer, 6);
        return WKPF_OK;
    }

}

uint8_t wkpf_send_monitor_property_boolean(wkcomm_address_t progression_server_id, uint16_t wuclass_id, uint8_t port_number, bool value) {

    uint8_t message_buffer[5];
    if (progression_server_id >= DEVICE_NATIVE_ZWAVE_SWITCH) {
        return WKPF_COMM_CMD_ERROR_R;
    } else {
        message_buffer[0] = (uint8_t)(wuclass_id >> 8);
        message_buffer[1] = (uint8_t)(wuclass_id);
        message_buffer[2] = port_number;
        message_buffer[3] = WKPF_PROPERTY_TYPE_BOOLEAN;
        message_buffer[4] = (uint8_t)(value);
        send_message_withou_reply(progression_server_id, WUKONG_MONITOR_PROPERTY, message_buffer, 5);
        return WKPF_OK;
    }
}

uint8_t wkpf_send_monitor_property_refresh_rate(wkcomm_address_t progression_server_id, uint16_t wuclass_id, uint8_t port_number, wkpf_refresh_rate_t value) {

    uint8_t message_buffer[6];
    if (progression_server_id >= DEVICE_NATIVE_ZWAVE_SWITCH) {
        return WKPF_COMM_CMD_ERROR_R;
    } else {
        message_buffer[0] = (uint8_t)(wuclass_id >> 8);
        message_buffer[1] = (uint8_t)(wuclass_id);
        message_buffer[2] = port_number;
        message_buffer[3] = WKPF_PROPERTY_TYPE_REFRESH_RATE;
        message_buffer[4] = (uint8_t)(value >> 8);
        message_buffer[5] = (uint8_t)(value);
        send_message_withou_reply(progression_server_id, WUKONG_MONITOR_PROPERTY, message_buffer, 6);
        return WKPF_OK;
    }
}


uint8_t wkpf_send_request_property_init(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number) {
	uint8_t message_buffer[2];
	message_buffer[0] = port_number;
	message_buffer[1] = property_number;
	return send_message(dest_node_id, WKPF_COMM_CMD_REQUEST_PROPERTY_INIT, message_buffer, 2);
}


//void wkpf_comm_handle_message(wkcomm_address_t src, uint8_t nvmcomm_command, uint8_t *payload, uint8_t response_size, uint8_t response_cmd) {
void wkpf_comm_handle_message(void *data) {
	wkcomm_received_msg *msg = (wkcomm_received_msg *)data;
	uint8_t *payload = msg->payload;
	uint8_t response_size = 0, response_cmd = 0;
	uint8_t retval;

	if (dj_exec_getRunlevel() == RUNLEVEL_REPROGRAMMING)
		return;

	switch (msg->command) {
		case WKPF_COMM_CMD_GET_LOCATION: {
			// Format of get_location request messages: payload[0] offset of the first byte requested
			// Format of get_location return messages: payload[0..] the part of the location string

			// The length of the location is stored by the master as the first byte of the string.

			// Get the offset of the requested data within the location string
			uint8_t requested_offset = payload[0];

			// Read the EEPROM
			uint8_t length = wkpf_config_get_part_of_location_string((char *)payload, requested_offset, WKCOMM_MESSAGE_PAYLOAD_SIZE);

			DEBUG_LOG(DBG_WKPF, "WKPF_COMM_CMD_GET_LOCATION: Reading %d bytes at offset %d\n", length, requested_offset);

			response_cmd = WKPF_COMM_CMD_GET_LOCATION_R;
			response_size = length;
		}
		break;
		case WKPF_COMM_CMD_SET_LOCATION: {
			// Format of set_location request messages: payload[0] offset of part of the location string being sent
			// Format of set_location request messages: payload[1] the length of part of the location string being sent
			// Format of set_location request messages: payload[2..] the part of the location string
			// Format of set_location return messages: payload[0] the wkpf return code

			uint8_t written_offset = payload[0];
			uint8_t length = payload[1];

			DEBUG_LOG(DBG_WKPF, "WKPF_COMM_CMD_SET_LOCATION: Writing %d bytes at offset %d\n", length, written_offset);

			// Read the EEPROM
			retval = wkpf_config_set_part_of_location_string((char*) payload+2, written_offset, length);

			// Send response
			if (retval == WKPF_OK) {
				response_cmd = WKPF_COMM_CMD_SET_LOCATION_R;
			} else {
				response_cmd = WKPF_COMM_CMD_ERROR_R;
			}
			payload[0] = retval;       
			response_size = 1;
		}
		break;
		case WKPF_COMM_CMD_GET_FEATURES: {
			int count = 0;
			for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++) { // Needs to be changed if we have more features than fits in a single message, but for now it will work fine.
				if (wkpf_config_get_feature_enabled(i)) {
					payload[1+count++] = i;
				}
			}
			payload[0] = count;
			response_cmd = WKPF_COMM_CMD_GET_FEATURES_R;
			response_size = 1+count;
		}
		break;
		case WKPF_COMM_CMD_SET_FEATURE: {
			retval = wkpf_config_set_feature_enabled(payload[2], payload[3]);
			if (retval == WKPF_OK) {
				response_cmd = WKPF_COMM_CMD_SET_FEATURE_R;
				response_size = 0;
			} else {
				payload[2] = retval;       
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
			}
		}
		break;
		case WKPF_COMM_CMD_GET_WUCLASS_LIST: {
			// Request format: payload[0] request message number
			// Response format: payload[0] response message number
			// Response format: payload[1] total number of messages
			// Response format: payload[2] number of wuclasses

			uint8_t number_of_wuclasses = wkpf_get_number_of_wuclasses();
			uint8_t number_of_messages = (number_of_wuclasses / NUMBER_OF_WUCLASSES_PER_MESSAGE);
			if ((number_of_wuclasses % NUMBER_OF_WUCLASSES_PER_MESSAGE) != 0)
				number_of_messages++;
			uint8_t start_at_wuclass_index = payload[0]*NUMBER_OF_WUCLASSES_PER_MESSAGE;
			payload[1] = number_of_messages;
			payload[2] = number_of_wuclasses;

			uint8_t number_of_wuclasses_in_message = number_of_wuclasses - start_at_wuclass_index;
			if (number_of_wuclasses_in_message > NUMBER_OF_WUCLASSES_PER_MESSAGE)
				number_of_wuclasses_in_message = NUMBER_OF_WUCLASSES_PER_MESSAGE;

			for (uint8_t i=0; i<number_of_wuclasses_in_message; i++) {
				wuclass_t *wuclass;
				wkpf_get_wuclass_by_index(i+start_at_wuclass_index, &wuclass);

        payload[3*i + 3] = (uint8_t)(wuclass->wuclass_id >> 8);
        payload[3*i + 4] = (uint8_t)(wuclass->wuclass_id);
				if (wuclass->flags & WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE) {
					payload[3*i + 5] = WKPF_IS_VIRTUAL_WUCLASS(wuclass) ? 3 : 2;
				} else {
					payload[3*i + 5] = WKPF_IS_VIRTUAL_WUCLASS(wuclass) ? 1 : 0;
        }
			}
			response_size = 3*number_of_wuclasses_in_message + 3; // 3*wuclasses + 3 bytes for message nr, number of messages, number of wuclasses
			response_cmd = WKPF_COMM_CMD_GET_WUCLASS_LIST_R;
		}
		break;
		case WKPF_COMM_CMD_GET_WUOBJECT_LIST: {
			// Request format: payload[0] request message number
			// Response format: payload[0] response message number
			// Response format: payload[1] total number of messages
			// Response format: payload[2] number of wuobjects


			uint8_t number_of_wuobjects = wkpf_get_number_of_wuobjects();
			uint8_t number_of_wuobject_messages = (number_of_wuobjects / NUMBER_OF_WUOBJECTS_PER_MESSAGE);
			if ((number_of_wuobjects % NUMBER_OF_WUOBJECTS_PER_MESSAGE) != 0)
				number_of_wuobject_messages++;
			uint8_t start_at_wuobject_index = payload[0]*NUMBER_OF_WUOBJECTS_PER_MESSAGE;
			payload[1] = number_of_wuobject_messages;
			payload[2] = number_of_wuobjects;

			uint8_t number_of_wuobjects_in_message = number_of_wuobjects - start_at_wuobject_index;
			if (number_of_wuobjects_in_message > NUMBER_OF_WUOBJECTS_PER_MESSAGE)
				number_of_wuobjects_in_message = NUMBER_OF_WUOBJECTS_PER_MESSAGE;

			for (uint8_t i=0; i<number_of_wuobjects_in_message; i++) {
				wuobject_t *wuobject;
				wkpf_get_wuobject_by_index(start_at_wuobject_index+i, &wuobject);
				payload[4*i + 3] = (uint8_t)(wuobject->port_number);
				payload[4*i + 4] = (uint8_t)(wuobject->wuclass->wuclass_id >> 8);
				payload[4*i + 5] = (uint8_t)(wuobject->wuclass->wuclass_id);
        payload[4*i + 6] = WKPF_IS_VIRTUAL_WUCLASS(wuobject->wuclass);
			}
			response_size = 4*number_of_wuobjects_in_message + 3; // 4*wuobjects + 3 bytes for message nr, number of messages, number of wuobjects (max 39 bytes, barely over 40 bytes)
			response_cmd = WKPF_COMM_CMD_GET_WUOBJECT_LIST_R;
		}
		break;
		case WKPF_COMM_CMD_READ_PROPERTY: { // TODONR: check wuclassid
			uint8_t port_number = payload[0];
			// TODONR: uint16_t wuclass_id = (uint16_t)(payload[1]<<8)+(uint16_t)(payload[2]);
			uint8_t property_number = payload[3];
			wuobject_t *wuobject;
			retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
			if (retval != WKPF_OK) {
				payload [2] = retval;
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
				break;
			}
			uint8_t property_status;
			wkpf_get_property_status(wuobject, property_number, &property_status);
			if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT) {
				int16_t value;
				retval = wkpf_external_read_property_int16(wuobject, property_number, &value);
				payload[4] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
				payload[5] = property_status;
				payload[6] = (uint8_t)(value>>8);
				payload[7] = (uint8_t)(value);
				response_size = 8;
				response_cmd = WKPF_COMM_CMD_READ_PROPERTY_R;        
			} else if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN) {
				bool value;
				retval = wkpf_external_read_property_boolean(wuobject, property_number, &value);
				payload[4] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
				payload[5] = property_status;
				payload[6] = (uint8_t)(value);
				response_size = 7;
				response_cmd = WKPF_COMM_CMD_READ_PROPERTY_R;                
			} else if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
				wkpf_refresh_rate_t value;
				retval = wkpf_external_read_property_refresh_rate(wuobject, property_number, &value);
				payload[4] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
				payload[5] = property_status;
				payload[6] = (uint8_t)(value>>8);
				payload[7] = (uint8_t)(value);
				response_size = 8;
				response_cmd = WKPF_COMM_CMD_READ_PROPERTY_R;        
			} else
				retval = WKPF_ERR_SHOULDNT_HAPPEN;
				if (retval != WKPF_OK) {
				payload [0] = retval;
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
			}
		}
		break;
		case WKPF_COMM_CMD_WRITE_PROPERTY: {
			uint8_t port_number = payload[0];
			// TODONR: uint16_t wuclass_id = (uint16_t)(payload[1]<<8)+(uint16_t)(payload[2]);
			uint8_t property_number = payload[3];
			wuobject_t *wuobject;

			// link_entry link;
			// wkpf_get_link_by_dest_property_and_dest_wuclass_id(property_number, wuclass_id, &link);

			// // TODO: should we do that now?
			// // If the sender is not a leader
			// if (!wkpf_node_is_leader(link.src_component_id, src)) {
			// 	/*response_cmd = WKPF_COMM_CMD_ERROR_R;*/
			// 	/*response_size = 3;*/
			// 	/*break;*/
			// }

			retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
			if (retval != WKPF_OK) {
				payload[0] = retval;
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
				break;
			}
			if (payload[4] == WKPF_PROPERTY_TYPE_SHORT) {
				int16_t value;
				value = (int16_t)(payload[5]);
				value = (int16_t)(value<<8) + (int16_t)(payload[6]);
				retval = wkpf_external_write_property_int16(wuobject, property_number, value);
				response_size = 4;
				response_cmd = WKPF_COMM_CMD_WRITE_PROPERTY_R;        
			} else if (payload[4] == WKPF_PROPERTY_TYPE_BOOLEAN) {
				bool value;
				value = (bool)(payload[5]);
				retval = wkpf_external_write_property_boolean(wuobject, property_number, value);
				response_size = 4;
				response_cmd = WKPF_COMM_CMD_WRITE_PROPERTY_R;                
			} else if (payload[4] == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
				int16_t value;
				value = (int16_t)(payload[5]);
				value = (int16_t)(value<<8) + (int16_t)(payload[6]);
				retval = wkpf_external_write_property_refresh_rate(wuobject, property_number, value);
				response_size = 4;
				response_cmd = WKPF_COMM_CMD_WRITE_PROPERTY_R;
			} else
				retval = WKPF_ERR_SHOULDNT_HAPPEN;
			if (retval != WKPF_OK) {
				payload [0] = retval;
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
			}
		}
		break;
		case WKPF_COMM_CMD_REQUEST_PROPERTY_INIT: {
			uint8_t port_number = payload[0];
			uint8_t property_number = payload[1];
			wuobject_t *wuobject;

			retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
			if (retval == WKPF_OK) {
				retval = wkpf_property_needs_initialisation_push(wuobject, property_number);
			}
			if (retval != WKPF_OK) {
				payload [2] = retval;
				response_cmd = WKPF_COMM_CMD_ERROR_R;
				response_size = 1;
			} else {
				response_size = 4;
				response_cmd = WKPF_COMM_CMD_REQUEST_PROPERTY_INIT_R;                
			}
		}
		break;
	}
	if (response_cmd != 0)
		wkcomm_send_reply(msg, response_cmd, payload, response_size);
}
