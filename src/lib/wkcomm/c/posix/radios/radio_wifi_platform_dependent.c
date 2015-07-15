#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "core.h"
#include "hooks.h"
#include "config.h"
#include "debug.h"
#include "djtimer.h"
#include "routing/routing.h"
#include "posix_utils.h"
#include "../../../../wkpf/c/common/wkpf_config.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"

#ifdef RADIO_USE_WIFI

#define UDP_GW_CMD 2
#define UDP_GW_FWD 1

#define UDP_OVERHEAD 11

dj_time_t wifi_time_init = 0;

radio_wifi_address_t radio_udp_gw_ip = 1;
uint16_t radio_udp_gw_port = 5775;
uint16_t radio_wifi_self_port = 5775;

radio_wifi_address_t radio_wifi_ip_big_endian, radio_wifi_virtual_ip_big_endian;
radio_wifi_address_t radio_wifi_prefix_mask_big_endian;
// !!! host address should not be only 8 bit, this is temporary usage
uint8_t radio_wifi_virtual_host_address;

int radio_wifi_sockfd;

#define BUF_SIZE WKCOMM_MESSAGE_PAYLOAD_SIZE+3+ROUTING_MPTN_OVERHEAD+UDP_OVERHEAD
uint8_t radio_wifi_receive_buffer[BUF_SIZE+1]; // 3 for wkcomm overhead
dj_hook radio_wifi_shutdownHook;

void radio_wifi_shutdown(void *data) {
    close(radio_wifi_sockfd);
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
        send_buffer[3] = radio_wifi_ip_big_endian & 0xFF;
        send_buffer[4] = (radio_wifi_ip_big_endian >> 8) & 0xFF;
        send_buffer[5] = (radio_wifi_ip_big_endian >> 16) & 0xFF;
        send_buffer[6] = (radio_wifi_ip_big_endian >> 24) & 0xFF;
        send_buffer[7] = radio_wifi_self_port & 0xFF;
        send_buffer[8] = (radio_wifi_self_port >> 8) & 0xFF;
        send_buffer[9] = UDP_GW_CMD;
        send_buffer[10] = 0;

        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_port = htons(radio_udp_gw_port);
        cliaddr.sin_addr.s_addr = htonl(radio_udp_gw_ip);
        int retval = sendto(radio_wifi_sockfd, send_buffer, UDP_OVERHEAD, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ipstr, INET_ADDRSTRLEN);
        DEBUG_LOG(DBG_WIFI, "r_wifi discovery sent to %s, length %d, retval %d\n", ipstr, UDP_OVERHEAD, retval);
        if (retval == -1)
            DEBUG_LOG(DBG_WIFI, "r_wifi discovery send fail: %d\n", retval);
        // alternatively broadcast
        if ((radio_udp_gw_ip & 1) == 0){
            cliaddr.sin_addr.s_addr = htonl(radio_wifi_ip_big_endian | ~radio_wifi_prefix_mask_big_endian);
            retval = sendto(radio_wifi_sockfd, send_buffer, UDP_OVERHEAD, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            inet_ntop(AF_INET, &(cliaddr.sin_addr), ipstr, INET_ADDRSTRLEN);
            DEBUG_LOG(DBG_WIFI, "r_wifi discovery sent to %s, length %d, retval %d\n", ipstr, UDP_OVERHEAD, retval);
            if (retval == -1)
                DEBUG_LOG(DBG_WIFI, "r_wifi discovery send fail: %d\n", retval);
        }
        radio_udp_gw_ip = (radio_wifi_ip_big_endian & radio_wifi_prefix_mask_big_endian) | ((radio_udp_gw_ip+1) & (~radio_wifi_prefix_mask_big_endian));
    }
}

void radio_wifi_platform_dependent_init(void) {
    radio_wifi_shutdownHook.function = radio_wifi_shutdown;
    dj_hook_add(&dj_core_shutdownHook, &radio_wifi_shutdownHook);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    // get ip/netmask from interface
    char ipstr[INET_ADDRSTRLEN];
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, posix_interface_name, IFNAMSIZ-1);
    // get ip
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        DEBUG_LOG(DBG_WIFI, "Unable to get interface: %d\n", errno);
        close(fd);
        return;
    }
    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ipstr, INET_ADDRSTRLEN);
    DEBUG_LOG(DBG_WIFI, "ip: %s /", ipstr);
    radio_wifi_ip_big_endian = ntohl((((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr).s_addr);
    // get netmask
    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
        DEBUG_LOG(DBG_WIFI, "Unable to get interface: %d\n", errno);
        close(fd);
        return;
    }
    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ipstr, INET_ADDRSTRLEN);
    DEBUG_LOG(DBG_WIFI, "netmask: %s\n", ipstr);
    radio_wifi_prefix_mask_big_endian = ntohl((((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr).s_addr);


    radio_wifi_virtual_ip_big_endian = wkpf_config_get_myid();
    radio_udp_gw_ip = wkpf_config_get_gwid();
    radio_wifi_virtual_host_address = (radio_wifi_virtual_ip_big_endian & ~radio_wifi_prefix_mask_big_endian);


    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port= htons(radio_wifi_self_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1)
    {
        DEBUG_LOG(DBG_WIFI, "Unable to bind\n");
        close(fd);
        return;
    }
    int broadcastEnable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1)
    {
        DEBUG_LOG(DBG_WIFI, "Unable to SO_BROADCAST\n");
        close(fd);
        return;
    }
    radio_wifi_sockfd = fd;

    if (posix_arg_addnode){
        radio_wifi_platform_dependent_gateway_discovery();
    }
}

