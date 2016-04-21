#include "config.h"


#ifdef RADIO_USE_WIFI
#include "../../../../wkpf/include/common/wkpf_config.h"
#include "../../common/routing/routing.h"
#include "djtimer.h"
#include "uart.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define CMD_SET_WLAN_SSID        0
#define CMD_SET_WLAN_PASSWORD    1
#define CMD_JOIN_NETWORK         2
#define CMD_OPEN                 3
#define CMD_SHOW_CONNECTION      4
#define CMD_CLOSE                5
#define CMD_SAVE                 6
#define CMD_GET_NET_MASK_AND_IP  7
#define CMD_SET_IP_PROTO         8
#define CMD_SET_IP_HOST          9
#define CMD_SET_IP_REMOTE        10
#define CMD_SET_IP_LOCAL         11
#define CMD_SHOW_NET             12
#define CMD_REBOOT               13
#define CMD_FACTORY_RESET        14
#define CMD_SET_WLAN_JOIN_ONE    15
#define CMD_SET_WLAN_AUTH        16
#define CMD_SET_WLAN_RATE        17
#define CMD_SET_UART_BAUDRATE    18
#define CMD_SET_COMM_TIME        19
#define CMD_SET_UART_FLOW        20

#define WIFI_UART         3
#define SHOW_RESULT        true

#define UDP_GW_CMD 2
#define UDP_GW_FWD 1

#define UDP_OVERHEAD 11

dj_time_t wifi_time_init = 0;

radio_wifi_address_t radio_udp_gw_ip = 1;
uint16_t radio_udp_gw_port = 5775;
uint16_t radio_wifi_self_port = 5776;
radio_wifi_address_t pre_dest = 0;

radio_wifi_address_t radio_wifi_if_ip, radio_wifi_virtual_ip;
radio_wifi_address_t radio_wifi_prefix_mask;
// !!! host address should not be only 8 bit, this is temporary usage
uint8_t radio_wifi_virtual_host_address;

#define BUF_SIZE WKCOMM_MESSAGE_PAYLOAD_SIZE+3+ROUTING_MPTN_OVERHEAD+UDP_OVERHEAD // 3 for wkcomm overhead
uint8_t radio_wifi_receive_buffer[BUF_SIZE+1];

char wifi_ssid[] = "WK2G";
char wifi_security_key[] = "wukong2012";

bool avr_arg_addnode = false; // it would be better to declare this variable under <wkroot>/src/core/c/arduino folder
                              // and create a file called avr_utils.c such as posix_utils.c

uint32_t avr_time_btn_push = 0;
uint32_t avr_time_btn_release = 0;
bool avr_btn_is_push = false;
bool avr_btn_is_release = false;
uint32_t avr_led_time=0;

uint32_t inet_pton_wifi(char* ipstr){
    uint8_t i, num_of_bits = 0;
    unsigned char byte_c[3] = {0};
    uint32_t ipnum = 0;

    for(i = 0; i <= strlen(ipstr); i++){
        if(ipstr[i] == '.' || ipstr[i] == 0){
            ipnum <<= 8;
            switch(num_of_bits){
                case 1:
                    ipnum += (byte_c[0] - 48);
                    break;
                case 2:
                    ipnum += (byte_c[0] - 48) * 10 + (byte_c[1] - 48);
                    break;
                case 3:
                    ipnum += (byte_c[0] - 48) * 100 + (byte_c[1] - 48) * 10 + (byte_c[2] - 48);
                    break;
            }
            memset(byte_c, 0, sizeof(byte_c));
            num_of_bits = 0;
        }else{
            byte_c[num_of_bits++] = (unsigned char)ipstr[i];
        }

    }
    return ipnum;
}


