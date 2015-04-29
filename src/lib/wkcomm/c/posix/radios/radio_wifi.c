#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "core.h"
#include "hooks.h"
#include "types.h"
#include "config.h"
#include "debug.h"
#include "djtimer.h"
#include "routing/routing.h"
#include "radio_wifi.h"
#include "posix_utils.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"

#ifdef RADIO_USE_WIFI

// Protocol format: see WuKongNetworkServer.java
#define MODE_MESSAGE 1
#define MODE_DISCOVERY 2

int radio_udp_server_port = 5775;
char* radio_udp_server_ = "127.0.0.1";
bool radio_wifi_connected = false;
int radio_wifi_sockfd;
uint8_t radio_wifi_receive_buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3+ROUTING_MPTN_OVERHEAD]; // 3 for wkcomm overhead
dj_hook radio_wifi_shutdownHook;

dj_time_t radio_wifi_last_heartbeat = 0;

void open_connection() {
    fprintf(stderr, "Opening connection to udp server\n");
    struct sockaddr_in servaddr;

    radio_wifi_sockfd=socket(AF_INET,SOCK_STREAM,0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(posix_network_server_address);
    servaddr.sin_port=htons(posix_network_server_port);

    int retval = connect(radio_wifi_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    if (retval != 0) {
        fprintf(stderr, "Unable to establish radio connection: %d\n", errno);
        close(radio_wifi_sockfd);
        return;
    }
    int length=recv(radio_wifi_sockfd, radio_wifi_receive_buffer, 1, 0);
    if (length != 1 || radio_wifi_receive_buffer[0] != 42) {
        fprintf(stderr, "Unable to establish radio connection.\n");
        close(radio_wifi_sockfd);
        return;
    }
    uint8_t send_buffer[5];
    // Connect in messaging mode
    send_buffer[0] = MODE_MESSAGE;
    // Tell the server our network id
    send_buffer[1] = radio_wifi_get_node_id() & 0xFF;
    send_buffer[2] = (radio_wifi_get_node_id() >> 8) & 0xFF;
    send_buffer[3] = (radio_wifi_get_node_id() >> 16) & 0xFF;
    send_buffer[4] = (radio_wifi_get_node_id() >> 24) & 0xFF;
    retval = write(radio_wifi_sockfd, send_buffer, 5);
    if (retval == -1) {
        fprintf(stderr, "Unable to send local network id to server: %d\n", errno);
        close(radio_wifi_sockfd);
        return;
    }
    fprintf(stderr, "CONNECTED\n");

    radio_wifi_connected = true;
}

void radio_wifi_shutdown(void *data) {
    close(radio_wifi_sockfd);
}

void radio_wifi_init(void) {
    radio_wifi_shutdownHook.function = radio_wifi_shutdown;
    dj_hook_add(&dj_core_shutdownHook, &radio_wifi_shutdownHook);
}

radio_wifi_address_t radio_wifi_get_node_id() {
    return posix_local_network_id;
}

void radio_wifi_poll(void) {
    // TMP CODE TO PREVENT THE BUSY LOOP FROM GOING TO 100% CPU LOAD
    // THIS IS OBVIOUSLY NOT THE BEST WAY TO DO IT...
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 1000000L; // 1 millisecond
    nanosleep(&tim , &tim2);
    // END TMP CODE

    // fprintf(stderr, "poll %lld %lld\n", dj_timer_getTimeMillis(), radio_wifi_last_heartbeat);
    if ((dj_timer_getTimeMillis() - radio_wifi_last_heartbeat) > 1000) {
        radio_wifi_last_heartbeat = dj_timer_getTimeMillis();
        // Every second:
        if (radio_wifi_connected == false) {
            // If the connection is closed, try to open it.
            open_connection();
        } else {
            // If we think connection is open, send a heartbeat packet to check.
            uint8_t send_buffer[1];
            send_buffer[0] = 0;
            write(radio_wifi_sockfd, send_buffer, 1);
        }
    }

    uint8_t length;
    int retval = recv(radio_wifi_sockfd, &length, 1, MSG_DONTWAIT);
    if (retval > 0 && length > 0) {
        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, 4, 0);
        radio_wifi_address_t src = radio_wifi_receive_buffer[0]
                                            + (((uint32_t)radio_wifi_receive_buffer[1]) << 8)
                                            + (((uint32_t)radio_wifi_receive_buffer[2]) << 16)
                                            + (((uint32_t)radio_wifi_receive_buffer[3]) << 24);
        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, 4, 0); // skip dest
        recv(radio_wifi_sockfd, radio_wifi_receive_buffer, length-9, 0);
        DEBUG_LOG(DBG_WKCOMM, "Message received from %d, length %d\n", src, length-9);
        routing_handle_local_message(src, radio_wifi_receive_buffer, length-9);
    } else if (retval == 0) {
        fprintf(stderr, "Connection to network server lost.\n");
        close(radio_wifi_sockfd);
        radio_wifi_connected = false;
    }
}

uint8_t radio_wifi_send(radio_wifi_address_t dest, uint8_t *payload, uint8_t length) {
    if (!radio_wifi_connected)
        return -1;

    uint8_t send_buffer[length+9];
    send_buffer[0] = 9+length;
    send_buffer[1] = radio_wifi_get_node_id() & 0xFF;
    send_buffer[2] = (radio_wifi_get_node_id() >> 8) & 0xFF;
    send_buffer[3] = (radio_wifi_get_node_id() >> 16) & 0xFF;
    send_buffer[4] = (radio_wifi_get_node_id() >> 24) & 0xFF;
    send_buffer[5] = dest & 0xFF;
    send_buffer[6] = (dest >> 8) & 0xFF;
    send_buffer[7] = (dest >> 16) & 0xFF;
    send_buffer[8] = (dest >> 24) & 0xFF;
    memcpy(send_buffer+9, payload, length);
    int retval = write(radio_wifi_sockfd, send_buffer, length+9);
    DEBUG_LOG(DBG_WKCOMM, "Message sent to %d, length %d, retval %d\n", dest, length, retval);
    if (retval != -1)
        return 0;
    else
        return -1;
}

#endif // RADIO_USE_WIFI
