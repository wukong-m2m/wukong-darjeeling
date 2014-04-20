#include "config.h"

#ifdef ROUTING_USE_GATEWAY

#include "types.h"
#include "routing.h"
#include "debug.h"
#include <string.h>
#include "../../../wkpf/c/common/wkpf_config.h"
#include "djtimer.h"

// routing_none doesn't contain any routing protocol, but will just forward messages to the radio layer.
// Therefore, only 1 radio is allowed at a time.
// Addresses are translated 1-1 for zwave and xbee since all address types are just single bytes at the moment.
#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_XBEE)
#error Only 1 radio protocol allowed for routing_use_gateway
#endif

#define MULT_PROTO_LEN_DID              4
#define MULT_PROTO_LEN_MSG_TYPE         1
#define MULT_PROTO_LEN_MSG_SUBTYPE      1
#define MULT_PROTO_DEST_BYTE_OFFSET     0
#define MULT_PROTO_SRC_BYTE_OFFSET      MULT_PROTO_DEST_BYTE_OFFSET + MULT_PROTO_LEN_DID
#define MULT_PROTO_MSG_TYPE_BYTE_OFFSET MULT_PROTO_SRC_BYTE_OFFSET + MULT_PROTO_LEN_DID
#define MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET MULT_PROTO_MSG_TYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_TYPE
#define MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_SUBTYPE

#define MULT_PROTO_LEN_MAC				8

#define MULT_PROTO_MSG_TYPE_DID         0x01
#define MULT_PROTO_MSG_SUBTYPE_DID_REQ  0x00
#define MULT_PROTO_MSG_SUBTYPE_DID_UPD  0x01
#define MULT_PROTO_MSG_SUBTYPE_DID_OFFR 0x02
#define MULT_PROTO_MSG_SUBTYPE_DID_ACK  0x03
#define MULT_PROTO_MSG_SUBTYPE_DID_NAK  0x04

#define MULT_PROTO_MSG_TYPE_FWD         0x03
#define MULT_PROTO_MSG_SUBTYPE_FWD      0x00
#define MULT_PROTO_MSG_SUBTYPE_FWD_ACK  0x01
#define MULT_PROTO_MSG_SUBTYPE_FWD_NAK  0x02

struct Routing_Table
{
	wkcomm_address_t my_did;
	wkcomm_address_t gateway_did;
	uint16_t gateway_tcp_port;
	uint8_t mac_address[8];
} did_table;

void routing_handle_message(wkcomm_address_t wkcomm_addr, uint8_t *payload, uint8_t length);
void routing_poweron_init();
void routing_did_req();
void routing_get_gateway_did();
void routing_get_mac_address();

// MY NODE ID
// Get my own node id
wkcomm_address_t routing_get_node_id() {
	return wkpf_config_get_did();
}

