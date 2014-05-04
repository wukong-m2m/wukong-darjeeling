#include "config.h" // To get RADIO_USE_WIFI

#ifdef RADIO_USE_WIFI

#include "radio_wifi.h"
#include <stdlib.h>
#include <string.h>
#include "djtimer.h"
#include "debug.h"
#include "../../common/routing/routing.h"

char wifi_security_key[20]="0123456789";
char wifi_ssid[20]="home321";
char wifi_local_ip[16]="";//if local ip is null, it will get ip from DHCP 
char wifi_subnet[16]="";
char wifi_gateway[16]="";
//char wifi_local_ip[16]="192.168.2.2";
//char wifi_subnet[16]="255.255.255.0";
//char wifi_gateway[16]="192.168.2.222";

char * cmd_tbl[] ={
		"ATE0",
		"AT+WWPA=",
		"AT+WA=",
		"AT+NDHCP=0",
		"AT+NDHCP=1",
		"AT+WD",
		"AT+NSTCP=",
		"AT+NCTCP=",
		"AT+NMAC=?",
		"AT+DNSLOOKUP=",
		"AT+NCLOSE=",
		"AT+NSET=",
		"AT+WM=2",
		"AT+DHCPSRVR=1",
		"AT+WWEP1=",
		"AT+CID=?",
        "ATB=",
        "AT&W",
        "AT&Y"
};

uint8_t hex_to_int(char c)
{
	uint8_t val = 0;

	if (c >= '0' && c <= '9') {
		val = c - '0';
	}
	else if (c >= 'A' && c <= 'F') {
		val = c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'f') {
		val = c - 'a' + 10;
	}

	return val;
}

char int_to_hex(uint8_t c)
{
	char val = '0';

	if (c >= 0 && c <= 9) {
		val = c + '0';
	}
	else if (c >= 10 && c <= 15) {
		val = c + 'A' - 10;
	}

	return val;
}

