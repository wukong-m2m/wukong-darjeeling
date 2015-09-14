#include "config.h"
#ifdef RADIO_USE_WIFI_ARDUINO
#include "radio_wifi1.h"
#include "djtimer.h"
#include "uart.h"
#include "debug.h"
//#include "../../common/routing/routing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#define output_high(port, pin) port |= (1<<pin)
//#define output_low(port, pin) port &= ~(1<<pin)
uint32_t my_ip = 0;
uint32_t my_net_mask = 0;

char wifi_ssid[] = "1581449";
char wifi_security_key[] = "77121328";
//char wifi_ssid[] = "intel_wukong";
//char wifi_security_key[] = "wukong2012";
//char wifi_ssid[] = "dlink";
//char wifi_security_key[] = "my22346292";
//char wifi_ssid[] = "ICS";
//char wifi_security_key[] = "intel2012";

static uint32_t inet_pton_wifi(char* ip_str){
	uint8_t i, num_of_bits = 0;
	unsigned char byte_c[3] = {0};
	uint32_t ip_num = 0;

	for(i = 0; i <= strlen(ip_str); i++){
		if(ip_str[i] == '.' || ip_str[i] == 0){
			ip_num *= 256;
			switch(num_of_bits){
				case 1:
					ip_num += (byte_c[0] - 48);
					break;
				case 2:
					ip_num += (byte_c[0] - 48) * 10 + (byte_c[1] - 48);
					break;
				case 3:
					ip_num += (byte_c[0] - 48) * 100 + (byte_c[1] - 48) * 10 + (byte_c[2] - 48);
					break;
			}
			memset(byte_c, 0, sizeof(byte_c));
			num_of_bits = 0;
		}else{
			byte_c[num_of_bits++] = (unsigned char)ip_str[i];
		}

	}
	return ip_num;
}

static void inet_ntop_wifi(char* ip_str, uint32_t ip)
{
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	sprintf(ip_str, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]); 
}

static uint8_t wifi_wait_and_read(uint16_t wait_ms){
	if(uart_available(WIFI_UART, wait_ms)){
		return uart_read_byte(WIFI_UART);
	}
	return 0;
}

static void clear_buffer(){
	if( uart_available(WIFI_UART, 1000) ){
		uart_read_byte(WIFI_UART);
		while(uart_available(WIFI_UART, 100)) uart_read_byte(WIFI_UART);
	}
}

static void show_result(){
	if( uart_available(WIFI_UART, 1000) ){
		DEBUG_LOG(DBG_WIFI_ARDUINO, "%c", uart_read_byte(WIFI_UART));
		while(uart_available(WIFI_UART, 100))
			DEBUG_LOG(DBG_WIFI_ARDUINO, "%c", uart_read_byte(WIFI_UART));
	}
	DEBUG_LOG(DBG_WIFI_ARDUINO, "\n");
}

static void enter_cmd_mode(){
	uint8_t i;
	char temp[]="$$$";
	dj_timer_delay(250);
	for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
	dj_timer_delay(250);

	if(SHOW_RESULT){
		show_result();
	}else{
		clear_buffer();
	}
}

static void exit_cmd_mode(){
	uint8_t i;
	char temp[]="exit";
	for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
	uart_write_byte(WIFI_UART, '\r');

	if(SHOW_RESULT){
		show_result();
	}else{
		clear_buffer();
	}
}

