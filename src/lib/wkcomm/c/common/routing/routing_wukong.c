#include "config.h"

#ifdef ROUTING_USE_WUKONG

#include "types.h"
#include "routing.h"
#include "debug.h"
#include <string.h>
#include "../../../wkpf/c/common/wkpf_config.h"
#include "../wkcomm.h"
#include "djtimer.h"


#define NETWORK_MAX_SIZE 				10
#define WIFI_TABLE_SIZE					10
#define RT_FAILED						-1
#define RT_INTERFACE_ZWAVE				0
#define RT_INTERFACE_XB					1
#define RT_INTERFACE_WIFI				2
#define RT_MSG_TYPE_APP					0
#define RT_MSG_TYPE_RT					1
#define RT_MSG_TYPE_RT_TEST				0
#define RT_MSG_TYPE_RT_ASK				1
#define RT_MSG_TYPE_RT_REPLY			2
#define RT_MSG_TYPE_RT_EVENT_NORMAL		0
#define RT_MSG_TYPE_RT_EVENT_FAILED		1
#define RT_MSG_TYPE_GID					255
#define RT_MSG_TYPE_GID_REQ 			0
#define RT_MSG_TYPE_GID_REPLY 			1
#define RT_MSG_TYPE_GID_ACK 			2
#define RT_MSG_BROADCAST			 	0xffff
#define RT_MSG_TYPE_RT_ASK_ALL			255
#define RT_HANDLE_FAILED_SEND			0
#define RT_HANDLE_FAILED_RECV			1
#define RT_PERIOD_CHECK					100
#define RT_PERIOD_UPDATE				3000
#define RT_PERIOD_FAIL_TIMEOUT			3000
#define RT_PERIOD_FAIL_GIVEUP			10000
#define RT_POLICY_ENERGY				0
#define RT_POLICY_THROUGHPUT			1


struct Routing_Table
{
	uint16_t GID;
	uint16_t nexthop_GID;
	uint16_t nexthop_LID;
	uint8_t  nexthop_interface;
	uint8_t seq;
	int32_t score;
	uint8_t dirty;
} rt_table[NETWORK_MAX_SIZE];

struct Failed_List
{
	uint16_t GID;
	uint16_t nexthop_GID;
	uint8_t  nexthop_interface;
	uint8_t *payload;
	uint8_t payload_length;
	uint32_t timeout;
} rt_failed_list[NETWORK_MAX_SIZE];

struct Wifi_Table
{
	uint32_t ip;
	uint8_t cid;
} wifi_table[WIFI_TABLE_SIZE];

uint8_t flag_fail=0, flag_dirty=0, wifi_ok=1;
uint8_t	current_policy =0;
uint32_t check_time=0, updata_time=0;

uint8_t wifi_table_to_ip(uint16_t dest_LID);
void wifi_server_disconnect(uint8_t cid);
uint16_t routing_lookup_next_hop(wkcomm_address_t dest_gid, uint8_t *nexthop_interface);
int32_t routing_lookup_score(wkcomm_address_t lookup_gid);
uint8_t routing_lookup_seq(wkcomm_address_t lookup_gid);
uint8_t routing_lookup_index(wkcomm_address_t lookup_gid);
void routing_handle_message(uint16_t local_id, uint8_t *payload, uint8_t length, uint8_t interface);
uint32_t current_score(uint8_t interface);
void routing_poweron_init();
void routing_gid_req();
void routing_broadcast_test();
void routing_broadcast_ask(wkcomm_address_t ask_gid, uint8_t event_type);
void failed_handle(	uint16_t dst_GID, uint16_t prv_GID, uint16_t prv_LID, uint8_t  prv_interface,
					uint16_t nexthop_GID, uint8_t  nexthop_interface, 
					int32_t score, uint16_t nexthop_LID, uint8_t *payload, uint8_t payload_length, uint8_t handle_type);
void period_update();

// routing_none doesn't contain any routing protocol, but will just forward messages to the radio layer.
// Therefore, only 1 radio is allowed at a time.
// Addresses are translated 1-1 for zwave and xbee since all address types are just single bytes at the moment.
// #if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_XBEE)
// #error Only 1 radio protocol allowed for routing_none
// #endif


