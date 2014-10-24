#ifndef WKCOMM_WIFIH
#define WKCOMM_WIFIH

#include "types.h"

#define CMD_SET_WLAN_SSID 0
#define CMD_SET_WLAN_PASSWORD 1
#define CMD_JOIN_NETWORK 2
#define CMD_OPEN 3
#define CMD_SHOW_CONNECTION 4
#define CMD_CLOSE 5
#define CMD_SAVE 6
#define CMD_GET_IP 7

#define WIFI_UART 3

/*
static void clear_buffer();
static void show_result();
static void enter_cmd_mode();
static void exit_cmd_mode();
static void radio_wifi_send_cmd(uint8_t cmd, char* parameter);
static void radio_wifi_connect_to(char* ip, char* port);
static void radio_wifi_rece_data();
*/

void radio_wifi_initial();
uint8_t radio_wifi_send(char* ip, char* port, char* data);
void radio_wifi_poll();
void radio_wifi_get_ip(char* ip);

void radio_wifi_close();
void radio_wifi_status();

#endif // WKCOMM_WIFIH
