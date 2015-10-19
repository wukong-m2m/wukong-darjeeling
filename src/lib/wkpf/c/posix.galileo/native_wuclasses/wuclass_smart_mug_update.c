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
#include <netdb.h>
#include <netinet/in.h>

#define SERVER_PORT "2345" // the port client will be connecting to 
#define SERVER_IP "172.16.0.94"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define TV    1
#define LIGHT 2

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int    smart_mug_sockfd;            // Socket descriptors for server

void wuclass_smart_mug_setup(wuobject_t *wuobject) {
        
        struct addrinfo hints, *servinfo;
        int rv;
        char s[INET6_ADDRSTRLEN];
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &servinfo)) != 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): getaddrinfo error\n");
            return;
        }

        if ((smart_mug_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
        {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): socket error\n");
            return;
        }

        if((connect(smart_mug_sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
        {
            close(smart_mug_sockfd);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): connet error\n");
            return;
        }

        inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr), s, sizeof s);

        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): connet to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): socket %d\n", smart_mug_sockfd);
}

void wuclass_smart_mug_update(wuobject_t *wuobject) {
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): wait socket %d\n", smart_mug_sockfd);
    static char buffer[MAXDATASIZE]={'0'};        // Input and Receive buffer
    fd_set rs;
    struct timeval to={0,0};
    FD_ZERO(&rs);
    FD_SET(smart_mug_sockfd, &rs);
    int numbytes;
    if (select(smart_mug_sockfd+1, &rs, NULL, NULL, &to) > 0) {
        if ((numbytes = recv(smart_mug_sockfd, buffer, MAXDATASIZE-1, 0)) == -1) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): rev error\n");
        }
        buffer[numbytes] = '\0';
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): client received %s\n", buffer);
    }

    char *p;
    int tmp_value = 0;
    int id_angle_info[2]={0};
    int count =0;
    for (p = buffer; *p != '\0' && count < 2; p++)
    {
      if(*p != ','){
        id_angle_info[count] = tmp_value * 10 + (*p - '0');
        tmp_value = id_angle_info[count];
      }else{
        count++;
        tmp_value =0;
      }
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): id %d\n", id_angle_info[0]);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): angle %d\n", id_angle_info[1]);

    if(id_angle_info[0] == TV){
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SMART_MUG_VOLUME, id_angle_info[0]);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): TV_volume %d\n", id_angle_info[0]);
    }else if(id_angle_info[1] == LIGHT){
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SMART_MUG_COLOR, id_angle_info[1]);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Smart_Mug): LIGHT_color %d\n", id_angle_info[1]);
    }
}
