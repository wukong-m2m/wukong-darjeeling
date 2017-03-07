#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else //_WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif //_WIN32

#ifdef _WIN32
	#ifndef _PACKED_1_
		#define _PACKED_1_
	#endif
#else //_WIN32
	#ifndef _PACKED_1_
		#define _PACKED_1_ __attribute__ ((packed))
	#endif
#endif //_WIN32


#ifdef _WIN32
#pragma pack(1)
#endif //_WIN32

#define BUFFSIZE 255
void Die(char *mess) { perror(mess); exit(1); }
typedef struct {
    unsigned short version;
	unsigned short length;
	unsigned int connectionID;
	unsigned short op;
	unsigned short deviceID;
	unsigned char mac[6];
	unsigned int ip _PACKED_1_;
	unsigned int netmask _PACKED_1_;
	unsigned int gateway _PACKED_1_;
} Response;

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    unsigned char buffer[BUFFSIZE];
    int echolen, clientlen;
    int received = 0;
	int l=1;
	Response *res;
	struct in_addr addr;
#ifdef _WIN32
	WSADATA wsaData;
#endif //_WIN32

#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup error\n");
		return 1;
	}
#endif //_WIN32

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Failed to create socket");
    }
	setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(void*)&l,4);

    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = ~0;
    echoserver.sin_port = htons(8000);       /* server port */
    buffer[0] = 1;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0xc0;
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 1;
    buffer[10] = 0xff;
    buffer[11] = 0xff;
    echolen=12;
    if (sendto(sock, (void*)buffer, echolen, 0,
                     (struct sockaddr *) &echoserver,
                       sizeof(echoserver)) != echolen) {
       Die("Mismatch in number of sent bytes");
    }
    /* Receive the word back from the server */
    fprintf(stdout, "Received:\n");
    clientlen = sizeof(echoclient);
    while(1) {
        received = recvfrom(sock, (void*)buffer, BUFFSIZE, 0,
                             (struct sockaddr *) &echoclient,
                             &clientlen);
        buffer[received] = '\0';        /* Assure null terminated string */
        res = (Response *) buffer;
	printf("mac=%02x:%02x:%02x:%02x:%02x:%02x\t", res->mac[0],res->mac[1],res->mac[2],res->mac[3],res->mac[4],res->mac[5]);
	memcpy(&addr.s_addr,(void *) &res->ip,4);
	printf("ip=%s\n", inet_ntoa(addr));
    }
    close(sock);

#ifdef _WIN32
	WSACleanup();
#endif //_WIN32

    exit(0);
}