// ADDRESS TRANSLATION
#ifdef RADIO_USE_ZWAVE
#include "../radios/radio_zwave.h"
radio_zwave_address_t addr_wkcomm_to_zwave(wkcomm_address_t wkcomm_addr) {
	// ZWave address is only 8 bits. To translate wkcomm address to zwave, just ignore the higher 8 bits
	// (so effectively using routing_none we can still only use 256 nodes)
    return (radio_zwave_address_t)(wkcomm_addr & 0xFF);
}
wkcomm_address_t addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr) {
	return (wkcomm_address_t)zwave_addr;
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
#include "../radios/radio_xbee.h"
radio_xbee_address_t addr_wkcomm_to_xbee(wkcomm_address_t wkcomm_addr) {
	// XBee address is only 8 bits. To translate wkcomm address to xbee, just ignore the higher 8 bits
	// (so effectively using routing_none we can still only use 256 nodes)
    return (radio_xbee_address_t)(wkcomm_addr & 0xFF);
}
wkcomm_address_t addr_xbee_to_wkcomm(radio_xbee_address_t xbee_addr) {
	return (wkcomm_address_t)xbee_addr;
}
#endif // RADIO_USE_XBEE

#ifdef RADIO_USE_WIFI
#include "../../arduino/radios/radio_wifi.h"
#include <stdlib.h>


wkcomm_address_t addr_wifi_to_wkcomm(radio_wifi_address_t wifi_addr) {
	return (wkcomm_address_t)wifi_addr;
}

uint8_t wifi_table_to_ip(uint16_t dest_LID)
{
	SOCKET s1=wifi_table[dest_LID].cid;
	if(s1==INVALID_SOCKET)//connection is not established
	{
	    char ipstr[16]={0};
	    char portstr[6]={0};
	    ipuint32_to_str(wifi_table[dest_LID].ip, &ipstr[0]);	    
	    itoa(WIFI_LISTEN_PORT,portstr,10);
	    s1=radio_wifi_client_connect( ipstr, portstr );
	    wifi_table[dest_LID].cid = s1;
	    if(s1 == INVALID_SOCKET) 
	    { 
			DEBUG_LOG(DBG_WKCOMM, "Wi-fi server error\n"); 
    	}
	}

    return (radio_wifi_address_t)s1;
}

void wifi_server_disconnect(uint8_t cid)
{
	for(uint16_t i=0;i<WIFI_TABLE_SIZE;i++)
	{
		if(wifi_table[i].cid==cid)
		{
			wifi_table[i].cid = INVALID_SOCKET;
			break;
		}
	}
}

#endif // RADIO_USE_WIFI


// SENDING
uint8_t routing_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	wkcomm_address_t my_gid = routing_get_node_id();
	uint8_t nexthop_interface;
	uint16_t dest_LID;
	dest_LID = routing_lookup_next_hop(dest, &nexthop_interface);

	if(dest_LID==RT_FAILED)
	{ 
		DEBUG_LOG(DBG_WKROUTING, "no path\n");
		return RT_FAILED;
	}

	uint8_t rt_payload[WKCOMM_MESSAGE_PAYLOAD_SIZE+3+5]; // 3 bytes for wkcomm, 5 for the routing
	rt_payload[0] = (uint8_t)(dest >> 8);
 	rt_payload[1] = (uint8_t)(dest & 0xff);
	rt_payload[2] = (uint8_t)(my_gid >> 8);
	rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
 	rt_payload[4] = RT_MSG_TYPE_APP;
	memcpy (rt_payload+5, payload, length);


	if(nexthop_interface==RT_INTERFACE_WIFI)
	{
		#ifdef RADIO_USE_WIFI
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = wifi_table_to_ip(dest_LID);//return cid 			
				if(ret!=RT_FAILED)//connection is established
				{
					radio_wifi_send(ret, rt_payload, length+5);
					ret = 0;//return success
					break;
				}					
				else//send failed, delay then resend
				{
					DEBUG_LOG(DBG_WKROUTING, "resend=%d\n",i+1);
					dj_timer_delay(10);
					if(i==2)
					{
						uint8_t table_index = routing_lookup_index(dest);
						failed_handle(dest, 0,0,0, rt_table[table_index].nexthop_GID, nexthop_interface, 0, 0, rt_payload, length+5, RT_HANDLE_FAILED_SEND);
					}	
				}
			}
			return ret;
		#endif
	}
	else if(nexthop_interface==RT_INTERFACE_ZWAVE)
	{
		#ifdef RADIO_USE_ZWAVE
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = radio_zwave_send((dest_LID & 0xff), rt_payload, length+5);
				if(ret!=RT_FAILED)//send success
					break;
				else//send failed, delay then resend
				{
					DEBUG_LOG(DBG_WKROUTING, "resend=%d\n",i+1);
					dj_timer_delay(10);
					if(i==2)
					{
						uint8_t table_index = routing_lookup_index(dest);
						failed_handle(dest, 0,0,0, rt_table[table_index].nexthop_GID, nexthop_interface, 0, 0, rt_payload, length+5, RT_HANDLE_FAILED_SEND);
					}	
				}
			}
			return ret;
		#endif		
	}
	else if(nexthop_interface==RT_INTERFACE_XB)
	{
		#ifdef RADIO_USE_XBEE
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = radio_xbee_send((dest_LID & 0xff), rt_payload, length+5);
				if(ret!=RT_FAILED)//send success
					break;
				else//send failed, delay then resend
					dj_timer_delay(10);
			}
			return ret;
		#endif
	}

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

