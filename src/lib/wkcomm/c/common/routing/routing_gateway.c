#include "config.h"

#ifdef ROUTING_USE_GATEWAY

#include "types.h"
#include "routing.h"
#include "debug.h"
#include <string.h>
#include "../../../../wkpf/c/common/wkpf_config.h"
#include "djtimer.h"

// routing_none doesn't contain any routing protocol, but will just forward messages to the radio layer.
// Therefore, only 1 radio is allowed at a time.
// Addresses are translated 1-1 for zwave and xbee since all address types are just single bytes at the moment.
#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_XBEE)
#error Only 1 radio protocol allowed for routing_use_gateway
#endif


#define MPTN_ID_LEN                4

#define MPTN_UUID_LEN              16
#define MPTN_MAC_LEN                8

#define MPTN_MSGTYPE_LEN            1
#define MPTN_DEST_BYTE_OFFSET       0
#define MPTN_SRC_BYTE_OFFSET        MPTN_DEST_BYTE_OFFSET + MPTN_ID_LEN
#define MPTN_MSGTYPE_BYTE_OFFSET    MPTN_SRC_BYTE_OFFSET + MPTN_ID_LEN
#define MPTN_PAYLOAD_BYTE_OFFSET    MPTN_MSGTYPE_BYTE_OFFSET + MPTN_MSGTYPE_LEN

#define MPTN_MSGTYPE_GWDISCOVER     0
#define MPTN_MSGTYPE_GWOFFER        1
#define MPTN_MSGTYPE_IDREQ          2
#define MPTN_MSGTYPE_IDACK          3
#define MPTN_MSGTYPE_IDNAK          4
#define MPTN_MSGTYPE_GWIDREQ        5
#define MPTN_MSGTYPE_GWIDACK        6
#define MPTN_MSGTYPE_GWIDNAK        7

#define MPTN_MSGTYPE_RTPING         8
#define MPTN_MSGTYPE_RTREQ          9
#define MPTN_MSGTYPE_RTREP          10

#define MPTN_MSGTYPE_RPCCMD         16
#define MPTN_MSGTYPE_RPCREP         17

#define MPTN_MSGTYPE_FWDREQ         24
#define MPTN_MSGTYPE_FWDACK         25
#define MPTN_MSGTYPE_FWDNAK         26

#define MPTN_MASTER_ID           0

struct Routing_Table
{
    wkcomm_address_t my_id;
    wkcomm_address_t gateway_id;
#ifdef RADIO_USE_WIFI
    uint16_t gateway_port;
#endif
    uint8_t uuid[UUID_LENGTH];
} id_table;

#define NORMAL_MODE 0
#define GATEWAY_DISCOVERY_MODE  1
#define ID_REQ_MODE 2
int routing_mode = NORMAL_MODE;
void routing_handle_message(wkcomm_address_t wkcomm_addr, uint8_t *payload, uint8_t length);
void routing_poweron_init();
void routing_id_req();
void routing_discover_gateway();
void routing_get_value();