// ADDRESS TRANSLATION
#ifdef RADIO_USE_ZWAVE
#include "../radios/radio_zwave.h"
radio_zwave_address_t addr_wkcomm_to_zwave(wkcomm_address_t wkcomm_addr) {
	// ZWave address is only 8 bits. To translate wkcomm address to zwave, just ignore the higher 8 bits
	// (so effectively using routing_none we can still only use 256 nodes)
    return (radio_zwave_address_t)(wkcomm_addr & 0xFF);
}
wkcomm_address_t addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr) {
	wkcomm_address_t wkcomm_addr = (did_table.my_did & ~0xFF) | zwave_addr;
	return wkcomm_addr;
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
#include "../radios/radio_xbee.h"
radio_xbee_address_t addr_wkcomm_to_xbee(wkcomm_address_t wkcomm_addr) {
	// XBee address is only 8 bits. To translate wkcomm address to xbee, just ignore the higher 8 bits
	// (so effectively using routing_none we can still only use 256 nodes)
    return (radio_xbee_address_t)(wkcomm_addr & 0xFFFF);
}
wkcomm_address_t addr_xbee_to_wkcomm(radio_xbee_address_t xbee_addr) {
	wkcomm_address_t wkcomm_addr = (did_table.my_did & ~0xFFFF) | xbee_addr;
	return wkcomm_addr;
}
#endif // RADIO_USE_XBEE

// SENDING
uint8_t routing_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	uint8_t rt_payload[MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET+WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // 3 bytes for wkcomm
	uint8_t i;
	wkcomm_address_t temp_did;
#ifdef RADIO_USE_ZWAVE
	if ((did_table.my_did & ~0xFF) != (dest & ~0xFF))
#endif // RADIO_USE_ZWAVE
#ifdef RADIO_USE_XBEE
	if ((did_table.my_did & ~0xFFFF) != (dest & ~0xFFFF))
#endif // RADIO_USE_XBEE
	{
		dest = did_table.gateway_did;
	}
	for (temp_did = dest, i = MULT_PROTO_DEST_BYTE_OFFSET+MULT_PROTO_LEN_DID-1; i >= MULT_PROTO_DEST_BYTE_OFFSET; --i, temp_did>>=8)
	{
		rt_payload[i] = temp_did & 0xFF;
	}
	for (temp_did = did_table.my_did, i = MULT_PROTO_SRC_BYTE_OFFSET+MULT_PROTO_LEN_DID-1; i >= MULT_PROTO_SRC_BYTE_OFFSET; --i, temp_did>>=8)
	{
		rt_payload[i] = temp_did & 0xFF;
	}
	rt_payload[MULT_PROTO_MSG_TYPE_BYTE_OFFSET] = MULT_PROTO_MSG_TYPE_FWD;
	rt_payload[MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET] = MULT_PROTO_MSG_SUBTYPE_FWD;
	memcpy (rt_payload+MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET, payload, length);
	length += MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET;

	#ifdef RADIO_USE_ZWAVE
		return radio_zwave_send(addr_wkcomm_to_zwave(dest), payload, length);
	#endif
	#ifdef RADIO_USE_XBEE
		return radio_xbee_send(addr_wkcomm_to_xbee(dest), payload, length);
	#endif
	return 0;
}
uint8_t routing_send_raw(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	#ifdef RADIO_USE_ZWAVE
		return radio_zwave_send_raw(addr_wkcomm_to_zwave(dest), payload, length);
	#endif
	#ifdef RADIO_USE_XBEE
		return radio_xbee_send_raw(addr_wkcomm_to_xbee(dest), payload, length);
	#endif
	return 0;
}


// RECEIVING
	// Since this library doesn't contain any routing, we just always pass the message up to wkcomm.
	// In a real routing library there will probably be some messages to maintain the routing protocol
	// state that could be handled here, while messages meant for higher layers like wkpf and wkreprog
	// should be sent up to wkcomm.
#ifdef RADIO_USE_ZWAVE
void routing_handle_zwave_message(radio_zwave_address_t zwave_addr, uint8_t *payload, uint8_t length) {
	routing_handle_message(addr_zwave_to_wkcomm(zwave_addr), payload, length);
	//wkcomm_handle_message(addr_zwave_to_wkcomm(zwave_addr), payload, length);
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
void routing_handle_xbee_message(radio_xbee_address_t xbee_addr, uint8_t *payload, uint8_t length) {
	routing_handle_message(addr_xbee_to_wkcomm(xbee_addr), payload, length);
	//wkcomm_handle_message(addr_xbee_to_wkcomm(xbee_addr), payload, length);
}
#endif // RADIO_USE_XBEE

void routing_handle_message(wkcomm_address_t wkcomm_addr, uint8_t *payload, uint8_t length) 
{
	wkcomm_address_t dest=0, src=0;
	uint8_t msg_type, msg_subtype, i;
	if (length < MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET)
	{
		DEBUG_LOG(DBG_WKROUTING, "drop garbage packet\n");
	}

	for (i = MULT_PROTO_DEST_BYTE_OFFSET+MULT_PROTO_LEN_DID-1; i >= MULT_PROTO_DEST_BYTE_OFFSET; --i, dest<<=8)
	{
		dest |= payload[i];
	}
	for (i = MULT_PROTO_SRC_BYTE_OFFSET+MULT_PROTO_LEN_DID-1; i >= MULT_PROTO_SRC_BYTE_OFFSET; --i, src<<=8)
	{
		src |= payload[i];
	}
	msg_type = payload[MULT_PROTO_MSG_TYPE_BYTE_OFFSET];
	msg_subtype = payload[MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET];

	if (msg_type == MULT_PROTO_MSG_TYPE_DID &&
		msg_subtype == MULT_PROTO_MSG_SUBTYPE_DID_OFFR &&
		src == 0)
	{
		if (dest != did_table.my_did){
			wkpf_config_set_did(dest);
			DEBUG_LOG(DBG_WKROUTING, "set DID=%d\n",routing_get_node_id());
		} else {
			DEBUG_LOG(DBG_WKROUTING, "the same DID=%d\n",dest);
		}
	}
	else if(dest == did_table.my_did) //packet is for current node or broadcast
	{
		if(msg_type == MULT_PROTO_MSG_TYPE_FWD)
		{
			if(msg_subtype == MULT_PROTO_MSG_SUBTYPE_FWD){
				uint8_t buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // remove routing header from payload
				length -= MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET;
				memcpy (buffer, payload+MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET, length);				
				wkcomm_handle_message(src, buffer, length);	//send to application
			}
		}			
	}
	else
	{
		DEBUG_LOG(DBG_WKROUTING, "drop unknown packet\n");
		//DEBUG_LOG(DBG_WKROUTING, "forward\n");
		//routing_send(dest, payload, length);
	}
}

// INIT
void routing_init() {
	#ifdef RADIO_USE_ZWAVE
		radio_zwave_init();
	#endif
	#ifdef RADIO_USE_XBEE
		radio_xbee_init();
	#endif
	routing_poweron_init();
}

void routing_poweron_init()
{
	did_table.my_did = routing_get_node_id();
	routing_get_mac_address();
 	routing_get_gateway_did();
 	dj_timer_delay(10);
 	routing_did_req();
 	dj_timer_delay(10);
}

void routing_get_gateway_did()
{
	#ifdef RADIO_USE_ZWAVE
		did_table.gateway_did = 1;
	#endif
	#ifdef RADIO_USE_XBEE
		dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	#endif
}

void routing_get_mac_address()
{
	// get MAC address
	uint8_t uart_data[MULT_PROTO_LEN_MAC];
	for (uint8_t i = 0; i < MULT_PROTO_LEN_MAC; ++i)
	{
		did_table.mac_address[i] = uart_data[i];
	}
}

void routing_did_req()	//send DID request
{
	DEBUG_LOG(DBG_WKROUTING, "my_did=%d\n",did_table.my_did);
	uint8_t rt_payload[MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET + MULT_PROTO_LEN_MAC]; //Autonet MAC address
	uint8_t i;
	for (i = MULT_PROTO_DEST_BYTE_OFFSET; i < MULT_PROTO_DEST_BYTE_OFFSET+MULT_PROTO_LEN_DID; ++i)
	{
		rt_payload[i] = 0;	
	}
	for (i = MULT_PROTO_SRC_BYTE_OFFSET; i < MULT_PROTO_SRC_BYTE_OFFSET+MULT_PROTO_LEN_DID; ++i)
	{
		rt_payload[i] = 0xFF;
	}
	rt_payload[MULT_PROTO_MSG_TYPE_BYTE_OFFSET] = MULT_PROTO_MSG_TYPE_DID;
	rt_payload[MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET] = MULT_PROTO_MSG_SUBTYPE_DID_REQ;

	routing_send_raw(did_table.gateway_did, rt_payload, MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET + MULT_PROTO_LEN_MAC);
}

// POLL
// Call this periodically to receive data
// In a real routing protocol we could use a timer here
// to periodically send heartbeat messages etc.
void routing_poll() {
	#ifdef RADIO_USE_ZWAVE
		radio_zwave_poll();
	#endif
	#ifdef RADIO_USE_XBEE
		radio_xbee_poll();
	#endif
}

#endif // ROUTING_USE_GATEWAY