uint8_t routing_control_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
	
	//send debug
	// for(int i=0;i<length;i++)
	// {
	// 	DEBUG_LOG(DBG_WKROUTING, "send:i=%d,%d\n ",i,payload[i]);
	// 	dj_timer_delay(10);
	// }

	if(dest == RT_MSG_BROADCAST)
	{
		int8_t ret;
		for(uint8_t i=0;i<3;i++)//resend max three times
		{
			ret = radio_zwave_send(0xff, payload, length);
			if(ret!=RT_FAILED)//send success
				break;
			else//send failed, delay then resend
			{
				DEBUG_LOG(DBG_WKROUTING, "resend=%d\n",i+1);
				dj_timer_delay(10);
			}
		}
		return ret;
	}

	uint8_t nexthop_interface;
	uint16_t dest_LID;
	dest_LID = routing_lookup_next_hop(dest, &nexthop_interface);
	if(dest_LID==RT_FAILED)
	{ 
		DEBUG_LOG(DBG_WKROUTING, "no path\n");
		return RT_FAILED;
	}

	if(nexthop_interface==RT_INTERFACE_WIFI)
	{
		#ifdef RADIO_USE_WIFI
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = wifi_table_to_ip(dest_LID);//return cid 			
				if(ret!=RT_FAILED)//connection is established
				{
					radio_wifi_send(ret, payload, length);
					ret = 0;//return success
					break;
				}					
				else//send failed, delay then resend
				{
					DEBUG_LOG(DBG_WKROUTING, "resend=%d\n",i+1);
					dj_timer_delay(10);
					if(i==2)
					{
						uint8_t table_index = routing_lookup_index(dest);
						failed_handle(dest, 0,0,0, rt_table[table_index].nexthop_GID, nexthop_interface, 0, 0, payload, length, RT_HANDLE_FAILED_SEND);
					}	
				}
			}
			return ret;	
		#endif
	}
	else if(nexthop_interface==RT_INTERFACE_ZWAVE)
	{
		#ifdef RADIO_USE_ZWAVE
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = radio_zwave_send((dest_LID & 0xff), payload, length);
				if(ret!=RT_FAILED)//send success
					break;
				else//send failed, delay then resend
				{
					DEBUG_LOG(DBG_WKROUTING, "resend=%d\n",i+1);
					dj_timer_delay(10);
					if(i==2)
					{
						uint8_t table_index = routing_lookup_index(dest);
						failed_handle(dest, 0,0,0, rt_table[table_index].nexthop_GID, nexthop_interface, 0, 0, payload, length, RT_HANDLE_FAILED_SEND);
					}	
				}
			}
			return ret;
		#endif						
	}
	else if(nexthop_interface==RT_INTERFACE_XB)
	{
		#ifdef RADIO_USE_XBEE
			int8_t ret;
			for(uint8_t i=0;i<3;i++)//resend max three times
			{
				ret = radio_xbee_send((dest_LID & 0xff), payload, length);
				if(ret!=RT_FAILED)//send success
					break;
				else//send failed, delay then resend
					dj_timer_delay(10);
			}
			return ret;
		#endif
	}

	return 0;
}


// RECEIVING
	// Since this library doesn't contain any routing, we just always pass the message up to wkcomm.
	// In a real routing library there will probably be some messages to maintain the routing protocol
	// state that could be handled here, while messages meant for higher layers like wkpf and wkreprog
	// should be sent up to wkcomm.
#ifdef RADIO_USE_ZWAVE
void routing_handle_zwave_message(radio_zwave_address_t zwave_addr, uint8_t *payload, uint8_t length) {
	routing_handle_message((uint16_t)zwave_addr, payload, length, RT_INTERFACE_ZWAVE);	
}
#endif // RADIO_USE_ZWAVE

#ifdef RADIO_USE_XBEE
void routing_handle_xbee_message(radio_xbee_address_t xbee_addr, uint8_t *payload, uint8_t length) {
	//routing_handle_message(addr_xbee_to_wkcomm(xbee_addr), payload, length, RT_INTERFACE_XBEE);	
	//wkcomm_handle_message(addr_xbee_to_wkcomm(xbee_addr), payload, length);
}
#endif // RADIO_USE_XBEE

#ifdef RADIO_USE_WIFI
void routing_handle_wifi_message(uint8_t wifi_cid, uint8_t *payload, uint8_t length) {
	uint16_t wifi_lid=RT_FAILED;
	for(uint16_t i=0;i<WIFI_TABLE_SIZE;i++)
	{
		if(wifi_table[i].cid==wifi_cid)
		{
			wifi_lid = i;
			break;
		}
	}
	routing_handle_message(wifi_lid, payload, length, RT_INTERFACE_WIFI);	
	//wkcomm_handle_message(addr_wifi_to_wkcomm(wifi_addr), payload, length);
}
#endif // RADIO_USE_WIFI


// MY NODE ID
// Get my own node id
wkcomm_address_t routing_get_node_id() {
	return wkpf_config_get_gid();
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
		{	
			wifi_ok =0;
			DEBUG_LOG(DBG_WKROUTING, "Wifi init error\n");	
		}
	#endif
	routing_poweron_init();
}


// POLL
// Call this periodically to receive data
// In a real routing protocol we could use a timer here
// to periodically send heartbeat messages etc.
void routing_poll() {
	if(dj_timer_getTimeMillis()>check_time)
	{
		period_update();
		check_time = dj_timer_getTimeMillis()+RT_PERIOD_CHECK;
	}
	#ifdef RADIO_USE_ZWAVE
		radio_zwave_poll();
	#endif
	#ifdef RADIO_USE_XBEE
		radio_xbee_poll();
	#endif
	#ifdef RADIO_USE_WIFI
		if(wifi_ok)
		{
			radio_wifi_poll();			
		}
	#endif
}


uint16_t routing_lookup_next_hop(wkcomm_address_t dest_gid, uint8_t *nexthop_interface)
{
	int i;
	for(i=0;i<NETWORK_MAX_SIZE;i++)
	{
		if(dest_gid==rt_table[i].GID)
		{
			uint16_t ret;
			ret = rt_table[i].nexthop_LID;
			*nexthop_interface = rt_table[i].nexthop_interface;
			return ret;
		}
	}
	return RT_FAILED;
}

int32_t routing_lookup_score(wkcomm_address_t lookup_gid)
{
	int i;
	for(i=0;i<NETWORK_MAX_SIZE;i++)
	{
		if(lookup_gid==rt_table[i].GID)
		{
			int32_t ret;
			ret = rt_table[i].score;
			return ret;
		}
	}
	return RT_FAILED;
}