radio_wifi_address_t radio_wifi_platform_dependent_get_node_id() {
    return radio_wifi_virtual_ip_big_endian;
}

radio_wifi_address_t radio_wifi_platform_dependent_get_prefix_mask(){
    return radio_wifi_prefix_mask_big_endian;
}

void radio_wifi_platform_dependent_poll(void) {
    // TMP CODE TO PREVENT THE BUSY LOOP FROM GOING TO 100% CPU LOAD
    // THIS IS OBVIOUSLY NOT THE BEST WAY TO DO IT...
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 1000000L; // 1 millisecond
    nanosleep(&tim , &tim2);
    // END TMP CODE

    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    int retval = recvfrom(radio_wifi_sockfd, radio_wifi_receive_buffer, BUF_SIZE, MSG_DONTWAIT, (struct sockaddr *) &si_other, (socklen_t *)&slen);
    if (retval > 0){
        DEBUG_LOG(DBG_WIFI, "r_wifi msg retval %d\n", retval);
    }
    if (retval >= UDP_OVERHEAD && radio_wifi_receive_buffer[0] == 0xAA
            && radio_wifi_receive_buffer[1] == 0x55 ) {
        radio_wifi_address_t src = ntohl(si_other.sin_addr.s_addr);
        // radio_wifi_address_t src?? = radio_wifi_receive_buffer[3]
        //                                     + (((uint32_t)radio_wifi_receive_buffer[4]) << 8)
        //                                     + (((uint32_t)radio_wifi_receive_buffer[5]) << 16)
        //                                     + (((uint32_t)radio_wifi_receive_buffer[6]) << 24);
        // uint16_t port = radio_wifi_receive_buffer[7] + (((uint16_t)radio_wifi_receive_buffer[8]) << 8);
        uint8_t type = radio_wifi_receive_buffer[9], length = radio_wifi_receive_buffer[10];

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(si_other.sin_addr), ipstr, INET_ADDRSTRLEN);
        DEBUG_LOG(DBG_WIFI, "r_wifi msg from %s, length %d, type %d\n", ipstr, length, type);

        if (posix_arg_addnode && type == UDP_GW_CMD && length == 1){
            radio_wifi_virtual_host_address = radio_wifi_receive_buffer[UDP_OVERHEAD];
            posix_arg_addnode = false;
            radio_udp_gw_ip = src;
            wkpf_config_set_gwid(radio_udp_gw_ip);
            radio_wifi_virtual_ip_big_endian = (radio_wifi_prefix_mask_big_endian & radio_udp_gw_ip) | radio_wifi_virtual_host_address;
#ifdef ROUTING_USE_GATEWAY
            routing_discover_gateway();
#endif
        } else if (!posix_arg_addnode && type == UDP_GW_FWD){
            for (int j = 0; j < length; ++j) radio_wifi_receive_buffer[j] = radio_wifi_receive_buffer[UDP_OVERHEAD+j];
            radio_wifi_receive_buffer[length] = 0;
            routing_handle_wifi_message(src, radio_wifi_receive_buffer, length);
        }
    }

    if (posix_arg_addnode){
        radio_wifi_platform_dependent_gateway_discovery();
    }
}

uint8_t radio_wifi_platform_dependent_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    uint8_t send_buffer[BUF_SIZE+1];
    send_buffer[0] = 0xAA;
    send_buffer[1] = 0x55;
    send_buffer[2] = radio_wifi_virtual_host_address;
    send_buffer[3] = radio_wifi_ip_big_endian & 0xFF;
    send_buffer[4] = (radio_wifi_ip_big_endian >> 8) & 0xFF;
    send_buffer[5] = (radio_wifi_ip_big_endian >> 16) & 0xFF;
    send_buffer[6] = (radio_wifi_ip_big_endian >> 24) & 0xFF;
    send_buffer[7] = radio_wifi_self_port & 0xFF;
    send_buffer[8] = (radio_wifi_self_port >> 8) & 0xFF;
    send_buffer[9] = UDP_GW_FWD;
    send_buffer[10] = length;
    memcpy(send_buffer+UDP_OVERHEAD, payload, length);

    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(radio_udp_gw_port);
    cliaddr.sin_addr.s_addr = htonl(dest);
    int retval = sendto(radio_wifi_sockfd, send_buffer, length+UDP_OVERHEAD, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cliaddr.sin_addr), ipstr, INET_ADDRSTRLEN);
    DEBUG_LOG(DBG_WIFI, "r_wifi sent to %s, length %d, retval %d\n", ipstr, length, retval);
    if (retval != -1)
        return 0;
    else
        return -1;
}

uint8_t radio_wifi_platform_dependent_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    if(dest == 0 || posix_arg_addnode) return 0;
    return radio_wifi_platform_dependent_send_raw(dest, payload, length);
}

#endif // RADIO_USE_WIFI