// ADDRESS TRANSLATION
#ifdef RADIO_USE_ZWAVE
#include "../radios/radio_zwave.h"
#define GET_ID_PREFIX(x) (x & 0xFFFFFF00)
#define GET_ID_RADIO_ADDRESS(x) (x & 0x000000FF)
radio_zwave_address_t addr_wkcomm_to_zwave(wkcomm_address_t wkcomm_addr) {
    // ZWave address is only 8 bits. To translate wkcomm address to zwave, just ignore the higher 8 bits
    // (so effectively using routing_none we can still only use 256 nodes)
    return (radio_zwave_address_t)(GET_ID_RADIO_ADDRESS(wkcomm_addr));
}
wkcomm_address_t addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr) {
    wkcomm_address_t wkcomm_addr = GET_ID_PREFIX(id_table.my_id) | zwave_addr;
    return wkcomm_addr;
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
#include "../radios/radio_xbee.h"
#define GET_ID_PREFIX(x) (x & 0xFFFF0000)
#define GET_ID_RADIO_ADDRESS(x) (x & 0x0000FFFF)
radio_xbee_address_t addr_wkcomm_to_xbee(wkcomm_address_t wkcomm_addr) {
    // XBee address is only 8 bits. To translate wkcomm address to xbee, just ignore the higher 8 bits
    // (so effectively using routing_none we can still only use 256 nodes)
    return (radio_xbee_address_t)(GET_ID_RADIO_ADDRESS(wkcomm_addr));
}
wkcomm_address_t addr_xbee_to_wkcomm(radio_xbee_address_t xbee_addr) {
    wkcomm_address_t wkcomm_addr = GET_ID_PREFIX(id_table.my_id) | xbee_addr;
    return wkcomm_addr;
}
#endif // RADIO_USE_XBEE

#ifdef RADIO_USE_WIFI
#include "../radios/radio_wifi.h"
radio_wifi_address_t prefix_mask, host_mask;
#define GET_ID_RADIO_ADDRESS(x) (x)
wkcomm_address_t GET_ID_PREFIX(wkcomm_address_t wkcomm_addr){
    return wkcomm_addr & (wkcomm_address_t)(prefix_mask);
}
radio_wifi_address_t addr_wkcomm_to_wifi(wkcomm_address_t wkcomm_addr) {
    return (radio_wifi_address_t)(wkcomm_addr);
}
wkcomm_address_t addr_wifi_to_wkcomm(radio_wifi_address_t wifi_addr) {
    return (wkcomm_address_t)(wifi_addr);
}
#endif // RADIO_USE_WIFI

#define IP_ADDRSTRLEN 16
void routing_inet_ntop(char* ipstr, wkcomm_address_t ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    snprintf(ipstr, IP_ADDRSTRLEN, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]); 
}

// MY NODE ID
// Get my own node id
wkcomm_address_t routing_get_node_id() {
    // return wkpf_config_get_myid();
    #ifdef RADIO_USE_ZWAVE
        return addr_zwave_to_wkcomm(radio_zwave_get_node_id());
    #endif
    #ifdef RADIO_USE_XBEE
        return addr_xbee_to_wkcomm(radio_xbee_get_node_id());
    #endif
    #ifdef RADIO_USE_WIFI
        return addr_wifi_to_wkcomm(radio_wifi_get_node_id());
    #endif
}