uint8_t routing_lookup_seq(wkcomm_address_t lookup_gid)
{
	int i;
	for(i=0;i<NETWORK_MAX_SIZE;i++)
	{
		if(lookup_gid==rt_table[i].GID)
		{
			uint8_t ret;
			ret = rt_table[i].seq;
			return ret;
		}
	}
	return RT_FAILED;
}

uint8_t routing_lookup_index(wkcomm_address_t lookup_gid)
{
	int i;
	for(i=0;i<NETWORK_MAX_SIZE;i++)
	{
		if(lookup_gid==rt_table[i].GID)
		{
			return i;
		}
	}
	return RT_FAILED;
}

uint32_t current_score(uint8_t interface)
{
	if(current_policy==RT_POLICY_ENERGY)
	{
		if(interface==RT_INTERFACE_ZWAVE) 
			return 1;
		else if(interface==RT_INTERFACE_WIFI) 
			return 5;
		// else if(interface==RT_INTERFACE_XB) 
		// 	return 30;
	}
	else if(current_policy==RT_POLICY_THROUGHPUT)
	{
		if(interface==RT_INTERFACE_ZWAVE) 
			return 10;
		else if(interface==RT_INTERFACE_WIFI) 
			return 1;
		// else if(interface==RT_INTERFACE_XB) 
		// 	return 30;
	}
	return RT_FAILED;
}