void inet_ntop_wifi(char* ipstr, uint32_t ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    sprintf(ipstr, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
}

void clear_buffer(){
    if( uart_available(WIFI_UART, 1000) ){
        uart_read_byte(WIFI_UART);
        while(uart_available(WIFI_UART, 100)) uart_read_byte(WIFI_UART);
    }
}

void show_result(){
    if( uart_available(WIFI_UART, 1000) ){
        DEBUG_LOG(DBG_WIFI, "%c", uart_read_byte(WIFI_UART));
        while(uart_available(WIFI_UART, 100))
            DEBUG_LOG(DBG_WIFI, "%c", uart_read_byte(WIFI_UART));
    }
    DEBUG_LOG(DBG_WIFI, "\n");
}

void enter_cmd_mode(){
    char temp[]="$$$";
    dj_timer_delay(250);
    uint8_t i;
    for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
    //for (char *p = temp; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
    dj_timer_delay(250);

    if(SHOW_RESULT){
        show_result();
    }else{
        clear_buffer();
    }
}

void exit_cmd_mode(){
    char temp[]="exit";
    uint8_t i;
    for(i = 0; i < strlen(temp); i++) uart_write_byte(WIFI_UART, temp[i]);
    //for (char *p = temp; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
    uart_write_byte(WIFI_UART, '\r');

    if(SHOW_RESULT){
        show_result();
    }else{
        clear_buffer();
    }
}

void radio_wifi_send_cmd(uint8_t cmd, char* parameter){
    uint8_t i;
    switch (cmd) {
        case CMD_SET_WLAN_SSID:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set wlan ssid ");
            strcat(cmd_buf, wifi_ssid);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');
            //clear_buffer();
            //show_result();
            break;
        }
        case CMD_CLOSE:
        {
            char cmd_buf[]="close";
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
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
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');
            //clear_buffer();
            //show_result();
            break;
        }
        case CMD_SET_IP_PROTO:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set ip proto ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_IP_HOST:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set ip host ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_IP_REMOTE:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set ip remote ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_IP_LOCAL:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set ip local ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SHOW_NET:
        {
            char cmd_buf[]="show net";
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_REBOOT:
        {
            char cmd_buf[]="reboot";
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_FACTORY_RESET:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "factory RESET");
            strcat(cmd_buf, wifi_ssid);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_WLAN_JOIN_ONE:
        {
            char cmd_buf[]="set wlan join 1";
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_WLAN_AUTH:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set wlan auth ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_WLAN_RATE:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set wlan rate ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_UART_BAUDRATE:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set uart instant ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_COMM_TIME:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set comm time ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }
        case CMD_SET_UART_FLOW:
        {
            char cmd_buf[35]={0};
            strcat(cmd_buf, "set uart flow ");
            strcat(cmd_buf, parameter);
            for(i = 0; i < strlen(cmd_buf); i++) uart_write_byte(WIFI_UART, cmd_buf[i]);
            //for (char *p = cmd_buf; *p != '\0'; p++) uart_write_byte(WIFI_UART, *p);
            uart_write_byte(WIFI_UART, '\r');

            if(SHOW_RESULT){
                show_result();
            }else{
                clear_buffer();
            }
            break;
        }

    }
}

void radio_wifi_set_uart_instant(){
/*    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_SET_UART_BAUDRATE, "9600");
    uart_inituart(WIFI_UART, 9600);
    dj_timer_delay(1);*/
}

bool radio_wifi_get_net_mask_and_ip(){
    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_GET_NET_MASK_AND_IP, NULL);

    char tmparray[20][40]={{0}};
    uint8_t i=0, j=0;
    if( uart_available(WIFI_UART, 1000) ){
        do{
            char tmpchar = uart_read_byte(WIFI_UART);
            if(tmpchar != '\n'){
                  tmparray[i][j] = tmpchar;
                  j++;
            }else{
                i++;
                j=0;
            }
        }while(uart_available(WIFI_UART, 100));
    }

    char ipstr[20]={0}, maskstr[20]={0};
    for(i=0;i<20;i++){
        if(tmparray[i][0] == 'I' && tmparray[i][1] == 'P'){
            char* head = strchr(tmparray[i],'=');
            char* tail = strchr(tmparray[i],':');
            memset (ipstr, 0, 20);
            memcpy (ipstr, head+1, tail-(head+1) );
            ipstr[tail-(head+1)] = '\0';
            radio_wifi_if_ip = inet_pton_wifi(ipstr);
            DEBUG_LOG(DBG_WIFI, "radio wifi get ip=%s\n", ipstr);
            continue;
        }
        if(tmparray[i][0] == 'N' && tmparray[i][1] == 'M'){
            char* head = strchr(tmparray[i],'=');
            char* tail = strchr(tmparray[i],'\0');
            memset (maskstr, 0, 20);
            memcpy (maskstr, head+1, tail-1-(head+1) );
            maskstr[tail-1-(head+1)] = '\0';
            radio_wifi_prefix_mask = inet_pton_wifi(maskstr);
            DEBUG_LOG(DBG_WIFI, "radio wifi get netmask=%s\n", maskstr);
        }
    }

/*    for(i=0;i<20;i++){
        for(j=0;j<40;j++){
            DEBUG_LOG(DBG_WIFI, "%c", tmparray[i][j]);
        }
        DEBUG_LOG(DBG_WIFI, "\n");
    }
*/
    exit_cmd_mode();

    if( strcmp(ipstr, "0.0.0.0") == 0 || strcmp(ipstr, "\0") == 0 ){
        DEBUG_LOG(DBG_WIFI, "please wait for radio wifi association\n");
        return false;
    }else{
        DEBUG_LOG(DBG_WIFI, "radio wifi has been associated with a network\n");
        return true;
    }
}