// SENDING
uint8_t routing_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
    uint8_t rt_payload[MPTN_PAYLOAD_BYTE_OFFSET+WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // 3 bytes for wkcomm
    uint8_t i;
    char ipstr[IP_ADDRSTRLEN];

    wkcomm_address_t temp_id;
    for (temp_id = dest, i = 0; i < MPTN_ID_LEN; ++i)
    {
        rt_payload[MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN-1-i] = temp_id & 0xFF;
        temp_id >>= 8;
    }
    for (temp_id = id_table.my_id, i = 0; i < MPTN_ID_LEN; ++i)
    {
        rt_payload[MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN-1-i] = temp_id & 0xFF;
        temp_id >>= 8;
    }
    rt_payload[MPTN_MSGTYPE_BYTE_OFFSET] = MPTN_MSGTYPE_FWDREQ;
    memcpy (rt_payload+MPTN_PAYLOAD_BYTE_OFFSET, payload, length);
    length += MPTN_PAYLOAD_BYTE_OFFSET;

    DEBUG_LOG(DBG_WKROUTING, "routing send packet:[");
    for (i = 0; i < length; ++i){
        DEBUG_LOG(DBG_WKROUTING, "%X", rt_payload[i]);
        DEBUG_LOG(DBG_WKROUTING, " ");
    }
    DEBUG_LOG(DBG_WKROUTING, "]\n");

    #ifdef RADIO_USE_WIFI
        dest = id_table.gateway_id;
    #else
        if (GET_ID_PREFIX(id_table.my_id) != GET_ID_PREFIX(dest) || dest == MPTN_MASTER_ID)
        {
            dest = id_table.gateway_id;
        }
    #endif
    routing_inet_ntop(ipstr, dest);
    DEBUG_LOG(DBG_WKROUTING, "routing send dest id is %s\n", ipstr);
    #ifdef RADIO_USE_ZWAVE
        return radio_zwave_send(addr_wkcomm_to_zwave(dest), rt_payload, length);
    #endif
    #ifdef RADIO_USE_XBEE
        return radio_xbee_send(addr_wkcomm_to_xbee(dest), rt_payload, length);
    #endif
    #ifdef RADIO_USE_WIFI
        return radio_wifi_send(addr_wkcomm_to_wifi(dest), rt_payload, length);
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
    #ifdef RADIO_USE_WIFI
        return radio_wifi_send_raw(addr_wkcomm_to_wifi(dest), payload, length);
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

#ifdef RADIO_USE_WIFI
void routing_handle_wifi_message(radio_wifi_address_t wifi_addr, uint8_t *payload, uint8_t length) {
    routing_handle_message(addr_wifi_to_wkcomm(wifi_addr), payload, length);
    //wkcomm_handle_message(addr_wifi_to_wkcomm(wifi_addr), payload, length);
}
#endif // RADIO_USE_WIFI


void routing_handle_message(wkcomm_address_t wkcomm_addr, uint8_t *payload, uint8_t length)
{
    wkcomm_address_t dest=0, src=0;
    uint8_t msg_type, i;
    char ipstr[IP_ADDRSTRLEN];

    DEBUG_LOG(DBG_WKROUTING, "r_handle packet:[ ");
    for (i = 0; i < length; ++i){
        DEBUG_LOG(DBG_WKROUTING, "%d", payload[i]);
        DEBUG_LOG(DBG_WKROUTING, " ");
    }
    DEBUG_LOG(DBG_WKROUTING, "] from %d\n", wkcomm_addr);


    if (length < MPTN_PAYLOAD_BYTE_OFFSET)
    {
        DEBUG_LOG(DBG_WKROUTING, "r_handle: drops garbage\n");
        return;
    }
    for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        dest <<= 8;
        dest |= payload[i];
    }
    // DEBUG_LOG(DBG_WKROUTING, "r_handle: dest_id %d\n", dest);
    for (i = MPTN_SRC_BYTE_OFFSET; i < MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        src <<= 8;
        src |= payload[i];
    }
    // DEBUG_LOG(DBG_WKROUTING, "r_handle: src_id %d\n", src);

    msg_type = payload[MPTN_MSGTYPE_BYTE_OFFSET];
    // DEBUG_LOG(DBG_WKROUTING, "r_handle: msg_type %d\n", msg_type);
    // DEBUG_LOG(DBG_WKROUTING, "r_handle: my_id %d\n", id_table.my_id);

    if (routing_mode == GATEWAY_DISCOVERY_MODE)
    {
        if (msg_type == MPTN_MSGTYPE_GWOFFER)
        {
            id_table.gateway_id = src;
            wkpf_config_set_gwid(src);
            routing_inet_ntop(ipstr, src);
            DEBUG_LOG(DBG_WKROUTING, "r_handle: get gateway id %s\n", ipstr);
            DEBUG_LOG(DBG_WKROUTING, "r_handle: get uuid=");
            for (i = 0; i < MPTN_UUID_LEN; ++i)
            {
                id_table.uuid[i] = payload[i+MPTN_PAYLOAD_BYTE_OFFSET];
                DEBUG_LOG(DBG_WKROUTING, " %d", id_table.uuid[i]);
            }
            DEBUG_LOG(DBG_WKROUTING, "\n");
            wkpf_config_set_uuid(id_table.uuid);
            routing_id_req();
        }
    }
    else if (routing_mode == ID_REQ_MODE)
    {
        if (msg_type == MPTN_MSGTYPE_IDACK && src == 0)
        {
            DEBUG_LOG(DBG_WKROUTING, "r_handle: recv IDACK packet\n");
            routing_inet_ntop(ipstr, dest);
            if (dest != id_table.my_id){
                wkpf_config_set_myid(dest);
                id_table.my_id = dest;
                DEBUG_LOG(DBG_WKROUTING, "r_handle: set id=%s\n",ipstr);
                // id_table.gateway_id = GET_ID_PREFIX(dest) | GET_ID_RADIO_ADDRESS(id_table.gateway_id);
                // wkpf_config_set_gwid(id_table.gateway_id);
            } else {
                DEBUG_LOG(DBG_WKROUTING, "r_handle: the same ID=%s\n",ipstr);
            }
            routing_mode = NORMAL_MODE;
        }
    }
    else if(dest == id_table.my_id) //packet is for current node or broadcast
    {
        DEBUG_LOG(DBG_WKROUTING, "r_handle: FWD packet\n");
        if(msg_type == MPTN_MSGTYPE_FWDREQ)
        {
            uint8_t buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // remove routing header from payload
            length -= MPTN_PAYLOAD_BYTE_OFFSET;
            memcpy (buffer, payload+MPTN_PAYLOAD_BYTE_OFFSET, length);
            wkcomm_handle_message(src, buffer, length);    //send to application
        }
    }
    else
    {
        if (msg_type == MPTN_MSGTYPE_GWDISCOVER) // propogate to itself gateway
        {
            #ifdef RADIO_USE_ZWAVE
                radio_zwave_send(addr_wkcomm_to_zwave(id_table.gateway_id), payload, MPTN_PAYLOAD_BYTE_OFFSET);
            #endif
            #ifdef RADIO_USE_XBEE
                radio_xbee_send(addr_wkcomm_to_xbee(id_table.gateway_id), payload, MPTN_PAYLOAD_BYTE_OFFSET);
            #endif
            #ifdef RADIO_USE_WIFI
                radio_wifi_send(addr_wkcomm_to_wifi(id_table.gateway_id), payload, MPTN_PAYLOAD_BYTE_OFFSET);
            #endif
        }
        DEBUG_LOG(DBG_WKROUTING, "r_handle drops unknown packet\n");
        //DEBUG_LOG(DBG_WKROUTING, "forward\n");
        //routing_send(dest, payload, length);
    }
}