void routing_handle_message(uint16_t local_id, uint8_t *payload, uint8_t length, uint8_t interface) 
{
	wkcomm_address_t dst,src,my_gid;
	dst = (payload[0]<<8) | payload[1];
	src = (payload[2]<<8) | payload[3];
	my_gid = routing_get_node_id();
	uint8_t msg_type = payload[4];


	// for(int i=0;i<length;i++)
	// {
	// 	DEBUG_LOG(DBG_WKROUTING, "recv:i=%d,%d\n ",i,payload[i]); 
	// 	dj_timer_delay(10);
	// }

	if(	dst==0 && src==1 && 
		msg_type == RT_MSG_TYPE_GID && 
		payload[5]==RT_MSG_TYPE_GID_REPLY) //this is GID reply message
	{
		wkcomm_address_t new_gid= (payload[6]<<8) + (payload[7]);
		wkpf_config_set_gid(new_gid);	
		uint8_t buffer[6]={0};
		buffer[1] = 0x01;
		buffer[2] = (uint8_t)new_gid>>8;
		buffer[3] = (uint8_t)new_gid&0xff;
		buffer[4] = RT_MSG_TYPE_GID;
		buffer[5] = RT_MSG_TYPE_GID_ACK;
		radio_zwave_send(addr_wkcomm_to_zwave(1), buffer, 6);
		DEBUG_LOG(DBG_WKROUTING, "set GID=%d\n",routing_get_node_id());
	}
	else if(dst==my_gid || dst==RT_MSG_BROADCAST)//packet is for current node or broadcast
	{
		if(msg_type==RT_MSG_TYPE_APP)
		{
			uint8_t buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // remove routing header from payload
			memcpy (buffer, payload+5, length-5);
			
			wkcomm_handle_message(src, buffer, length-5);	//send to application	
		}
		else if(msg_type==RT_MSG_TYPE_RT)
		{
			uint8_t subtype = payload[5];
			uint8_t event_type = payload[6];
			uint8_t seq	= payload[7];
			if(subtype==RT_MSG_TYPE_RT_ASK)
			{
				uint8_t total = payload[8];
				if(event_type==RT_MSG_TYPE_RT_EVENT_NORMAL)	
				{
					if(total==RT_MSG_TYPE_RT_ASK_ALL)//ask all table
					{
						for(int i=0;i<NETWORK_MAX_SIZE;i++)
						{
							if(rt_table[i].GID==0)
							{
								total=i;
								break;
							}
							else if(i==NETWORK_MAX_SIZE-1)
							{
								total = NETWORK_MAX_SIZE;
								break;
							}
						}
						uint8_t *rt_payload = malloc(total*6+9);
						rt_payload[0] = (uint8_t)(src >> 8);//send message back to source
						rt_payload[1] = (uint8_t)(src & 0xff);
						rt_payload[2] = (uint8_t)(my_gid >> 8);
						rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
						rt_payload[4] = RT_MSG_TYPE_RT;
						rt_payload[5] = RT_MSG_TYPE_RT_REPLY;
						rt_payload[6] = RT_MSG_TYPE_RT_EVENT_NORMAL;
						rt_payload[7] = seq;

						uint8_t rt_length=9;
						for(int i=0;i<total;i++)
						{
							rt_payload[rt_length] = (uint8_t)(rt_table[i].GID>>8);//reply my routing table
							rt_payload[rt_length+1] = (uint8_t)(rt_table[i].GID & 0xff);
							rt_payload[rt_length+2] = (uint8_t)(rt_table[i].score>>24);
							rt_payload[rt_length+3] = (uint8_t)(rt_table[i].score>>16);
							rt_payload[rt_length+4] = (uint8_t)(rt_table[i].score>>8);
							rt_payload[rt_length+5] = (uint8_t)(rt_table[i].score & 0xff);
							rt_length+=6;
						}

						rt_payload[8] = (rt_length-9)/6;//total exist in routing table
						routing_control_send(src, rt_payload, rt_length);
						free(rt_payload); 
					}
					else
					{
						uint8_t *rt_payload = malloc(total*6+9);
						rt_payload[0] = (uint8_t)(src >> 8);//send message back to source
						rt_payload[1] = (uint8_t)(src & 0xff);
						rt_payload[2] = (uint8_t)(my_gid >> 8);
						rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
						rt_payload[4] = RT_MSG_TYPE_RT;
						rt_payload[5] = RT_MSG_TYPE_RT_REPLY;
						rt_payload[6] = RT_MSG_TYPE_RT_EVENT_NORMAL;
						rt_payload[7] = seq;

						uint8_t rt_length=9;
						for(int i=9;i<total*2+9;i=i+2)
						{
							wkcomm_address_t ask_gid;
							int32_t val;
							ask_gid = (payload[i]<<8) | payload[i+1];
							val = routing_lookup_score(ask_gid);
							if(val!=RT_FAILED)//exist ask GID in table
							{
								rt_payload[rt_length] = (uint8_t)(ask_gid>>8);
								rt_payload[rt_length+1] = (uint8_t)(ask_gid & 0xff);
								rt_payload[rt_length+2] = (uint8_t)(val>>24);
								rt_payload[rt_length+3] = (uint8_t)(val>>16);
								rt_payload[rt_length+4] = (uint8_t)(val>>8);
								rt_payload[rt_length+5] = (uint8_t)(val & 0xff);
								rt_length+=6;
							}


						}
						rt_payload[8] = (rt_length-9)/6;//total exist in routing table
						routing_control_send(src, rt_payload, rt_length);
						free(rt_payload); 
					}										
				}
				else if(event_type==RT_MSG_TYPE_RT_EVENT_FAILED)	
				{										
					wkcomm_address_t ask_gid = (payload[9]<<8) | payload[10];
					uint8_t ask_index = routing_lookup_index(ask_gid);
					if(ask_index==RT_FAILED)//no answer in my table
					{	return;	}
					else//exist answer in my table
					{
						uint8_t *rt_payload = malloc(20);
						rt_payload[0] = (uint8_t)(src >> 8);//send message back to source
						rt_payload[1] = (uint8_t)(src & 0xff);
						rt_payload[2] = (uint8_t)(my_gid >> 8);
						rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
						rt_payload[4] = RT_MSG_TYPE_RT;
						rt_payload[5] = RT_MSG_TYPE_RT_REPLY;
						rt_payload[6] = RT_MSG_TYPE_RT_EVENT_FAILED;
						rt_payload[7] = seq;
						rt_payload[8] = 1;//failed event, total=1 
						rt_payload[9] = (uint8_t)(ask_gid>>8);//destination GID
						rt_payload[10] = (uint8_t)(ask_gid & 0xff);
						rt_payload[11] = (uint8_t)(rt_table[ask_index].nexthop_GID>>8);//next hop GID
						rt_payload[12] = (uint8_t)(rt_table[ask_index].nexthop_GID & 0xff);
						rt_payload[13] = (uint8_t)(rt_table[ask_index].nexthop_interface);//next hop interface
						rt_payload[14] = (uint8_t)(rt_table[ask_index].score>>24);//path score
						rt_payload[15] = (uint8_t)(rt_table[ask_index].score>>16);
						rt_payload[16] = (uint8_t)(rt_table[ask_index].score>>8);
						rt_payload[17] = (uint8_t)(rt_table[ask_index].score & 0xff);	
						rt_payload[18] = (uint8_t)(rt_table[ask_index].nexthop_LID>>8);//next hop GID
						rt_payload[19] = (uint8_t)(rt_table[ask_index].nexthop_LID & 0xff);											
						routing_control_send(src, rt_payload, 20);
						free(rt_payload); 
					}
				}
			}
			else if(subtype==RT_MSG_TYPE_RT_REPLY)
			{			
				uint8_t total = payload[8];
				if(event_type==RT_MSG_TYPE_RT_EVENT_NORMAL)
				{
					uint8_t rt_length=9;
					for(int i=0;i<total;i++)
					{
						wkcomm_address_t reply_gid;
						uint32_t reply_score,table_score;
						reply_gid = (uint16_t)(payload[rt_length]<<8) + (uint16_t)(payload[rt_length+1] & 0xff);
						if(reply_gid==my_gid)
						{
							rt_length+=6;
							continue;
						}
						reply_score = 	((uint32_t)(payload[rt_length+2])<<24) + 
										((uint32_t)(payload[rt_length+3])<<16) +
										((uint32_t)(payload[rt_length+4])<<8) +
										((uint32_t)(payload[rt_length+5]) & 0xff);
						reply_score += current_score(interface);
						table_score = routing_lookup_score(reply_gid);

						uint8_t index;
						if(table_score == RT_FAILED) //create a new row
						{
							index = routing_lookup_index(0);
						}
						else //update current row
						{
							index = routing_lookup_index(reply_gid);
						}

						if((table_score==RT_FAILED) || (reply_score < table_score) ||
							rt_table[index].nexthop_GID==src)//no score or better score or nexthop=previours hop, write into table
						{
							rt_table[index].GID = reply_gid;//update routing table
							rt_table[index].nexthop_GID = src;					
							rt_table[index].nexthop_LID = local_id;
							rt_table[index].nexthop_interface = interface;
							rt_table[index].seq = seq;
							rt_table[index].score = reply_score;
							rt_table[index].dirty = 1;
							flag_dirty =1;
						}
						rt_length+=6;
					}
				}
				else if(event_type==RT_MSG_TYPE_RT_EVENT_FAILED)	
				{
					wkcomm_address_t ask_gid,reply_nexthop_gid;
					uint16_t reply_nexthop_lid;
					uint8_t reply_interface;
					uint32_t reply_score;
					ask_gid = (payload[9]<<8) | payload[10];
					reply_nexthop_gid = (payload[11]<<8) | payload[12];
					reply_interface = payload[13];
					reply_score = 	((uint32_t)(payload[14])<<24) + 
									((uint32_t)(payload[15])<<16) +
									((uint32_t)(payload[16])<<8) +
									((uint32_t)(payload[17]) & 0xff);
					reply_score += current_score(interface);
					reply_nexthop_lid = (payload[18]<<8) | payload[19];

					failed_handle(ask_gid, src, local_id, interface, reply_nexthop_gid, reply_interface, reply_score, reply_nexthop_lid, NULL, 0, RT_HANDLE_FAILED_RECV);
				}					
			}
			else if(subtype==RT_MSG_TYPE_RT_TEST)
			{
				if(src == my_gid) return; //ignore broadcast message from myself
				uint32_t path_score = 	((uint32_t)(payload[8])<<24) + 
										((uint32_t)(payload[9])<<16) +
										((uint32_t)(payload[10])<<8) +
										((uint32_t)(payload[11]) & 0xff);
				path_score += current_score(interface);
				uint32_t table_score = routing_lookup_score(src);
				uint8_t table_seq = routing_lookup_seq(src);
				if(	(table_score == RT_FAILED) 	|| //no path in routing table
					(table_seq != seq) 			|| //new update
					((table_seq == seq) && (path_score<table_score)))//better path
				{
					uint8_t index;
					if(table_score == RT_FAILED) //create a new row
					{
						index = routing_lookup_index(0);
					}
					else //update current row
					{
						index = routing_lookup_index(src);
					}
					rt_table[index].GID = src;//update routing table
					rt_table[index].nexthop_GID = (uint16_t)(payload[8]<<8) + (uint16_t)(payload[9] & 0xff);					
					rt_table[index].nexthop_LID = local_id;
					rt_table[index].nexthop_interface = interface;
					rt_table[index].seq = seq;
					rt_table[index].score = path_score;

					routing_lookup_index(src);

					uint8_t *rt_payload = malloc(14);
					rt_payload[0] = (uint8_t)(RT_MSG_BROADCAST >> 8);
					rt_payload[1] = (uint8_t)(RT_MSG_BROADCAST & 0xff);
					rt_payload[2] = (uint8_t)(src >> 8);
					rt_payload[3] = (uint8_t)(src & 0xff); 	
					rt_payload[4] = RT_MSG_TYPE_RT; 
					rt_payload[5] = RT_MSG_TYPE_RT_TEST;
					rt_payload[6] = RT_MSG_TYPE_RT_EVENT_NORMAL;
					rt_payload[7] = seq;	
					rt_payload[8] = (uint8_t)(my_gid >> 8);
					rt_payload[9] = (uint8_t)(my_gid & 0xff);
					rt_payload[10] = (uint8_t)(path_score >> 24);
					rt_payload[11] = (uint8_t)(path_score >> 16);
					rt_payload[12] = (uint8_t)(path_score >> 8);
					rt_payload[13] = (uint8_t)(path_score & 0xff);
					routing_control_send(RT_MSG_BROADCAST, rt_payload, 14);
					free(rt_payload); 
				}
			}
		}
	}
	else
	{
		DEBUG_LOG(DBG_WKROUTING, "forward\n");
		routing_control_send(dst, payload, length);
	}

	// for(int i=0;i<NETWORK_MAX_SIZE;i++)//print table debug
	// {
	// 	DEBUG_LOG(DBG_WKROUTING, "table:gid=%d,ng=%d,nl=%d,ni=%d,seq=%d,sco=%d\n ",
	// 		rt_table[i].GID,rt_table[i].nexthop_GID,rt_table[i].nexthop_LID,
	// 		rt_table[i].nexthop_interface,rt_table[i].seq,rt_table[i].score); 
	// 	dj_timer_delay(10);
	// 	if(rt_table[i].GID==0)
	// 	{	break;	}
	// }

}