/*void radio_wifi_setup_udp(char* dest){
    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_SET_IP_HOST, tmpstr);
    radio_wifi_send_cmd(CMD_SAVE, NULL);
    radio_wifi_send_cmd(CMD_REBOOT, NULL);
    //radio_wifi_send_cmd(CMD_JOIN_NETWORK, NULL);
    exit_cmd_mode();
}*/

int8_t radio_wifi_uart_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length){
    if (dest != pre_dest){
        char tmpstr[16] = {0};
        inet_ntop_wifi(tmpstr, dest);
        enter_cmd_mode();
        radio_wifi_send_cmd(CMD_SET_IP_HOST, tmpstr);
        memset(tmpstr, 0, 16);
        snprintf(tmpstr, 16, "%d", radio_udp_gw_port);
        radio_wifi_send_cmd(CMD_SET_IP_REMOTE, tmpstr);
        // memset(tmpstr, 0, 16);
        // snprintf(tmpstr, 16, "%d", radio_wifi_self_port);
        // radio_wifi_send_cmd(CMD_SET_IP_LOCAL, tmpstr);
        radio_wifi_send_cmd(CMD_SAVE, NULL);
        radio_wifi_send_cmd(CMD_REBOOT, NULL);
        exit_cmd_mode();

        radio_wifi_set_uart_instant();

        DEBUG_LOG(DBG_WIFI,"Set up the UDP connection %d\n", dest);
        pre_dest = dest;
        dj_timer_delay(50);
    }

    uint8_t i;
    for(i = 0; i < length; i++) uart_write_byte(WIFI_UART, payload[i]);
    return 0;
    // return -1;
}

uint8_t radio_wifi_uart_recv(){
    uint8_t i = 0;
    if( uart_available(WIFI_UART, 0) ){
        radio_wifi_receive_buffer[i++] = uart_read_byte(WIFI_UART);
        while(uart_available(WIFI_UART, 100)){
            radio_wifi_receive_buffer[i++] = uart_read_byte(WIFI_UART);
            if (i == BUF_SIZE) break;
        }
        // clear buffer
        while(uart_available(WIFI_UART, 100)) uart_read_byte(WIFI_UART);
    }
    radio_wifi_receive_buffer[i] = 0;

    return i;
}