static void radio_wifi_send_cmd(uint8_t cmd, char* parameter){
	uint8_t i;
	switch (cmd) {
		case CMD_SET_WLAN_SSID:
		{
			char cmd_buf[35]={0};
			strcat(cmd_buf, "set wlan ssid ");
			strcat(cmd_buf, wifi_ssid);
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;
		}
		case CMD_SET_WLAN_PASSWORD:
		{
			char cmd_buf[35]={0};
			strcat(cmd_buf, "set wlan phrase ");
			strcat(cmd_buf, wifi_security_key);
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;
		}
		case CMD_JOIN_NETWORK:
		{

			char cmd_buf[35]={0};
			strcat(cmd_buf, "join ");
			strcat(cmd_buf, wifi_ssid);
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');
			dj_timer_delay(100);

		      	if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;

		}
		case CMD_OPEN:
		{

			char cmd_buf[35]={0};
			strcat(cmd_buf, "open ");
			strcat(cmd_buf, parameter);
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');
			dj_timer_delay(100);

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;

		}
		case CMD_SHOW_CONNECTION:
		{

			char cmd_buf[]="show connection";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');
			//clear_buffer();
			//show_result();
			break;

		}
		case CMD_CLOSE:
		{

			char cmd_buf[]="close";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;

		}
		case CMD_SAVE:
		{
			char cmd_buf[]="save";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
			break;
		}
		case CMD_GET_NET_MASK_AND_IP:
		{
			char cmd_buf[]="get ip";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');
			//clear_buffer();
			//show_result();
			break;
		}
		case CMD_SET_IP_PROTO_THREE:
		{
			char cmd_buf[]="set ip proto 3";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
		}
		case CMD_SET_IP_HOST:
		{
			char cmd_buf[35]={0};
			strcat(cmd_buf, "set ip host ");
			strcat(cmd_buf, parameter);
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
		}
		case CMD_SET_WLAN_JOIN_ONE:
		{
			char cmd_buf[]="set wlan join 1";
			for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
			uart_write_byte(WIFI_UART, '\r');

			if(SHOW_RESULT){
				show_result();
			}else{
				clear_buffer();
			}
		}
	}
}

void radio_wifi_init(){
	//turn on wifi module on wudevice 3.0
	DDRL |= (1<<5);
	//PORTL &= ~(1<<5);
	PORTL |= (1<<5);

	uart_inituart(WIFI_UART, 9600);  
	enter_cmd_mode();
	radio_wifi_send_cmd(CMD_SET_WLAN_SSID, NULL);
	radio_wifi_send_cmd(CMD_SET_WLAN_PASSWORD, NULL);
	radio_wifi_send_cmd(CMD_SET_IP_PROTO_THREE, NULL);
	radio_wifi_send_cmd(CMD_SET_WLAN_JOIN_ONE, NULL);
	radio_wifi_send_cmd(CMD_SAVE, NULL);
	//radio_wifi_send_cmd(CMD_JOIN_NETWORK, NULL);
	exit_cmd_mode();
	radio_wifi_get_net_mask_and_ip();

	DEBUG_LOG(DBG_WIFI_ARDUINO, "\nmy_ip = %u.%u.%u.%u, my_net_mask = %u.%u.%u.%u\n", 
	(uint8_t)(my_ip >> 24 & 0xFF), (uint8_t)(my_ip >> 16 & 0xFF), (uint8_t)(my_ip >> 8 & 0xFF), (uint8_t)(my_ip & 0xFF), 
	(uint8_t)(my_net_mask >> 24 & 0xFF), (uint8_t)(my_net_mask >> 16 & 0xFF), 
	(uint8_t)(my_net_mask >> 8 & 0xFF), (uint8_t)(my_net_mask & 0xFF));
}

static void clear_line(){	
	while(uart_available(WIFI_UART, 100)){
		if(uart_read_byte(WIFI_UART) == '\n') break;
	}
}

