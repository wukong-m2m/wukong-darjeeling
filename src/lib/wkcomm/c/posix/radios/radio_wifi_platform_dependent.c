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

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"

#ifdef RADIO_USE_WIFI

#define UDP_GW_CMD 2
#define UDP_GW_FWD 1

#define UDP_OVERHEAD 11

radio_wifi_address_t radio_udp_gw_ip;
uint16_t radio_udp_gw_port = 5775;
uint16_t radio_wifi_self_port = 5775;

radio_wifi_address_t radio_wifi_ip_big_endian;
radio_wifi_address_t radio_wifi_prefix_mask_big_endian;
// !!! host address should not be only 8 bit, this is temporary usage
uint8_t radio_wifi_host_address = 0;

int radio_wifi_sockfd;

uint8_t radio_wifi_receive_buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3+ROUTING_MPTN_OVERHEAD+UDP_OVERHEAD]; // 3 for wkcomm overhead
dj_hook radio_wifi_shutdownHook;

void radio_wifi_shutdown(void *data) {
    close(radio_wifi_sockfd);
}

void radio_wifi_platform_dependent_init(void) {
    radio_wifi_shutdownHook.function = radio_wifi_shutdown;
    dj_hook_add(&dj_core_shutdownHook, &radio_wifi_shutdownHook);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, posix_interface_name, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        fprintf(stderr, "Unable to get interface: %d\n", errno);
        close(fd);
        return;
    }
    radio_wifi_ip_big_endian = ntohl((((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr).s_addr);

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
        fprintf(stderr, "Unable to get interface: %d\n", errno);
        close(fd);
        return;
    }
    radio_wifi_prefix_mask_big_endian = ntohl((((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr).s_addr);
    // close(fd);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port= htons(radio_wifi_self_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "Unable to bind");
        close(fd);
        return;
    }
    radio_wifi_sockfd = fd;
}

radio_wifi_address_t radio_wifi_platform_dependent_get_node_id() {
    return radio_wifi_ip_big_endian;
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

    uint8_t start_aa_55;
    int retval = recv(radio_wifi_sockfd, &start_aa_55, 1, MSG_DONTWAIT);
    if (retval > 0 && start_aa_55 == 0xAA) {
        recv(radio_wifi_sockfd, &start_aa_55, 1, 0);
        if (start_aa_55 != 0x55) {
            DEBUG_LOG(DBG_WKCOMM, "not 0x55\n");
            return;
        }
        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, 1, 0); // skip src_host_addr 1
        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, 8, 0); // ip_addr 4, port 2, type 1, length 1
        radio_wifi_address_t src = radio_wifi_receive_buffer[0]
                                            + (((uint32_t)radio_wifi_receive_buffer[1]) << 8)
                                            + (((uint32_t)radio_wifi_receive_buffer[2]) << 16)
                                            + (((uint32_t)radio_wifi_receive_buffer[3]) << 24);
        // uint16_t port = radio_wifi_receive_buffer[4] + (((uint16_t)radio_wifi_receive_buffer[5]) << 8);
        uint8_t type = radio_wifi_receive_buffer[6], length = radio_wifi_receive_buffer[7];

        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, length, 0)
        DEBUG_LOG(DBG_WKCOMM, "r_wifi msg from %d, length %d\n", src, length);
        if (posix_arg_addnode && type == UDP_GW_CMD && length == 1){
            radio_wifi_host_address = radio_wifi_receive_buffer[0];
            posix_arg_addnode = false;
#ifdef ROUTING_USE_GATEWAY
            routing_discover_gateway();
#endif
        } else if (!posix_arg_addnode && type == UDP_GW_FWD){
            routing_handle_wifi_message(src, radio_wifi_receive_buffer, length);
        }
    }
}

uint8_t radio_wifi_platform_dependent_send_raw(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    uint8_t send_buffer[length+UDP_OVERHEAD];
    send_buffer[0] = 0xAA;
    send_buffer[1] = 0x55;
    send_buffer[2] = radio_wifi_host_address;
    send_buffer[3] = radio_wifi_platform_dependent_get_node_id() & 0xFF;
    send_buffer[4] = (radio_wifi_platform_dependent_get_node_id() >> 8) & 0xFF;
    send_buffer[5] = (radio_wifi_platform_dependent_get_node_id() >> 16) & 0xFF;
    send_buffer[6] = (radio_wifi_platform_dependent_get_node_id() >> 24) & 0xFF;
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
    DEBUG_LOG(DBG_WKCOMM, "msg sent to %d, length %d, retval %d\n", dest, length, retval);
    if (retval != -1)
        return 0;
    else
        return -1;
}

uint8_t radio_wifi_platform_dependent_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    return radio_wifi_platform_dependent_send_raw(dest, payload, length);
}

#endif // RADIO_USE_WIFI