void radio_wifi_platform_dependent_gateway_discovery(void){
    if (wifi_time_init == 0) {
        wifi_time_init = dj_timer_getTimeMillis();
    }
    if ((dj_timer_getTimeMillis()-wifi_time_init > 500)) {
        wifi_time_init = dj_timer_getTimeMillis();
        uint8_t send_buffer[UDP_OVERHEAD+1];
        send_buffer[0] = 0xAA;
        send_buffer[1] = 0x55;
        send_buffer[2] = radio_wifi_virtual_host_address;
        send_buffer[3] = radio_wifi_if_ip & 0xFF;
        send_buffer[4] = (radio_wifi_if_ip >> 8) & 0xFF;
        send_buffer[5] = (radio_wifi_if_ip >> 16) & 0xFF;
        send_buffer[6] = (radio_wifi_if_ip >> 24) & 0xFF;
        send_buffer[7] = radio_wifi_self_port & 0xFF;
        send_buffer[8] = (radio_wifi_self_port >> 8) & 0xFF;
        send_buffer[9] = UDP_GW_CMD;
        send_buffer[10] = 0;

        char ipstr2[16] = "192.168.4.17";
        radio_udp_gw_ip = inet_pton_wifi(ipstr2);
        radio_wifi_uart_send(radio_udp_gw_ip, send_buffer, UDP_OVERHEAD);
        char ipstr[16] = {0};
        inet_ntop_wifi(ipstr, radio_udp_gw_ip);
        DEBUG_LOG(DBG_WIFI, "r_wifi discovery sent to %s, length %d\n", ipstr, UDP_OVERHEAD);

        // alternatively broadcast
        // if ((radio_udp_gw_ip & 1) == 0){
        //     radio_wifi_uart_send((radio_wifi_if_ip | ~radio_wifi_prefix_mask), send_buffer, UDP_OVERHEAD);
        //     memset(ipstr, 0, 16);
        //     inet_ntop_wifi(ipstr, (radio_wifi_if_ip | ~radio_wifi_prefix_mask));
        //     DEBUG_LOG(DBG_WIFI, "r_wifi discovery sent to %s, length %d\n", ipstr, UDP_OVERHEAD);
        // }
        // radio_udp_gw_ip = (radio_wifi_if_ip & radio_wifi_prefix_mask) | ((radio_udp_gw_ip+1) & (~radio_wifi_prefix_mask));
    }
}

void radio_wifi_associate_with_a_network(){
    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_SET_COMM_TIME, "7");
    radio_wifi_send_cmd(CMD_SET_WLAN_SSID, NULL);
    radio_wifi_send_cmd(CMD_SET_WLAN_PASSWORD, NULL);
    radio_wifi_send_cmd(CMD_SET_IP_PROTO, "1");
    // radio_wifi_send_cmd(CMD_SET_WLAN_AUTH, "4");
    radio_wifi_send_cmd(CMD_SET_WLAN_RATE, "15");
    radio_wifi_send_cmd(CMD_SET_WLAN_JOIN_ONE, NULL);
    char tmpstr[10] = {0};
    snprintf(tmpstr, 5, "%d", radio_wifi_self_port);
    radio_wifi_send_cmd(CMD_SET_IP_LOCAL, tmpstr);
    /*radio_wifi_send_cmd(CMD_SET_UART_FLOW, "1");*/
    radio_wifi_send_cmd(CMD_SAVE, NULL);
    radio_wifi_send_cmd(CMD_REBOOT, NULL);
    //radio_wifi_send_cmd(CMD_JOIN_NETWORK, NULL);
    exit_cmd_mode();
}