void routing_poweron_init()
{
	wkcomm_address_t my_gid=routing_get_node_id();
	for(int i=0;i<WIFI_TABLE_SIZE;i++)
	{
		wifi_table[i].cid = INVALID_SOCKET;
	}

	current_policy = RT_POLICY_ENERGY;


  //   wifi_table[0].ip = ipstr_to_uint32("192.168.2.8");//write ip here just for test

 	routing_gid_req();
 	dj_timer_delay(10);
	rt_table[0].GID = my_gid;
 	rt_table[0].nexthop_GID = my_gid;
 	rt_table[0].nexthop_LID = 0;
 	rt_table[0].nexthop_interface =  RT_INTERFACE_ZWAVE;//don't care
 	rt_table[0].seq =  0;
 	rt_table[0].score =  0;
 	routing_broadcast_test();
 	dj_timer_delay(10);
 	routing_broadcast_ask(RT_MSG_TYPE_RT_ASK_ALL, RT_MSG_TYPE_RT_EVENT_NORMAL);


}

void routing_gid_req()	//send GID request
{
	wkcomm_address_t my_gid = routing_get_node_id();
	DEBUG_LOG(DBG_WKROUTING, "my_gid=%d\n",my_gid);
	uint8_t *rt_payload = malloc(6);
	rt_payload[0] = 0;	
	rt_payload[1] = 1;	//bridge LID
	rt_payload[2] = (uint8_t)my_gid>>8;
	rt_payload[3] = (uint8_t)my_gid&0xff;
	rt_payload[4] = RT_MSG_TYPE_GID;
	rt_payload[5] = RT_MSG_TYPE_GID_REQ;
	radio_zwave_send(addr_wkcomm_to_zwave(1), rt_payload, 6);
	//routing_control_send(RT_MSG_BROADCAST, rt_payload, 6);	
	free(rt_payload); 
}

