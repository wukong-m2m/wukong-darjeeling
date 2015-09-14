#ifndef WKCOMM_WIFIH
#define WKCOMM_WIFIH

#include "types.h"

#define CMD_SET_WLAN_SSID 	0
#define CMD_SET_WLAN_PASSWORD 	1
#define CMD_JOIN_NETWORK 	2
#define CMD_OPEN 		3
#define CMD_SHOW_CONNECTION 	4
#define CMD_CLOSE 		5
#define CMD_SAVE 		6
#define CMD_GET_NET_MASK_AND_IP	7
#define CMD_SET_IP_PROTO_THREE 	8
#define CMD_SET_IP_HOST 	9
#define CMD_SET_WLAN_JOIN_ONE 	10

#define WIFI_UART 		3
#define SHOW_RESULT		false
//typedef char* radio_wifi_address_t;
//extern uint32_t my_ip;
//extern uint32_t my_net_mask;

void radio_wifi_init();
void radio_wifi_send(uint32_t ip, unsigned char* data, uint8_t length);
void radio_wifi_send_without_ip(unsigned char* data, uint8_t length);
void radio_wifi_poll();
void radio_wifi_get_net_mask_and_ip();
extern void routing_handle_wifi_message(uint32_t wifi_addr, uint8_t *payload, uint8_t length);

#endif // WKCOMM_WIFIH
