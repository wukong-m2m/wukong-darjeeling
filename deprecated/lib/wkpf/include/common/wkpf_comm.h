#ifndef WKPF_COMM_H
#define WKPF_COMM_H

#include "wkcomm.h"
#include "wkpf.h"

extern uint8_t wkpf_send_set_property_int16(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, int16_t value);
extern uint8_t wkpf_send_set_property_boolean(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, bool value);
extern uint8_t wkpf_send_set_property_refresh_rate(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, wkpf_refresh_rate_t value);
extern uint8_t wkpf_send_request_property_init(wkcomm_address_t dest_node_id, uint8_t port_number, uint8_t property_number);
extern void wkpf_comm_handle_message(void *msg); // Will be called with a pointer to a wkcomm_received_msg

// Message types
#define WKPF_COMM_CMD_GET_WUCLASS_LIST            0x90
#define WKPF_COMM_CMD_GET_WUCLASS_LIST_R          0x91
#define WKPF_COMM_CMD_GET_WUOBJECT_LIST           0x92
#define WKPF_COMM_CMD_GET_WUOBJECT_LIST_R         0x93
#define WKPF_COMM_CMD_READ_PROPERTY               0x94
#define WKPF_COMM_CMD_READ_PROPERTY_R             0x95
#define WKPF_COMM_CMD_WRITE_PROPERTY              0x96
#define WKPF_COMM_CMD_WRITE_PROPERTY_R            0x97
#define WKPF_COMM_CMD_REQUEST_PROPERTY_INIT       0x98
#define WKPF_COMM_CMD_REQUEST_PROPERTY_INIT_R     0x99
#define WKPF_COMM_CMD_GET_LOCATION                0x9A
#define WKPF_COMM_CMD_GET_LOCATION_R              0x9B
#define WKPF_COMM_CMD_SET_LOCATION                0x9C
#define WKPF_COMM_CMD_SET_LOCATION_R              0x9D
#define WKPF_COMM_CMD_GET_FEATURES                0x9E
#define WKPF_COMM_CMD_GET_FEATURES_R              0x9F
#define WKPF_COMM_CMD_SET_FEATURE                 0xA0
#define WKPF_COMM_CMD_SET_FEATURE_R               0xA1
#define WKPF_COMM_CMD_ERROR_R                     0xAF

#define DEVICE_NATIVE_ZWAVE_SWITCH 64
#define DEVICE_NATIVE_ZWAVE_DIMMER 65
#define DEVICE_NATIVE_ZWAVE_CURTAIN 66
#define DEVICE_NATIVE_ZWAVE_SIMPLE_AV 67

#endif // WKPF_COMM_H

