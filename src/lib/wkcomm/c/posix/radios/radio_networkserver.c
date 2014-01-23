#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "types.h"
#include "config.h"
#include "radio_networkserver.h"
#include "posix_utils.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"


// Protocol format: see WuKongNetworkServer.java
#define MODE_MESSAGE 1
#define MODE_DISCOVERY 2

int radio_networkserver_sockfd;
struct sockaddr_in radio_networkserver_servaddr;
uint8_t radio_networkserver_receive_buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // 4 for local network overhead, 3 for wkcomm overhead


void open_connection() {
	struct sockaddr_in radio_networkserver_servaddr;

	radio_networkserver_sockfd=socket(AF_INET,SOCK_STREAM,0);

	memset(&radio_networkserver_servaddr, 0, sizeof(radio_networkserver_servaddr));
	radio_networkserver_servaddr.sin_family = AF_INET;
	radio_networkserver_servaddr.sin_addr.s_addr=inet_addr(posix_network_server_address);
	radio_networkserver_servaddr.sin_port=htons(posix_network_server_port);

	int retval = connect(radio_networkserver_sockfd, (struct sockaddr *)&radio_networkserver_servaddr, sizeof(radio_networkserver_servaddr));

	if (retval != 0) {
		fprintf(stderr, "Unable to establish local radio connection: %d\n", errno);
	}
	int length=recv(radio_networkserver_sockfd, radio_networkserver_receive_buffer, 1, 0);
	if (length != 1 || radio_networkserver_receive_buffer[0] != 42) {
		fprintf(stderr, "Unable to establish local radio connection.\n");
	}
	uint8_t send_buffer[3];
	// Connect in messaging mode
	send_buffer[0] = MODE_MESSAGE;
	// Tell the server our network id
	send_buffer[1] = radio_networkserver_get_node_id() & 0xFF;
	send_buffer[2] = (radio_networkserver_get_node_id() >> 8) & 0xFF;
    retval = write(radio_networkserver_sockfd, send_buffer, 3);
	if (retval == -1) {
		fprintf(stderr, "Unable to send local network id to server: %d\n", errno);
		exit(1);
	}
}

void radio_networkserver_init(void) {
	open_connection();
}

radio_networkserver_address_t radio_networkserver_get_node_id() {
	return posix_local_network_id;
}

void radio_networkserver_poll(void) {
	uint8_t length;
	if (recv(radio_networkserver_sockfd, &length, 1, MSG_DONTWAIT) > 0) {
		if (length == 0) {
			// Just a heartbeat.
			return;
		}
		recv(radio_networkserver_sockfd, radio_networkserver_receive_buffer, 2, 0);
		radio_networkserver_address_t src = radio_networkserver_receive_buffer[0] + 256*radio_networkserver_receive_buffer[1];
		recv(radio_networkserver_sockfd, radio_networkserver_receive_buffer, 2, 0); // skip dest
		recv(radio_networkserver_sockfd, radio_networkserver_receive_buffer, length-5, 0); // skip dest
		printf("-------------------> message received from %d, length %d\n", src, length-5);
		routing_handle_local_message(src, radio_networkserver_receive_buffer, length-5);
	}
}

uint8_t radio_networkserver_send(radio_networkserver_address_t dest, uint8_t *payload, uint8_t length) {
	uint8_t send_buffer[length+5];
	send_buffer[0] = 5+length;
	send_buffer[1] = radio_networkserver_get_node_id() & 0xFF;
	send_buffer[2] = (radio_networkserver_get_node_id() >> 8) & 0xFF;
	send_buffer[3] = dest & 0xFF;
	send_buffer[4] = (dest >> 8) & 0xFF;
	memcpy(send_buffer+5, payload, length);
    int retval = write(radio_networkserver_sockfd, send_buffer, length+5);
	printf("-------------------> message sent to %d, length %d\n", dest, length);
	if (retval != -1)
		return 0;
	else
		return -1;
}