// INIT
void routing_init() {
    DEBUG_LOG(DBG_WKROUTING, "r_init\n");
    #ifdef RADIO_USE_ZWAVE
        radio_zwave_init();
    #endif
    #ifdef RADIO_USE_XBEE
        radio_xbee_init();
    #endif
    #ifdef RADIO_USE_WIFI
        radio_wifi_init();
        prefix_mask = radio_wifi_get_prefix_mask();
        host_mask = ~prefix_mask;
    #endif
    routing_poweron_init();
}

void routing_poweron_init()
{
    DEBUG_LOG(DBG_WKROUTING, "r_poweron_init\n");
    id_table.my_id = wkpf_config_get_myid();
    id_table.gateway_id = wkpf_config_get_gwid();
    wkpf_config_get_uuid(id_table.uuid);
    // routing_get_mac_address();
    routing_id_req();
    // dj_timer_delay(10);
}

dj_time_t routing_search_time = 0;
#ifdef RADIO_USE_ZWAVE
#define BROADCAST_ADDRESS 0xFF
radio_zwave_address_t scan_id = BROADCAST_ADDRESS;
#endif
#ifdef RADIO_USE_XBEE
#define BROADCAST_ADDRESS 0xFFFF
radio_xbee_address_t scan_id = BROADCAST_ADDRESS;
#endif
// #ifdef RADIO_USE_WIFI
// #define BROADCAST_ADDRESS 0xFFFFFFFF
// radio_wifi_address_t scan_id = BROADCAST_ADDRESS;
// #endif
void routing_discover_gateway()
{
    // DEBUG_LOG(DBG_WKROUTING, "routing discover gateway_id before=%d\n",id_table.gateway_id);
    uint8_t rt_payload[MPTN_PAYLOAD_BYTE_OFFSET]; //Autonet MAC address
    uint8_t i;
    char ipstr[IP_ADDRSTRLEN];
    if (routing_mode != GATEWAY_DISCOVERY_MODE)
    {
        routing_mode = GATEWAY_DISCOVERY_MODE;
        id_table.my_id = routing_get_node_id();
        wkpf_config_set_myid(id_table.my_id);
#ifdef RADIO_USE_WIFI
        id_table.gateway_id = wkpf_config_get_gwid();
#else
        id_table.gateway_id = 0;
        wkpf_config_set_gwid(id_table.gateway_id);
#endif
        for (i = 0; i < MPTN_UUID_LEN; ++i)
        {
            id_table.uuid[i] = 0;
        }
        wkpf_config_set_uuid(id_table.uuid);
        routing_inet_ntop(ipstr, id_table.gateway_id);
        DEBUG_LOG(DBG_WKROUTING, "r_discover: gw_id=%s ", ipstr);
        routing_inet_ntop(ipstr, id_table.my_id);
        DEBUG_LOG(DBG_WKROUTING, "my_id=%s\n", ipstr);
    }
    for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0xFF;
    }
    wkcomm_address_t temp_id;
    for (temp_id = id_table.my_id, i = 0; i < MPTN_ID_LEN; ++i)
    {
        rt_payload[MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN-1-i] = temp_id & 0xFF;
        temp_id >>= 8;
    }
    rt_payload[MPTN_MSGTYPE_BYTE_OFFSET] = MPTN_MSGTYPE_GWDISCOVER;
    routing_search_time = dj_timer_getTimeMillis();
