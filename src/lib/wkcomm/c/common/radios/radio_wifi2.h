#ifndef WKCOMM_WIFIH
#define WKCOMM_WIFIH

//#include <HardwareSerial.h>
//#include <Arduino.h>

#include "uart.h"
#include "types.h"


#define WIFI_UART        3
//#define WIFI_BAUDRATE 	 9600
 #define WIFI_BAUDRATE 	 115200
#define WIFI_LISTEN_PORT 80
#define WIFI_SUCCESS	 0
#define WIFI_FAILED	 255

#define MAX_SOCK_NUM 	4

// command identifiers
// config
#define CMD_DISABLE_ECHO 0
#define CMD_SET_WPA	 1
#define CMD_SET_SSID     2
#define CMD_DISABLE_DHCP 3
#define CMD_ENABLE_DHCP  4
#define CMD_DISCONNECT   5
#define CMD_LISTEN       6
#define CMD_TCP_CONN     7
#define CMD_GET_MAC_ADDR 8
#define CMD_DNS_LOOKUP   9
#define CMD_CLOSE_CONN   10
#define CMD_NETWORK_SET  11
#define CMD_WIRELESS_MODE 12
#define CMD_ENABLE_DHCPSVR 13
#define CMD_SET_WEP      14
#define CMD_GET_CID      15
#define CMD_SET_UART_BAUDRATE 16
#define CMD_SAVE_PROFILE 17
#define CMD_SET_DEFAULT_PROFILE 18

// device operation modes
#define DEV_OP_MODE_COMMAND 0
#define DEV_OP_MODE_DATA    1
#define DEV_OP_MODE_DATA_RX 2

// device wireless connection state
#define DEV_CONN_ST_DISCONNECTED 0
#define DEV_CONN_ST_CONNECTED    1

// connection ID
#define INVALID_CID 255
#define INVALID_SOCKET 255

#define IPPROTO_TCP 6

// wifi mode
#define NORMAL_MODE     0
#define ADHOC_MODE      1
#define AP_MODE         2

// socket status
#define SOCK_STATUS_CLOSED 0
#define SOCK_STATUS_INIT    1
#define SOCK_STATUS_LISTEN 2
#define SOCK_STATUS_ESTABLISHED    3
#define SOCK_STATUS_CLOSE_WAIT 4



typedef uint8_t SOCKET;
typedef uint8_t radio_wifi_address_t;

typedef struct _SOCK_TABLE {
	uint8_t status;
	uint8_t protocol;
	uint16_t port;//source port is no use
	uint8_t cid;
} SOCK_TABLE;


extern uint8_t radio_wifi_init();
extern uint8_t radio_wifi_send(radio_wifi_address_t dst, uint8_t *buf, uint16_t  len);
extern uint8_t radio_wifi_client_connect(char dst_ip[], char dst_port[]); //return socket
extern void radio_wifi_poll(void);
extern radio_wifi_address_t radio_wifi_get_node_id();

extern uint32_t ipstr_to_uint32(char* ipstr);
extern void ipuint32_to_str(uint32_t ipint, char * ipstr);

uint8_t wifi_send_cmd_w_resp(uint8_t cmd);
uint8_t wifi_send_cmd(uint8_t cmd);
uint8_t wifi_parse_resp(uint8_t cmd);
uint8_t wifi_connect_AP();
uint8_t wifi_isDataOnSock(SOCKET s);
uint8_t wifi_connect_socket(char ip[], char port[]);
uint8_t wifi_readSocketStatus(SOCKET s);
void wifi_configSocket(SOCKET s, uint8_t protocol, uint16_t port);
void wifi_process();
void wifi_parse_cmd(char buf[]);
void wifi_readline(char *strBuf);
uint8_t wifi_get_remoteIP_fromCID(uint8_t cid, char* ip);
void radio_wifi_recvData();
uint8_t wifi_client_status(SOCKET s);
uint8_t wifi_client_check_connected(SOCKET s);
void wifi_client_stop(uint8_t cid); 
uint8_t wifi_server_listen(SOCKET s);
uint8_t wifi_available();

#endif // WKCOMM_WIFIH