uint32_t ipstr_to_uint32(char* ipstr) {
  int ipbytes[4];
  sscanf(ipstr, "%d.%d.%d.%d", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
  return ((uint32_t)ipbytes[0]) | ((uint32_t)ipbytes[1] << 8) | ((uint32_t)ipbytes[2] << 16) | ((uint32_t)ipbytes[3] << 24);
}

void ipuint32_to_str(uint32_t ipint, char * ipstr)
{

    for(int i=3;i>=0;i--)
    {
	char buffer [4]={0};
	itoa ((unsigned int) (ipint >> 8*i) & 0xFF, buffer,10);
	strcat(ipstr,buffer);
	if(i>0)
	{    strcat(ipstr,(char *)".");	}
    }

}

uint8_t wifi_mode = NORMAL_MODE;
uint8_t wifi_dev_mode;
uint8_t wifi_connection_state;
uint8_t wifi_serv_cid;
uint16_t wifi_srcport = 1024;
char wifi_connect_dst_ip[16]={0};
char wifi_connect_dst_port[6]={0};
char wifi_dev_mac[18]={0};
char wifi_dns_url_ip[16]={0};
char wifi_lookup_cid_ip[16]={0};
uint8_t wifi_lookup_cid;


SOCKET dataOnSock;
uint8_t socket_num;
SOCK_TABLE sock_table[MAX_SOCK_NUM];

uint8_t radio_wifi_init()
{
	//!!!important!!! if we use wifi for the first time, the baudrate should be 9600
	uart_inituart(WIFI_UART, 9600);
	dj_timer_delay(1);    
	while (uart_available(WIFI_UART, 100))
	{
		uart_read_byte(WIFI_UART);
	}
	uart_write_byte(WIFI_UART, '\n');
	dj_timer_delay(100);
	// set new baudrate
	wifi_send_cmd(CMD_SET_UART_BAUDRATE);
	dj_timer_delay(100);
	uart_inituart(WIFI_UART, WIFI_BAUDRATE);
	dj_timer_delay(100);
	while (uart_available(WIFI_UART, 100))
	{
		uart_read_byte(WIFI_UART);
	}
	uart_write_byte(WIFI_UART, '\n');
	dj_timer_delay(100);
	wifi_send_cmd(CMD_SAVE_PROFILE);
	wifi_send_cmd(CMD_SET_DEFAULT_PROFILE);//set power on default

     
	// uart_inituart(WIFI_UART, WIFI_BAUDRATE);
	// dj_timer_delay(1);    
	// while (uart_available(WIFI_UART, 100))
	// {
	// 	uart_read_byte(WIFI_UART);
	// }
	// uart_write_byte(WIFI_UART, '\n');
	// dj_timer_delay(100);



	wifi_dev_mode = DEV_OP_MODE_COMMAND;
	wifi_connection_state = DEV_CONN_ST_DISCONNECTED;
	dataOnSock = INVALID_SOCKET;



	for (int i = 0; i < MAX_SOCK_NUM; i++) {
	    sock_table[i].cid = INVALID_CID;
	    sock_table[i].port = 0;
	    sock_table[i].protocol = 0;
	    sock_table[i].status = 0;
	}

	// disable echo
	if (!wifi_send_cmd_w_resp(CMD_DISABLE_ECHO)) {
		return WIFI_FAILED;
	}


	// get device ID
	if (!wifi_send_cmd_w_resp(CMD_GET_MAC_ADDR)) {
		return WIFI_FAILED;
	}

        if (!wifi_connect_AP())
        {
        	return WIFI_FAILED;
        }

        //wifi client initial
        socket_num=MAX_SOCK_NUM;

        //wifi server initial
        for (int i = 0; i < MAX_SOCK_NUM; i++) {
                if (sock_table[i].status == SOCK_STATUS_CLOSED) {
                        wifi_configSocket(i, IPPROTO_TCP, WIFI_LISTEN_PORT);
                        wifi_server_listen(i);
                        break;
                }
        }  
 
	return WIFI_SUCCESS;
}

uint8_t wifi_send_cmd_w_resp(uint8_t cmd)
{
	if (wifi_send_cmd(cmd)) {
		return wifi_parse_resp(cmd);
	} else {
		return 0;
	}
}

uint8_t wifi_send_cmd(uint8_t cmd)
{
	int i;
	while (uart_available(WIFI_UART, 100))
	{
		uart_read_byte(WIFI_UART);
	}


	switch(cmd) {
	case CMD_DISABLE_ECHO:
	case CMD_DISABLE_DHCP:
	case CMD_DISCONNECT:
	case CMD_ENABLE_DHCP:
	case CMD_GET_MAC_ADDR:
	case CMD_WIRELESS_MODE:
	case CMD_ENABLE_DHCPSVR:
    case CMD_GET_CID:
	{
		for(i=0;i<strlen(cmd_tbl[cmd]);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_tbl[cmd][i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_SAVE_PROFILE:
	case CMD_SET_DEFAULT_PROFILE:
	{
		char cmd_buf[50]={0};
		strcat(cmd_buf,cmd_tbl[cmd]);  
		strcat(cmd_buf,"1");  
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
	}
    case CMD_SET_UART_BAUDRATE:
    {
        char cmd_buf[50]={0};
        char rate[7]={0};
        ltoa(WIFI_BAUDRATE,rate,10);    
        strcat(cmd_buf,cmd_tbl[cmd]);            
        strcat(cmd_buf,rate);           
        strcat(cmd_buf,",8,n,1");     
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
    }
	case CMD_LISTEN:
    {
        char cmd_buf[50]={0};
        char port[6]={0};
        itoa(sock_table[socket_num].port,port,10);
        strcat(cmd_buf,cmd_tbl[cmd]);            
        strcat(cmd_buf,port);                
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;        
    }
	case CMD_SET_WPA:
	case CMD_SET_WEP:
	{
        char cmd_buf[50]={0};
        strcat(cmd_buf,cmd_tbl[cmd]);  
        strcat(cmd_buf,wifi_security_key);                  
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_SET_SSID:
	{
                char cmd_buf[50]={0};
		if (wifi_mode == NORMAL_MODE)
                {
                        strcat(cmd_buf,cmd_tbl[cmd]);  
                        strcat(cmd_buf,wifi_ssid);   
                }
		else if (wifi_mode == AP_MODE)
                {
                        strcat(cmd_buf,cmd_tbl[cmd]);  
                        strcat(cmd_buf,wifi_ssid);   
                        strcat(cmd_buf,",,11");                         
                }
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_TCP_CONN:
	{
                char cmd_buf[50]={0};
                strcat(cmd_buf,cmd_tbl[cmd]);  
                strcat(cmd_buf,wifi_connect_dst_ip);   
                strcat(cmd_buf,",");  
                strcat(cmd_buf,wifi_connect_dst_port);                   
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_NETWORK_SET:
	{
                char cmd_buf[50]={0};
                strcat(cmd_buf,cmd_tbl[cmd]);  
                strcat(cmd_buf,wifi_local_ip);   
                strcat(cmd_buf,",");  
                strcat(cmd_buf,wifi_subnet);     
                strcat(cmd_buf,",");  
                strcat(cmd_buf,wifi_gateway);                 
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_DNS_LOOKUP:
	{
                char cmd_buf[50]={0};
                strcat(cmd_buf,cmd_tbl[cmd]);  
                strcat(cmd_buf,wifi_dns_url_ip);     
		for(i=0;i<strlen(cmd_buf);i++) 
		{
			uart_write_byte(WIFI_UART, cmd_buf[i]);
		}
		uart_write_byte(WIFI_UART, '\n');
		break;
	}
	case CMD_CLOSE_CONN:
	{
		if (sock_table[socket_num].status != SOCK_STATUS_CLOSED) 
                {
                        char cmd_buf[50]={0};
                        char cid[6]={0};
                        itoa(wifi_lookup_cid,cid,10);
                        strcat(cmd_buf,cmd_tbl[cmd]);  
                        strcat(cmd_buf,cid);                     
			for(i=0;i<strlen(cmd_buf);i++) 
			{
				uart_write_byte(WIFI_UART, cmd_buf[i]);
			}
			uart_write_byte(WIFI_UART, '\n');
		} 
                else 
                {
			return 0;
		}
		break;
	}
	default:
		break;
	}

	return 1;
}


uint8_t wifi_parse_resp(uint8_t cmd)
{
	uint8_t resp_done = 0;
	uint8_t ret = 0;

	while (!resp_done) {
    	char buf[100]={0};
        char strBuf[100]={0};

        wifi_readline(&strBuf[0]);    
        strcpy(buf,strBuf);

        //debug for wifi module response
		// 	DEBUG_LOG(DBG_WKROUTING, "cmd=%d,%s\n",cmd,buf);

		switch(cmd) {
		case CMD_DISABLE_ECHO:
		case CMD_DISABLE_DHCP:
		case CMD_DISCONNECT:
		case CMD_SET_WPA:
		case CMD_SET_WEP:
		case CMD_SET_SSID:
		case CMD_ENABLE_DHCP:
		case CMD_NETWORK_SET:
		case CMD_WIRELESS_MODE:
		case CMD_ENABLE_DHCPSVR:
		case CMD_SAVE_PROFILE:
		case CMD_SET_DEFAULT_PROFILE:
		{
			if (strncmp("OK",buf,2)==0) {
				// got OK 
				ret = 1;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
				ret = 0;
				resp_done = 1;
			}
			break;
		}
        case CMD_GET_CID:
        {
            char compare_str[4]={0};
            itoa(wifi_lookup_cid,compare_str,10);
            uint8_t compare_length=strlen(compare_str);                        
            if (strncmp(compare_str,buf,compare_length)==0) {
                for(int i=15;i>0;i--)
                {
                    wifi_lookup_cid_ip[15-i]=buf[strlen(buf)-i];//IP address located at the end of buffer
                }
				ret = 1;
				resp_done = 1;
			} else if (strncmp(" No validCids",buf,13)==0) {
				// got ERROR 
				ret = 0;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
				ret = 0;
				resp_done = 1;
			}
			break;
        }
		case CMD_LISTEN:
		{
			if (strncmp("CONNECT",buf,7)==0) {
				// got CONNECT 
				wifi_serv_cid = hex_to_int(buf[8]);
				sock_table[socket_num].cid = hex_to_int(buf[8]);
				sock_table[socket_num].status = SOCK_STATUS_LISTEN;
				
			} else if (strncmp("OK",buf,2)==0) {
				// got OK 
				ret = 1;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
				wifi_serv_cid = INVALID_CID;
				sock_table[socket_num].cid = INVALID_CID;
				sock_table[socket_num].status = SOCK_STATUS_CLOSED;
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_TCP_CONN:
		{
			if (strncmp("CONNECT",buf,7)==0) {
				// got CONNECT 
				sock_table[socket_num].cid = hex_to_int(buf[8]);
				sock_table[socket_num].status = SOCK_STATUS_ESTABLISHED;
			} else if (strncmp("OK",buf,2)==0) {
				// got OK 
				ret = 1;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
				sock_table[socket_num].cid = INVALID_CID;
				sock_table[socket_num].status = SOCK_STATUS_CLOSED;
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_GET_MAC_ADDR:
		{
			if (strncmp("00",buf,2)==0) {
				// got MAC addr 
                                strcpy(wifi_dev_mac,buf);
			} else if (strncmp("OK",buf,2)==0) {
				// got OK 
				ret = 1;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
                                strcpy(wifi_dev_mac,"ff:ff:ff:ff:ff:ff");
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_DNS_LOOKUP:
		{
            if (strncmp("IP:",buf,3)==0) {
				// got IP address 
                for(int i=0;i<16;i++)
                {
                        wifi_dns_url_ip[i]=buf[i+3];
                }
			} else if (strncmp("OK",buf,2)==0) {
				// got OK 
				ret = 1;
				resp_done = 1;
			} else if (strncmp("ERROR",buf,5)==0) {
				// got ERROR 
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_CLOSE_CONN:
		{
		    if (strncmp("OK",buf,2)==0) {
		        // got OK 
		        ret = 1;
		        resp_done = 1;

		        // clean up socket 
		        sock_table[socket_num].status = 0;
		        sock_table[socket_num].cid = INVALID_CID;
		        sock_table[socket_num].port = 0;
		        sock_table[socket_num].protocol = 0;
				
			wifi_dev_mode = DEV_OP_MODE_COMMAND;
				
			//clear flag 
			dataOnSock = INVALID_SOCKET;
		    } else if (strncmp("ERROR",buf,5)==0) {
		        //got ERROR
		        ret = 0;
		        resp_done = 1;
		    }
		    break;
		}
		default:
			break;
		}
	}

	return ret;
}


void wifi_readline(char * strBuf)
{
	char inByte;

	bool endDetected = false;

	uint32_t start_time = dj_timer_getTimeMillis();

	while (!endDetected)
	{
		if (uart_available(WIFI_UART, 1))
		{
			// valid data in HW UART buffer, so check if it's \r or \n
			// if so, throw away
			// if strBuf length greater than 0, then this is a true end of line, so break out
			inByte = uart_read_byte(WIFI_UART);
			if ((inByte == '\r') || (inByte == '\n'))
			{
				// throw away
				if ((strlen(strBuf) > 0) && (inByte == '\n'))
				{
					endDetected = true;
				}
			}
			else
			{
                char temp_char[2]={0};
                temp_char[0]=inByte;
                strcat(strBuf,temp_char);
			}
		}
		else if(dj_timer_getTimeMillis()-start_time>3000)
		{
			strcat(strBuf,"ERROR: wifi No response");
			break;
		}
	}

}


uint8_t wifi_connect_AP()
{

	if (!wifi_send_cmd_w_resp(CMD_DISCONNECT)) {
		return 0;
	}

	if (!wifi_send_cmd_w_resp(CMD_DISABLE_DHCP)) {
		return 0;
	}

	if (wifi_mode == NORMAL_MODE) {
		if (!wifi_send_cmd_w_resp(CMD_SET_WEP)) {
			return 0;
		}
		dj_timer_delay(10);

		if (!wifi_send_cmd_w_resp(CMD_SET_SSID)) {
			return 0;
		}
		dj_timer_delay(10);

  		if ( strncmp("",wifi_local_ip,1)==0) {
			if (!wifi_send_cmd_w_resp(CMD_ENABLE_DHCP)) {
				return 0;
			}
			dj_timer_delay(10);
		} else {
			if (!wifi_send_cmd_w_resp(CMD_NETWORK_SET)) {		
				return 0;
			}
			dj_timer_delay(10);			
		}

	} else if (wifi_mode == AP_MODE) {
		if (!wifi_send_cmd_w_resp(CMD_NETWORK_SET)) {
                	return 0;
                }
		if (!wifi_send_cmd_w_resp(CMD_WIRELESS_MODE)) {
                        return 0;
                }
		if (!wifi_send_cmd_w_resp(CMD_SET_SSID)) {
                        return 0;
                }
		if (!wifi_send_cmd_w_resp(CMD_ENABLE_DHCPSVR)) {
			return 0;
		}
		
	} 

	wifi_connection_state = DEV_CONN_ST_CONNECTED;
	return 1;
}

void wifi_configSocket(SOCKET s, uint8_t protocol, uint16_t port)
{
	sock_table[s].protocol = protocol;
	sock_table[s].port = port;
	sock_table[s].status = SOCK_STATUS_INIT;
}

uint8_t wifi_readSocketStatus(SOCKET s)
{
	return sock_table[s].status;
}

uint8_t wifi_connect_socket(char ip[], char port[])
{
        strcpy(wifi_connect_dst_ip,ip);
        strcpy(wifi_connect_dst_port,port);        

	if (!wifi_send_cmd_w_resp(CMD_TCP_CONN)) {
		return 0;
	}

	return 1;
}

uint8_t wifi_client_status(SOCKET s) 
{
  return wifi_readSocketStatus(s);
}

uint8_t radio_wifi_client_connect(char dst_ip[], char dst_port[]) 
{	
	for (int i = 0; i < MAX_SOCK_NUM; i++) {
		if (wifi_readSocketStatus(i) == SOCK_STATUS_CLOSED) {
			socket_num = i;
			break;
		}
	}
	
	if (socket_num == MAX_SOCK_NUM)
		return INVALID_SOCKET;
	
	wifi_srcport++;
	if (wifi_srcport == 0) wifi_srcport = 1024;
	
        wifi_configSocket(socket_num, IPPROTO_TCP, wifi_srcport);	

	if (!wifi_connect_socket(dst_ip, dst_port)) {
		socket_num = MAX_SOCK_NUM;
		return INVALID_SOCKET;
	}
	
	while (wifi_client_status(socket_num) != SOCK_STATUS_ESTABLISHED) {
		dj_timer_delay(1);
		if (wifi_client_status(socket_num) == SOCK_STATUS_CLOSED) {
			socket_num = MAX_SOCK_NUM;
			return INVALID_SOCKET;
		}
	}
	
	return socket_num;
}



uint8_t radio_wifi_send(radio_wifi_address_t dst, uint8_t *buf, uint16_t  len)
{	
	//DEBUG_LOG(DBG_WKCOMM, "Handling command %d from %d, length %d:\n", payload[0], addr, length);


	int i=0;
	if ((len == 0) || (buf[0] == '\r')){
	} else {


		uart_write_byte(WIFI_UART, 0x1b);// data start [ESC] sequence
		uart_write_byte(WIFI_UART, 0x53);
		uart_write_byte(WIFI_UART, (uint8_t)int_to_hex(sock_table[dst].cid)); //connection ID
		if (len == 1){
			if (buf[0] != '\r' && buf[0] != '\n'){ 
				uart_write_byte(WIFI_UART, buf[0]);// data to send				
			} else if (buf[0] == '\n') {
				uart_write_byte(WIFI_UART, '\n');  //new line
				uart_write_byte(WIFI_UART, '\r');				
			} 
		} else {
				for(i=0;i<len;i++) 
				{
					uart_write_byte(WIFI_UART, buf[i]);
				}
		}
		uart_write_byte(WIFI_UART, 0x1b);// data end
		uart_write_byte(WIFI_UART, 0x45);
	}
	//dj_timer_delay(10);

    return WIFI_SUCCESS;//success
}

void radio_wifi_recvData()
{
	uint8_t radio_wifi_receive_buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+10+7+3]={0}; //10 for MPTN overhead, 7 for ZW overhead, 3 for wkcomm overhead
    uint16_t dataLen = 0;
    uint8_t tmp1, tmp2;


    if (wifi_dev_mode == DEV_OP_MODE_DATA_RX) 
    {
	    if(!uart_available(WIFI_UART, 0))
	        return;

	    while(uart_available(WIFI_UART, 0)) 
	    {
	  	    tmp1=uart_read_byte(WIFI_UART);
            if (tmp1 == 0x1b) 
            {
                // escape seq
                // read in escape sequence 
                while(1) 
                {
	    	    	if(uart_available(WIFI_UART, 1)) 
	    	    	{
  						tmp2=uart_read_byte(WIFI_UART);
                        break;
                    }
                }
				
                if (tmp2 == 0x45) {
                    // data end, switch to command mode 
                    wifi_dev_mode = DEV_OP_MODE_COMMAND;
                    // clear flag 
                    dataOnSock = INVALID_SOCKET;
                    break;
                } else {
                    radio_wifi_receive_buffer[dataLen++] = tmp1;
                    radio_wifi_receive_buffer[dataLen++] = tmp2;
                }
            } 
            else 
            {            	
                // data
                radio_wifi_receive_buffer[dataLen] = tmp1;
                dataLen++;
                DEBUG_LOG(DBG_WKROUTING, "recv,%d\n",dataLen);
            }
	    }
    }

    // for(int i=0;i<dataLen;i++)
    // {
    // 	DEBUG_LOG(DBG_WKROUTING, "i:%d,%d",i,radio_wifi_receive_buffer[i]);	
    // }
	
    routing_handle_wifi_message( dataOnSock, radio_wifi_receive_buffer, dataLen);    
}

uint8_t wifi_available() 
{

    if (dataOnSock!=255) 
    {
		return 1;
	} 
	else 
	{
		wifi_process();
		return 0;
	}
	return 0;
}



void wifi_process()
{
    char strBuf[50]={0};
    char inByte;
    uint8_t processDone = 0;

    if(!uart_available(WIFI_UART, 1))
	return;

    while (!processDone) {
        if (wifi_dev_mode == DEV_OP_MODE_COMMAND) {
            while (1) {
		    	if(uart_available(WIFI_UART, 1)) {
					inByte=uart_read_byte(WIFI_UART);
                    if (inByte == 0x1b) {
                        // escape seq, switch mode
                        wifi_dev_mode = DEV_OP_MODE_DATA;
                        break;
                    } else {
                        // command string
                        if ((inByte == '\r') || (inByte == '\n')) {
                            // throw away
                            if ((strlen(strBuf) > 0) && (inByte == '\n'))
                            {
                                // parse command
                                wifi_parse_cmd(strBuf);
                                processDone = 1;
                                break;
                            }
                        }
                        else
                        {
                            char temp_char[2]={0};
                            temp_char[0]=inByte;
                            strcat(strBuf,temp_char);
                        }
                    }
                }
            }
        } else if (wifi_dev_mode == DEV_OP_MODE_DATA) {
            // data mode 
            while(1) {
				if(uart_available(WIFI_UART, 1)) {
		    		inByte=uart_read_byte(WIFI_UART);
                    if (inByte == 0x53) {
                        // data start, switch to data RX mode 
                        wifi_dev_mode = DEV_OP_MODE_DATA_RX;
                        // read in CID 
                        while(1) {
		    				if(uart_available(WIFI_UART, 1)) {
								inByte=uart_read_byte(WIFI_UART);						
                                break;
                            }
                        }

                        // find socket from CID
                        for (SOCKET new_sock = 0; new_sock < MAX_SOCK_NUM; new_sock++) {
                            if (sock_table[new_sock].cid == hex_to_int(inByte)) {
                                dataOnSock = new_sock;
			        			break;
                            }
                        }

                        break;
                    } else if (inByte == 0x45) {
                        // data end, switch to command mode 
                        wifi_dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else if (inByte == 0x4f) {
                        // data mode ok 
                        wifi_dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else if (inByte == 0x46) {
                        // TX failed 
                        wifi_dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else {
                        // unknown 
                        wifi_dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    }
                }
            }
        } else if (wifi_dev_mode ==  DEV_OP_MODE_DATA_RX) {
            processDone = 1;
        }
    }
}

void wifi_parse_cmd(char buf[])
{
        if (strncmp("CONNECT",buf,7)==0) {
		// got CONNECT 
			if (wifi_serv_cid == hex_to_int(buf[8])) {
				// new client connected can do something
			}

			for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
				if ((sock_table[sock].status == SOCK_STATUS_LISTEN) &&
					(sock_table[sock].cid == hex_to_int(buf[8])))
				{
					for (int new_sock = 0; new_sock < MAX_SOCK_NUM; new_sock++) {
						if (sock_table[new_sock].status == SOCK_STATUS_CLOSED) {
							sock_table[new_sock].cid = hex_to_int(buf[10]);
							sock_table[new_sock].port = sock_table[sock].port;
							sock_table[new_sock].protocol = sock_table[sock].protocol;
							sock_table[new_sock].status = SOCK_STATUS_ESTABLISHED;
							break;
						}
					}
				}
			}
        } else if (strncmp("DISCONNECT",buf,10)==0) {
			// got disconnect 
			for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
				if ((sock_table[sock].status == SOCK_STATUS_ESTABLISHED) &&
					(sock_table[sock].cid == hex_to_int(buf[11])))
				{
								DEBUG_LOG(DBG_WKCOMM, "disconnect1\n");
					wifi_server_disconnect(sock_table[sock].cid);
					sock_table[sock].cid = INVALID_CID;
					sock_table[sock].port = 0;
					sock_table[sock].protocol = 0;
					sock_table[sock].status = SOCK_STATUS_CLOSED;
					break;
				}
			}
			// FIXME : need to handle socket disconnection
        } else if (strncmp("Disassociation Event",buf,20)==0) {
			// disconnected from AP 
			wifi_connection_state = DEV_CONN_ST_DISCONNECTED;
		}

}

uint8_t wifi_client_check_connected(SOCKET s) {
  if (s == MAX_SOCK_NUM) return 0;
  return (wifi_client_status(s) == SOCK_STATUS_ESTABLISHED);
}

void wifi_client_stop(uint8_t cid) {
  if (cid == INVALID_CID)
    return;

  wifi_lookup_cid = cid;
  
  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) { //find socket from CID
	if ( sock_table[sock].cid == cid )
	{
		socket_num = sock;
		break;
	}
  }
  
  if (!wifi_send_cmd_w_resp(CMD_CLOSE_CONN)) {
  }
  wifi_lookup_cid = INVALID_CID;

}

uint8_t wifi_server_listen(SOCKET s)
{
  if (wifi_readSocketStatus(s) != SOCK_STATUS_INIT)
    return 0;
  
  socket_num = s;
  if (!wifi_send_cmd_w_resp(CMD_LISTEN)) {
    return 0;
	}
  return 1;
}

uint8_t wifi_get_remoteIP_fromCID(uint8_t cid, char* ip)
{
    for(int i=0;i<16;i++) { //clear buffer
       wifi_lookup_cid_ip[i]='\0';
    }
    wifi_lookup_cid = cid;
    if (!wifi_send_cmd_w_resp(CMD_GET_CID)) {
        return 0;
    }
    
    for(int i=0;i<16;i++) {
       ip[i]=wifi_lookup_cid_ip[i];
    }
    wifi_lookup_cid = INVALID_CID;
    return 1;
}

void radio_wifi_poll(void) {
    
    if (wifi_available())
    {    
        //DEBUG_LOG(DBG_WKCOMM, "data_available\n");
        radio_wifi_recvData();
    }
}

radio_wifi_address_t radio_wifi_get_node_id() {
	return 0;
}
#endif // RADIO_USE_WIFI