#ifdef RADIO_USE_ZWAVE
    radio_zwave_send(addr_wkcomm_to_zwave(scan_id++), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
    if ((scan_id & 1) ==1) radio_zwave_send(addr_wkcomm_to_zwave(BROADCAST_ADDRESS), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
#endif
#ifdef RADIO_USE_XBEE
    radio_xbee_send(addr_wkcomm_to_xbee(scan_id++), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
    if ((scan_id & 1) ==1) radio_xbee_send(addr_wkcomm_to_xbee(BROADCAST_ADDRESS), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
#endif
#ifdef RADIO_USE_WIFI
    // radio_wifi_address_t host_mask = ~prefix_mask, prefix = id_table.my_id & prefix_mask;
    // scan_id = (scan_id & host_mask) | prefix;
    radio_wifi_send(addr_wkcomm_to_wifi(id_table.gateway_id), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
    // scan_id = ((scan_id + 1) & host_mask) | prefix;
    // if ((scan_id & 1) ==1) radio_wifi_send(addr_wkcomm_to_wifi((BROADCAST_ADDRESS & host_mask) | prefix), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
#endif
}

void routing_get_mac_address()
{
    // could get MAC address as lower 8 bytes
    // uint8_t uart_data[MPTN_UUID_LEN];
    // for (uint8_t i = 0; i < MPTN_MAC_LEN; ++i)
    // {
    //     id_table.uuid[i] = uart_data[i];
    // }
}

void routing_id_req()    //send ID request
{
    char ipstr[IP_ADDRSTRLEN];
    if (routing_mode != ID_REQ_MODE)
    {
        routing_inet_ntop(ipstr, id_table.gateway_id);
        DEBUG_LOG(DBG_WKROUTING, "r_idreq: gw_id=%s ", ipstr);
        routing_inet_ntop(ipstr, id_table.my_id);
        DEBUG_LOG(DBG_WKROUTING, "my_id=%s\n", ipstr);
        routing_mode = ID_REQ_MODE;
    }
    uint8_t rt_payload[MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN];
    uint8_t i;
    for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0;
    }
    for (i = MPTN_SRC_BYTE_OFFSET; i < MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0xFF;
    }
    rt_payload[MPTN_MSGTYPE_BYTE_OFFSET] = MPTN_MSGTYPE_IDREQ;
    for (i = 0; i < MPTN_UUID_LEN; ++i)
    {
        rt_payload[i+MPTN_PAYLOAD_BYTE_OFFSET] = id_table.uuid[i];
    }
    routing_search_time = dj_timer_getTimeMillis();
#ifdef RADIO_USE_ZWAVE
    radio_zwave_send(addr_wkcomm_to_zwave(id_table.gateway_id), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN);
#endif
#ifdef RADIO_USE_XBEE
    radio_xbee_send(addr_wkcomm_to_xbee(id_table.gateway_id), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN);
#endif
#ifdef RADIO_USE_WIFI
    radio_wifi_send(addr_wkcomm_to_wifi(id_table.gateway_id), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN);
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
    if (dj_timer_getTimeMillis() - routing_search_time > 3000)
    {
        if (routing_mode == GATEWAY_DISCOVERY_MODE)
        {
            routing_discover_gateway();
        }
        else if(routing_mode == ID_REQ_MODE)
        {
            routing_id_req();
        }
    }
}

#endif // ROUTING_USE_GATEWAY


