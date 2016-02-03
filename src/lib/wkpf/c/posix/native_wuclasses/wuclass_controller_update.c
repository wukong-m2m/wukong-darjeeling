#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <math.h>
#include "config.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 1024

long long start_time = 0;

long long str2ll (char * buffer) {
    long long a = 0;
    int i;
    for (i = 0; buffer[i]; i++) {
        if (buffer[i] < '0'|| buffer[i] > '9') break;
        a *= 10;
        a += buffer[i]-'0';
    }
    return a;
}

int    sockfd;            // Socket descriptors for server
struct sockaddr_in cliaddr;        // Broadcast Receiver Address
struct sockaddr_in srvaddr;     // Broadcast Server Address

void wuclass_controller_setup(wuobject_t *wuobject) {
    
    //int    broadcast = 1;    // Socket Option.
//    int    rcvlen = 0;                // Receive Message Size
    int    port = 456;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): socket fail\n");
        return;
    }
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr*) &cliaddr, sizeof(cliaddr)) == -1) {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): bind fail\n");
        return;
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): socket: %d\n", sockfd);
}

void wuclass_controller_update(wuobject_t *wuobject) {
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): wait socket %d\n", sockfd);
    char buffer[BUFSIZE];        // Input and Receive buffer
    int srvaddr_len = sizeof( srvaddr );
    fd_set rs;
    struct timeval to={0,0};
    FD_ZERO(&rs);
    FD_SET(sockfd, &rs);
    int16_t comm;
    bool timeUp = false;

    if (select(sockfd+1, &rs, NULL, NULL, &to) > 0) {
        recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&srvaddr, (socklen_t *)&srvaddr_len);
        start_time = str2ll(buffer);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): socket %lld\n", start_time);
        timeUp = true;
    }
    if (timeUp) {
        wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_CONTROLLER_INPUT, &comm);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONTROLLER_OUTPUT, comm);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONTROLLER_INPUT, 0);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): comm out %d\n", comm);
    } else {
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_CONTROLLER_OUTPUT, 0);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Controller): no comm\n");
    }
}