void radio_wifi_get_net_mask_and_ip(){
	char temp[7] = {0};
	uint8_t i = 0;
	enter_cmd_mode();
	radio_wifi_send_cmd(CMD_GET_NET_MASK_AND_IP, NULL);
	while(uart_available(WIFI_UART, 0) == false);
	temp[i] = uart_read_byte(WIFI_UART);
	while(uart_available(WIFI_UART, 100) && temp[i] != '\n'){
		temp[++i] = uart_read_byte(WIFI_UART);
	}

	if(strncmp(temp, "get ip", strlen("get ip")) == 0){
		for(i = 1; i <= 2; i++) clear_line();
		//get ip
		char* ip = (char*)malloc(16);
		memset(ip, 0, 16);
		for(i = 1; i <= 3; i++){
			if(uart_available(WIFI_UART, 100)) uart_read_byte(WIFI_UART);
		}
		i = 0;
		while(uart_available(WIFI_UART, 100)){
			unsigned char temp2 = uart_read_byte(WIFI_UART);
			if(temp2 == ':') {
				ip[i]='\0';
				clear_line();
				break;
			}else{
				ip[i++] = temp2;
			}
		}
		my_ip = inet_pton_wifi(ip);
		free(ip);

		//get net mask
		char* net_mask = (char*)malloc(16);
		memset(net_mask, 0, 16);
		for(i = 1; i <= 3; i++){
			if(uart_available(WIFI_UART, 100)) uart_read_byte(WIFI_UART);
		}
		i = 0;
		while(uart_available(WIFI_UART, 100)){
			unsigned char temp2 = uart_read_byte(WIFI_UART);
			if(temp2 == '\n') {
				net_mask[--i]='\0';
				clear_buffer();
				break;
			}else{
				net_mask[i++] = temp2;
			}
		}
		my_net_mask = inet_pton_wifi(net_mask);
		free(net_mask);
	}else{
		clear_buffer();
	}
	exit_cmd_mode();
}

static void send_wukong_pattern(){
	uart_write_byte(WIFI_UART, 'W');
	uart_write_byte(WIFI_UART, 'K');
}

void radio_wifi_send(uint32_t ip, unsigned char* data, uint8_t length){
	char* ip_str = (char*)malloc(16);
	memset(ip_str, 0, 16);
	inet_ntop_wifi(ip_str, ip);
	enter_cmd_mode();
	radio_wifi_send_cmd(CMD_SET_IP_HOST, ip_str);
	exit_cmd_mode();
	free(ip_str);
	//send pattern
	send_wukong_pattern();
	//transmit data
  	uart_write_byte(WIFI_UART, length);
	uint8_t i;
	for(i = 0; i < length; i++) uart_write_byte(WIFI_UART, data[i]);
	//send ip
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 24 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 16 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 8 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip & 0xFF));
}

void radio_wifi_send_without_ip(unsigned char* data, uint8_t length){
	//send pattern
	send_wukong_pattern();
	//transmit data
  	uart_write_byte(WIFI_UART, length);
	uint8_t i;
	for(i = 0; i < length; i++) uart_write_byte(WIFI_UART, data[i]);
	//send ip
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 24 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 16 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip >> 8 & 0xFF));
	uart_write_byte(WIFI_UART, (uint32_t)(my_ip & 0xFF));
}

static void radio_wifi_rece_data(){
	if(uart_available(WIFI_UART, 0)){
		if(wifi_wait_and_read(100) == 'W' && wifi_wait_and_read(100) == 'K'){
			uint8_t data_len = wifi_wait_and_read(100);
			uint8_t i;
			unsigned char* data = (unsigned char*)malloc(data_len);
			memset(data, 0, data_len);

			for(i = 0; i < data_len; i++) data[i] = wifi_wait_and_read(100);
			DEBUG_LOG(DBG_WIFI_ARDUINO, "WIFI_DEBUG: data %s\n", data);

			uint32_t ip = 0;
			ip |= wifi_wait_and_read(100);
			ip <<= 8;
			ip |= wifi_wait_and_read(100);
			ip <<= 8;
			ip |= wifi_wait_and_read(100);
			ip <<= 8;
			ip |= wifi_wait_and_read(100);
			DEBUG_LOG(DBG_WIFI_ARDUINO, "WIFI_DEBUG ip: %u.%u.%u.%u\n",
			(uint8_t)(ip >> 24 & 0xFF), (uint8_t)(ip >> 16 & 0xFF), (uint8_t)(ip >> 8 & 0xFF), (uint8_t)(ip & 0xFF));

			routing_handle_wifi_message(ip , data, data_len);
			free(data);
		}else{
			DEBUG_LOG(DBG_WIFI_ARDUINO, "Unkonw data\n");
			clear_buffer();
			//while(uart_available(WIFI_UART, 100))
			//	uart_read_byte(WIFI_UART);
		}
	}
}

void radio_wifi_poll() {
	radio_wifi_rece_data();
}

#endif