void routing_broadcast_test()	////broadcast test
{
	wkcomm_address_t my_gid = routing_get_node_id();
	uint8_t *rt_payload = malloc(14);
	rt_payload[0] = (uint8_t)(RT_MSG_BROADCAST >> 8);//send message back to source
	rt_payload[1] = (uint8_t)(RT_MSG_BROADCAST & 0xff);
	rt_payload[2] = (uint8_t)(my_gid >> 8);
	rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
	rt_payload[4] = RT_MSG_TYPE_RT; 
	rt_payload[5] = RT_MSG_TYPE_RT_TEST;
	rt_payload[6] = RT_MSG_TYPE_RT_EVENT_NORMAL;
	rt_payload[7] = dj_timer_getTimeMillis()%256;//random seq number	
	rt_payload[8] = (uint8_t)(my_gid >> 8);
	rt_payload[9] = (uint8_t)(my_gid & 0xff);
	rt_payload[10] = 0;//no score while initalize
	rt_payload[11] = 0;
	rt_payload[12] = 0;
	rt_payload[13] = 0;
	routing_control_send(RT_MSG_BROADCAST, rt_payload, 14);
	free(rt_payload); 
}

void routing_broadcast_ask(wkcomm_address_t ask_gid, uint8_t event_type)	////broadcast ask
{
	wkcomm_address_t my_gid = routing_get_node_id();
	uint8_t *rt_payload = malloc(11);
	rt_payload[0] = (uint8_t)(RT_MSG_BROADCAST >> 8);//send message back to source
	rt_payload[1] = (uint8_t)(RT_MSG_BROADCAST & 0xff);
	rt_payload[2] = (uint8_t)my_gid>>8;
	rt_payload[3] = (uint8_t)my_gid&0xff;
	rt_payload[4] = RT_MSG_TYPE_RT;
	rt_payload[5] = RT_MSG_TYPE_RT_ASK;
	rt_payload[6] = event_type;
	rt_payload[7] = dj_timer_getTimeMillis()%256;//random seq number
	if(ask_gid==RT_MSG_TYPE_RT_ASK_ALL)	
	{
		rt_payload[8] = RT_MSG_TYPE_RT_ASK_ALL;//total	
		routing_control_send(RT_MSG_BROADCAST, rt_payload, 9);
	}
	else
	{
		rt_payload[8] = 1;//failed event, total=1 
		rt_payload[9] = (uint8_t)(ask_gid>>8);//destination GID
		rt_payload[10] = (uint8_t)(ask_gid & 0xff);
		routing_control_send(RT_MSG_BROADCAST, rt_payload, 11);
	}
	free(rt_payload); 
}

void failed_handle(	uint16_t dst_GID, uint16_t prv_GID, uint16_t prv_LID, uint8_t  prv_interface, 
					uint16_t nexthop_GID, uint8_t  nexthop_interface, int32_t score, 
					uint16_t nexthop_LID, uint8_t *payload, uint8_t payload_length, uint8_t handle_type)
{
	if(handle_type==RT_HANDLE_FAILED_RECV)//call from handle
	{
		uint8_t index = RT_FAILED;
		for(int i=0;i<NETWORK_MAX_SIZE;i++)//find available entry
		{
			if(rt_failed_list[i].GID==dst_GID)
			{
				index = i;
				break;
			}
		}	
		if(index == RT_FAILED)// no available memory
			return;


		if( (nexthop_GID != rt_failed_list[index].nexthop_GID ||//the same nexthop interface or the same nexthop
			nexthop_interface != rt_failed_list[index].nexthop_interface) && 
			nexthop_GID != routing_get_node_id() //next hop can not be current node
			)
		{
			uint8_t table_index = routing_lookup_index(rt_failed_list[index].GID);
			if(table_index==RT_FAILED)//not found in routing table
				return;

			if( rt_table[table_index].nexthop_GID==rt_failed_list[index].nexthop_GID &&
				rt_table[table_index].nexthop_interface==rt_failed_list[index].nexthop_interface)//old routing table, update immediately
			{
				rt_table[table_index].nexthop_GID = prv_GID;
				rt_table[table_index].score = score;
				rt_table[table_index].nexthop_interface = prv_interface;
				rt_table[table_index].nexthop_LID = prv_LID;
				rt_table[table_index].dirty = 1;
				flag_dirty =1;
			}
			else if(score < rt_table[table_index].score)//new table value, compare score
			{
				rt_table[table_index].nexthop_GID = prv_GID;
				rt_table[table_index].score = score;
				rt_table[table_index].nexthop_interface = prv_interface;
				rt_table[table_index].nexthop_LID = prv_LID;
				rt_table[table_index].dirty = 1;
				flag_dirty =1;
			}
		}
	}
	else if(handle_type==RT_HANDLE_FAILED_SEND)//call from send
	{
		uint8_t index = RT_FAILED;
		for(int i=0;i<NETWORK_MAX_SIZE;i++)//find available entry
		{
			if(rt_failed_list[i].GID==0)
			{
				index = i;
				break;
			}
		}		
		if(index == RT_FAILED)// no available memory
			return;

		flag_fail = 1;
		rt_failed_list[index].GID = dst_GID;
		rt_failed_list[index].nexthop_GID = nexthop_GID;
		rt_failed_list[index].nexthop_interface = nexthop_interface;
		uint8_t *buffer = malloc(payload_length);
		memcpy( buffer, payload, payload_length);
		rt_failed_list[index].payload = buffer;
		rt_failed_list[index].payload_length = payload_length;
		routing_broadcast_ask(dst_GID, RT_MSG_TYPE_RT_EVENT_FAILED);
		rt_failed_list[index].timeout = dj_timer_getTimeMillis()+RT_PERIOD_FAIL_TIMEOUT;				
	}

}

