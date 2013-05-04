#include "config.h"

#ifdef ROUTING_USE_NONE

#include "types.h"
#include "routing.h"
#include "debug.h"


// routing_none doesn't contain any routing protocol, but will just forward messages to the radio layer.
// Therefore, only 1 radio is allowed at a time.
// Addresses are translated 1-1 for zwave and xbee since all address types are just single bytes at the moment.
#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_XBEE)
#error Only 1 radio protocol allowed for routing_none
#endif


// ADDRESS TRANSLATION
#ifdef RADIO_USE_ZWAVE
#include "../radios/radio_zwave.h"
radio_zwave_address_t addr_wkcomm_to_zwave(wkcomm_address_t wkcomm_addr) {
    return (radio_zwave_address_t)wkcomm_addr;
}
wkcomm_address_t addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr) {
	return (wkcomm_address_t)zwave_addr;
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
#include "../radios/radio_xbee.h"
radio_xbee_address_t addr_wkcomm_to_xbee(wkcomm_address_t wkcomm_addr) {
    return (radio_xbee_address_t)wkcomm_addr;
}
wkcomm_address_t addr_xbee_to_wkcomm(radio_xbee_address_t xbee_addr) {
	return (wkcomm_address_t)xbee_addr;
}
#endif // RADIO_USE_XBEE

#ifdef RADIO_USE_WIFI
#include "../radios/radio_wifi.h"
#include <stdlib.h>
uint32_t wifi_table[WIFI_TABLE_SIZE];

radio_wifi_address_t addr_wkcomm_to_wifi(wkcomm_address_t wkcomm_addr) {
    wifi_table[0]=ipstr_to_uint32("192.168.2.8");//write ip here just for test
    char ipstr[16]={0};
    char portstr[6]={0};
    ipuint32_to_str(wifi_table[wkcomm_addr], &ipstr[0]);
    itoa(WIFI_LISTEN_PORT,portstr,10);

    SOCKET s1=radio_wifi_client_connect( ipstr, portstr );
    if(s1 == INVALID_SOCKET) { 
	DEBUG_LOG(DBG_WKCOMM, "Wi-fi connect to server/client error"); 
    }
    return (radio_wifi_address_t)s1;
}
wkcomm_address_t addr_wifi_to_wkcomm(radio_wifi_address_t wifi_addr) {
	return (wkcomm_address_t)wifi_addr;
}
#endif // RADIO_USE_WIFI


// SENDING
uint8_t routing_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	#ifdef RADIO_USE_ZWAVE
		return radio_zwave_send(addr_wkcomm_to_zwave(dest), payload, length);
	#endif
	#ifdef RADIO_USE_XBEE
		return radio_xbee_send(addr_wkcomm_to_xbee(dest), payload, length);
	#endif
	#ifdef RADIO_USE_WIFI
		return radio_wifi_send(addr_wkcomm_to_wifi(dest), payload, length);
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
	wkcomm_handle_message(addr_zwave_to_wkcomm(zwave_addr), payload, length);
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
void routing_handle_xbee_message(radio_xbee_address_t xbee_addr, uint8_t *payload, uint8_t length) {
	wkcomm_handle_message(addr_xbee_to_wkcomm(xbee_addr), payload, length);
}
#endif // RADIO_USE_XBEE

#ifdef RADIO_USE_WIFI
void routing_handle_wifi_message(SOCKET wifi_addr, uint8_t *payload, uint8_t length) {
	wkcomm_handle_message(addr_wifi_to_wkcomm(wifi_addr), payload, length);
}
#endif // RADIO_USE_WIFI


// MY NODE ID
// Get my own node id
wkcomm_address_t routing_get_node_id() {
	// TODO: This doesn't work for xbee yet, but it didn't in nanovm either. What's my node ID if there's both XBee and ZWave?
	#ifdef RADIO_USE_ZWAVE
		return addr_zwave_to_wkcomm(radio_zwave_get_node_id());
	#endif
	#ifdef RADIO_USE_XBEE
		return addr_xbee_to_wkcomm(radio_xbee_get_node_id());
	#endif
	#ifdef RADIO_USE_WIFI
		return addr_wifi_to_wkcomm(radio_wifi_get_node_id());
	#endif
	return 1; // Just return 1 if we have no radios at all.
}


// INIT
void routing_init() {
	#ifdef RADIO_USE_ZWAVE
		radio_zwave_init();
	#endif
	#ifdef RADIO_USE_XBEE
		radio_xbee_init();
	#endif
	#ifdef RADIO_USE_WIFI
		if(radio_wifi_init() != WIFI_SUCCESS)
		{	DEBUG_LOG(DBG_WKCOMM, "Wi-Fi initalize error");	}
	#endif
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
	#ifdef RADIO_USE_WIFI
		radio_wifi_poll();
	#endif
}

#endif // ROUTING_USE_NONE


