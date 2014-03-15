#include "wifi.h"
#include "djtimer.h"
#include "uart.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

char wifi_ssid[]="intel_wukong";
char wifi_security_key[]="wukong2012";
char* cmd_list[] ={
  "set wlan ssid ",
  "set wlan phrase ",
  "join ",
  "open ",
  "show connection",
  "close",
  "save",
  "get ip a"
};
//used in radio_wifi_poll() to check it is data or trash
uint8_t trueData=0;

static void clear_buffer(){
  while( uart_available(WIFI_UART, 0) == false );
  dj_timer_delay(200);
  while(uart_available(WIFI_UART, 0)) uart_read_byte(WIFI_UART);
}

static void show_result(){
  while( uart_available(WIFI_UART, 0) == false );
  dj_timer_delay(200);
  char temp[300]={0};
  uint8_t i = 0;
  while(uart_available(WIFI_UART, 0)) temp[i++] = uart_read_byte(WIFI_UART);
  DEBUG_LOG(DBG_WIFI, "%s", temp);
}

static void enter_cmd_mode(){
  uint8_t i;
  char temp[]="$$$";
  dj_timer_delay(250);
  for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
  dj_timer_delay(250);
  //show_result();
  clear_buffer();
}

static void exit_cmd_mode(){
  uint8_t i;
  char temp[]="exit";
  for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
  uart_write_byte(WIFI_UART, '\r');
  //show_result();
  clear_buffer();
}

static void radio_wifi_send_cmd(uint8_t cmd, char* parameter){
  uint8_t i;
  switch (cmd) {
    case CMD_SET_WLAN_SSID:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      strcat(cmd_buf, wifi_ssid);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      show_result();
      break;
    }
    case CMD_SET_WLAN_PASSWORD:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      strcat(cmd_buf, wifi_security_key);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      show_result();
      break;
    }
    case CMD_JOIN_NETWORK:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      strcat(cmd_buf, wifi_ssid);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      show_result();
      break;
    }
    case CMD_OPEN:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      strcat(cmd_buf, parameter);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      dj_timer_delay(100);
      clear_buffer();
      //show_result();
      break;
    }
    case CMD_SHOW_CONNECTION:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      //show_result();
      break;
    }
    case CMD_CLOSE:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      clear_buffer();
      //show_result();
      break;
    }
    case CMD_SAVE:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      show_result();
      break;
    }
    case CMD_GET_IP:
    {
      char cmd_buf[50]={0};
      strcat(cmd_buf, cmd_list[cmd]);  
      for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
      uart_write_byte(WIFI_UART, '\r');
      //clear_buffer();
      //show_result();
      break;
    }
  }
}

void radio_wifi_initial(){
  uart_inituart(WIFI_UART, 9600);  
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_SET_WLAN_SSID, NULL);
  radio_wifi_send_cmd(CMD_SET_WLAN_PASSWORD, NULL);
  radio_wifi_send_cmd(CMD_SAVE, NULL);
  //radio_wifi_send_cmd(CMD_JOIN_NETWORK, NULL);
  exit_cmd_mode();
}

void radio_wifi_get_ip(char* ip){
  char temp;
  uint8_t i = 0;
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_GET_IP, NULL);
  dj_timer_delay(150);
  while(uart_available(WIFI_UART, 0)){ 
    temp = uart_read_byte(WIFI_UART);
    if(temp =='\n') break;
  }
  while(uart_available(WIFI_UART, 0)){ 
    ip[i] = uart_read_byte(WIFI_UART);
    if(ip[i] =='\n') {
      ip[i]='\0';
      clear_buffer();
      break;
    }
    i++;
  }
  exit_cmd_mode();
}

static void radio_wifi_connect_to(char* ip, char* port){
  char addr[25]={0};
  strcat(addr, ip);
  strcat(addr, " ");
  strcat(addr, port);
  radio_wifi_send_cmd(CMD_OPEN, addr);
}