void radio_wifi_platform_dependent_init(void){

    //turn on wifi module on wudevice 3.0
    DDRL |= (1<<5);
    //PORTL &= ~(1<<5);
    PORTL |= (1<<5);

    //initialize button for addnode
    // DDRE &= ~_BV(4);    //set PE4 as input
    // PORTE |= _BV(4);    //pull high
    DDRG &= ~_BV(5);
    PORTG |= _BV(5);
    //initialize PK0 for LED blinking
    DDRK |= _BV(0);

    uart_inituart(WIFI_UART, 9600);

    enter_cmd_mode();
    radio_wifi_send_cmd(CMD_FACTORY_RESET, NULL);
    radio_wifi_send_cmd(CMD_REBOOT, NULL);
    exit_cmd_mode();
    dj_timer_delay(10000);


    radio_wifi_associate_with_a_network();
    // DEBUG_LOG(DBG_WIFI, "please wait for radio wifi association\n");
    dj_timer_delay(10000);

    radio_wifi_set_uart_instant();

    while(!radio_wifi_get_net_mask_and_ip()){
        enter_cmd_mode();
        radio_wifi_send_cmd(CMD_SET_WLAN_PASSWORD, NULL);
        radio_wifi_send_cmd(CMD_JOIN_NETWORK, NULL);
        exit_cmd_mode();
        dj_timer_delay(10000);
        //radio_wifi_associate_with_a_network();
        enter_cmd_mode();
        radio_wifi_send_cmd(CMD_SHOW_CONNECTION, NULL);
        exit_cmd_mode();
    }

    radio_wifi_virtual_ip = wkpf_config_get_myid(); // src/lib/wkpf/c/arduino/wkpf_config.c
    radio_udp_gw_ip = wkpf_config_get_gwid();       // src/lib/wkpf/c/arduino/wkpf_config.c
    radio_wifi_virtual_host_address = (radio_wifi_virtual_ip & ~radio_wifi_prefix_mask);

    if (avr_arg_addnode){
        radio_wifi_platform_dependent_gateway_discovery();
    }

    DEBUG_LOG(DBG_WIFI, "Init successfully!\n");
}

radio_wifi_address_t radio_wifi_platform_dependent_get_node_id(){
    return radio_wifi_virtual_ip;
}
radio_wifi_address_t radio_wifi_platform_dependent_get_prefix_mask(){
    return radio_wifi_prefix_mask;
}

void avr_addnode_button_poll(){
    static uint32_t avr_pg5_press=0;
    static uint32_t avr_pe4_press=0;
    if (avr_pg5_press==0 && avr_pe4_press==0) {
        if ((PING&_BV(5))==0) {
            avr_pg5_press = dj_timer_getTimeMillis();
            avr_time_btn_push = dj_timer_getTimeMillis();
            avr_btn_is_push = true;
            PORTK &= ~_BV(0);
            DEBUG_LOG(DBG_WKCOMM,"PG5 is pressed");
        }
        // if ((PINE&_BV(4))==0) {
        //     avr_pe4_press = dj_timer_getTimeMillis();
        //     avr_time_btn_push = dj_timer_getTimeMillis();
        //     avr_btn_is_push = true;
        //     PORTK &= ~_BV(0);
        //     DEBUG_LOG(true,"PE4 0\n");
        // }
    // } else if (avr_pe4_press != 0) {
    //     if ((PINE&_BV(4))) {
    //         if (dj_timer_getTimeMillis() > avr_pe4_press + 100) {
    //             avr_time_btn_release = dj_timer_getTimeMillis();
    //             avr_btn_is_push = false;
    //             avr_btn_is_release = true;
    //             avr_led_time = avr_time_btn_release;
    //             PORTK &= ~_BV(0);
    //             DEBUG_LOG(true,"PE4 1");
    //         }
    //         avr_pe4_press = 0;
    //         avr_btn_is_push = false;
    //     }
    } else {
        if ((PING&_BV(5))) {
            if (dj_timer_getTimeMillis() > avr_pg5_press + 100) {
                avr_time_btn_release=dj_timer_getTimeMillis();
                avr_btn_is_push=false;
                avr_btn_is_release=true;
                avr_led_time = avr_time_btn_release;
                PORTK &= ~_BV(0);
                avr_pg5_press = 0;
            }
        }
    }
    if ( avr_led_time > 0 && dj_timer_getTimeMillis() > avr_led_time) {
        PORTK |= _BV(0);
        avr_led_time = 0;
    }

    if( avr_btn_is_release==true )
    {
        if( (avr_time_btn_release-avr_time_btn_push)<5000 )//push btn <5s go to learning mode
        {
            avr_arg_addnode = true;
/*            if(zwave_learn_on)//push btn in learning mode -> stop learning
                zwave_time_learn_start=0;//timeout stop learning
            else//push btn in normal mode -> start learning
                zwave_mode=1;
        }
        else//push btn >5s go to reset mode
        {
            zwave_mode=2;*/
        }
        avr_btn_is_release=false;
    }
}