void period_update()
{
	if(flag_fail==1)
	{
		uint8_t empty_count=0;
		for(uint8_t i=0;i<NETWORK_MAX_SIZE;i++)
		{
			if(rt_failed_list[i].GID!=0 )//fail table is not empty
			{
				if(dj_timer_getTimeMillis()>rt_failed_list[i].timeout)//is time out
				{
					uint8_t table_index;
					table_index = routing_lookup_index(rt_failed_list[i].GID);
					if( rt_failed_list[i].nexthop_GID!=rt_table[table_index].nexthop_GID ||	//the same nexthop or the same nexthop interface
						rt_failed_list[i].nexthop_interface!=rt_table[table_index].nexthop_interface )
					{
						routing_control_send(rt_failed_list[i].GID, rt_failed_list[i].payload, rt_failed_list[i].payload_length);
						free(rt_failed_list[i].payload);
						rt_failed_list[i].GID=0;
						rt_failed_list[i].nexthop_GID=0;
						rt_failed_list[i].nexthop_interface=0;
						rt_failed_list[i].payload=NULL;
						rt_failed_list[i].payload_length=0;
						rt_failed_list[i].timeout=0;
					}
					else//no new path after asking
					{
						DEBUG_LOG(DBG_WKROUTING, "unreachable=%d\n",rt_failed_list[i].GID);													
						if(dj_timer_getTimeMillis()>rt_failed_list[i].timeout+RT_PERIOD_FAIL_GIVEUP)//time out give up
						{
							free(rt_failed_list[i].payload);
							rt_failed_list[i].GID=0;
							rt_failed_list[i].nexthop_GID=0;
							rt_failed_list[i].nexthop_interface=0;
							rt_failed_list[i].payload=NULL;
							rt_failed_list[i].payload_length=0;
							rt_failed_list[i].timeout=0;
						}
					}
				}
			}
			else
			{
				empty_count++;
				if(empty_count==NETWORK_MAX_SIZE)
				{
					flag_fail=0;
				}
			}
		}

	}
	if(flag_dirty==1 && dj_timer_getTimeMillis()>updata_time)
	{
		uint8_t total=0;
		for(int i=0;i<NETWORK_MAX_SIZE;i++)
		{
			if(rt_table[i].dirty==1)
			{
				total++;
			}
		}
		wkcomm_address_t my_gid = routing_get_node_id();
		uint8_t *rt_payload = malloc(total*6+9);
		rt_payload[0] = (uint8_t)(RT_MSG_BROADCAST >> 8);
		rt_payload[1] = (uint8_t)(RT_MSG_BROADCAST & 0xff);
		rt_payload[2] = (uint8_t)(my_gid >> 8);
		rt_payload[3] = (uint8_t)(my_gid & 0xff); 	
		rt_payload[4] = RT_MSG_TYPE_RT;
		rt_payload[5] = RT_MSG_TYPE_RT_REPLY;
		rt_payload[6] = RT_MSG_TYPE_RT_EVENT_NORMAL;
		rt_payload[7] = dj_timer_getTimeMillis()%256;//random seq number	

		uint8_t rt_length=9;
		for(int i=0;i<NETWORK_MAX_SIZE;i++)
		{
			if(rt_table[i].dirty==1)
			{
				rt_payload[rt_length] = (uint8_t)(rt_table[i].GID>>8);
				rt_payload[rt_length+1] = (uint8_t)(rt_table[i].GID & 0xff);
				rt_payload[rt_length+2] = (uint8_t)(rt_table[i].score>>24);
				rt_payload[rt_length+3] = (uint8_t)(rt_table[i].score>>16);
				rt_payload[rt_length+4] = (uint8_t)(rt_table[i].score>>8);
				rt_payload[rt_length+5] = (uint8_t)(rt_table[i].score & 0xff);
				rt_length+=6;		
			}
		}
		rt_payload[8] = total;
		routing_control_send(RT_MSG_BROADCAST, rt_payload, rt_length);
		free(rt_payload); 
		flag_dirty=0;
		updata_time = dj_timer_getTimeMillis() + RT_PERIOD_UPDATE;
	}
}

#endif // ROUTING_USE_WUKONG