uint8_t radio_wifi_send(char* ip, char* port, char* data){
  char state[5]={0}, dataLen[3]={0}, temp=0;

  enter_cmd_mode();

  /*****check status before establish connection*****/
  radio_wifi_send_cmd(CMD_SHOW_CONNECTION, NULL); 
  //wait return value of CMD_SHOW_CONNECTION
  while( uart_available(WIFI_UART, 0) == false );
  dj_timer_delay(150);
  while( uart_available(WIFI_UART, 0) ){ 
    temp = uart_read_byte(WIFI_UART);
    if(temp =='\n') break;
  }
  if(uart_available(WIFI_UART, 0)){
    uint8_t i;
    for(i = 0; i < 4; i++) state[i] = uart_read_byte(WIFI_UART);
    clear_buffer();
  }
  DEBUG_LOG(DBG_WIFI, "before open: %s\n", state); 
  //status '0' means the wifi module is idle
  if(state[3]!='0'){  
    exit_cmd_mode();
    return 0;
  }
  /**********check end**********/

  //establish connection
  radio_wifi_connect_to(ip, port);
  
  /*****check status before establish connection*****/
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_SHOW_CONNECTION, NULL);
 
  //wait return value of CMD_SHOW_CONNECTION
  while( uart_available(WIFI_UART, 0) == false );
  dj_timer_delay(150);
  while( uart_available(WIFI_UART, 0) ){ 
    temp = uart_read_byte(WIFI_UART);
    if(temp =='\n') break;
  }
  if(uart_available(WIFI_UART, 0)){
    uint8_t i;
    for(i = 0; i < 4; i++) state[i] = uart_read_byte(WIFI_UART);
    clear_buffer();
  }
 
  DEBUG_LOG(DBG_WIFI, "after open: %s\n", state); 
  exit_cmd_mode();

  //status '1' means the wifi module is connected, '4' meas the wifi module is connecting with someone
  //I am confused because it usually returns status '1' instead of status '4'
  if(state[3]!='1' && state[3]!='4'){  
    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_CLOSE, NULL);
    exit_cmd_mode();
    return 0;
  }
  /**********check end**********/

  //transmit data
  sprintf(dataLen,"%u",strlen(data));
  char data_len_num_bits = (char)((uint8_t) '0' + strlen(dataLen));
  uart_write_byte(WIFI_UART, data_len_num_bits);
  uart_write_byte(WIFI_UART, dataLen[0]);
  uart_write_byte(WIFI_UART, dataLen[1]);
  uint8_t i;
  for(i = 0; i < strlen(data); i++)
    uart_write_byte(WIFI_UART, data[i]);

  //close connection
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_CLOSE, NULL);
  exit_cmd_mode();
  
  return 1;
};

static void radio_wifi_rece_data() {
  char temp;
  uint8_t counter, datalen, len;
  char data[99]={0};
  if ( uart_available(WIFI_UART, 0) ){
    dj_timer_delay(150);
    temp = uart_read_byte(WIFI_UART);

    switch (temp) {
      case '*':
        temp = uart_read_byte(WIFI_UART);
        if( temp == 'O') {
		len = 11; 
		trueData = 1;
	}
        else if(temp == 'C') len = 4;
	else len = 0;
        for(counter = 0; counter < len; counter++) temp = uart_read_byte(WIFI_UART);
        break;
      case '1':
	if (trueData == 1){
        	datalen = uart_read_byte(WIFI_UART) - '0';
        	for(counter = 0; counter < datalen ; counter++) data[counter] = uart_read_byte(WIFI_UART);
		trueData = 0;
        	DEBUG_LOG(DBG_WIFI, "data: %s\n", data);
        	//routing_handle_wifi_msg();
	}else{
  		while(uart_available(WIFI_UART, 0)) uart_read_byte(WIFI_UART);
	}
        break;
      case '2':
	if (trueData == 1){
        	datalen = (uart_read_byte(WIFI_UART) - '0') * 10 + (uart_read_byte(WIFI_UART) - '0');
        	for(counter = 0; counter < datalen ; counter++) data[counter] = uart_read_byte(WIFI_UART);
		trueData = 0;
        	DEBUG_LOG(DBG_WIFI, "data: %s\n", data);
        	//routing_handle_wifi_msg();
	}else{
  		while(uart_available(WIFI_UART, 0)) uart_read_byte(WIFI_UART);
	}
        break;
      default:
  	while(uart_available(WIFI_UART, 0)) uart_read_byte(WIFI_UART);
        break;
    }
  }
}

void radio_wifi_poll() {
  radio_wifi_rece_data();
}

void radio_wifi_close(){
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_CLOSE, NULL);
  exit_cmd_mode();
}

void radio_wifi_status(){
  enter_cmd_mode();
  radio_wifi_send_cmd(CMD_SHOW_CONNECTION, NULL);
  show_result();
  exit_cmd_mode();
}