void radio_wifi_platform_dependent_poll(){
    avr_addnode_button_poll();
    if (avr_arg_addnode){
        radio_wifi_platform_dependent_gateway_discovery();
    }

    uint8_t retval = radio_wifi_uart_recv();
    if (!retval) return;
    DEBUG_LOG(DBG_WIFI, "r_wifi msg recv: ");
    for (int i = 0; i < retval; ++i){
        DEBUG_LOG(DBG_WIFI, "%d ", radio_wifi_receive_buffer[i]);
    }
    DEBUG_LOG(DBG_WIFI, "\n");

    if (retval >= UDP_OVERHEAD && radio_wifi_receive_buffer[0] == 0xAA && radio_wifi_receive_buffer[1] == 0x55 ){
        uint8_t type = radio_wifi_receive_buffer[9], length = radio_wifi_receive_buffer[10];
        DEBUG_LOG(DBG_WIFI, "r_wifi msg recv, length %d, type %d\n", length, type);

        if (avr_arg_addnode && type == UDP_GW_CMD && length == 1){
            radio_wifi_virtual_host_address = radio_wifi_receive_buffer[UDP_OVERHEAD];
            avr_arg_addnode = false;
            radio_udp_gw_ip = 0;
            for (int i = 6; i > 2; --i){
                radio_udp_gw_ip <<= 8;
                radio_udp_gw_ip |= (radio_wifi_receive_buffer[i] & 0xFF);
            }
            wkpf_config_set_gwid(radio_udp_gw_ip);
            radio_wifi_virtual_ip = (radio_wifi_prefix_mask & radio_udp_gw_ip) | radio_wifi_virtual_host_address;
#ifdef ROUTING_USE_GATEWAY
            routing_discover_gateway();
#endif
        } else if (!avr_arg_addnode && type == UDP_GW_FWD){
            for (int j = 0; j < length; ++j) radio_wifi_receive_buffer[j] = radio_wifi_receive_buffer[UDP_OVERHEAD+j];
            radio_wifi_receive_buffer[length] = 0;
            routing_handle_wifi_message(radio_udp_gw_ip, radio_wifi_receive_buffer, length);
        }
    }
}

uint8_t radio_wifi_platform_dependent_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length){
    uint8_t send_buffer[BUF_SIZE+1];
    send_buffer[0] = 0xAA;
    send_buffer[1] = 0x55;
    send_buffer[2] = radio_wifi_virtual_host_address;
    send_buffer[3] = radio_wifi_if_ip & 0xFF;
    send_buffer[4] = (radio_wifi_if_ip >> 8) & 0xFF;
    send_buffer[5] = (radio_wifi_if_ip >> 16) & 0xFF;
    send_buffer[6] = (radio_wifi_if_ip >> 24) & 0xFF;
    send_buffer[7] = radio_wifi_self_port & 0xFF;
    send_buffer[8] = (radio_wifi_self_port >> 8) & 0xFF;
    send_buffer[9] = UDP_GW_FWD;
    send_buffer[10] = length;
    memcpy(send_buffer+UDP_OVERHEAD, payload, length);

    int retval;
    retval = radio_wifi_uart_send(dest, send_buffer, length+UDP_OVERHEAD);

    char ipstr[16] = {0};
    inet_ntop_wifi(ipstr, dest);

    if (retval != -1){
        DEBUG_LOG(DBG_WIFI, "r_wifi sent to %s, length %d, retval %d\n", ipstr, length, retval);
        return 0;
    }else
        return -1;
}


uint8_t radio_wifi_platform_dependent_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length){
    if(dest == 0 || avr_arg_addnode) return 0;
    return radio_wifi_platform_dependent_send_raw(dest, payload, length);
}

#endif // RADIO_USE_WIFI7
