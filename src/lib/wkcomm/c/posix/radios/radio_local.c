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
#include "radio_local.h"
#include "posix_utils.h"

// Here we have a circular dependency between radio_X and routing.
// Bit of a code smell, but since the two are supposed to be used together I'm leaving it like this for now.
// (routing requires at least 1 radio_ library to be linked in)
#include "../../common/routing/routing.h"


// Message format:
// 1 byte remaining length (=2+2+X), 2 byte source 2 byte dest, X byte payload


int radio_local_sockfd;
struct sockaddr_in radio_local_servaddr;
uint8_t radio_local_receive_buffer[WKCOMM_MESSAGE_PAYLOAD_SIZE+3]; // 4 for local network overhead, 3 for wkcomm overhead


void open_connection() {
	struct sockaddr_in radio_local_servaddr;

	radio_local_sockfd=socket(AF_INET,SOCK_STREAM,0);

	memset(&radio_local_servaddr, 0, sizeof(radio_local_servaddr));
	radio_local_servaddr.sin_family = AF_INET;
	radio_local_servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	radio_local_servaddr.sin_port=htons(10008);

	int retval = connect(radio_local_sockfd, (struct sockaddr *)&radio_local_servaddr, sizeof(radio_local_servaddr));

	if (retval != 0) {
		fprintf(stderr, "Unable to establish local radio connection: %d\n", errno);
	}
	int length=recv(radio_local_sockfd, radio_local_receive_buffer, 1, 0);
	if (length != 1 || radio_local_receive_buffer[0] != 42) {
		fprintf(stderr, "Unable to establish local radio connection.\n");
	}

	// Tell the server our network id
	uint8_t send_buffer[2];
	send_buffer[0] = radio_local_get_node_id() & 0xFF;
	send_buffer[1] = (radio_local_get_node_id() >> 8) & 0xFF;
    retval = write(radio_local_sockfd, send_buffer, 2);
	if (retval == -1)
		fprintf(stderr, "Unable to send local network id to server: %d\n", errno);
}

void radio_local_init(void) {
	open_connection();
}

radio_local_address_t radio_local_get_node_id() {
	return posix_local_network_id;
}

void radio_local_poll(void) {
	uint8_t length;
	if (recv(radio_local_sockfd, &length, 1, MSG_DONTWAIT) > 0) {
		if (length == 0) {
			// Just a heartbeat.
			return;
		}
		recv(radio_local_sockfd, radio_local_receive_buffer, 2, 0);
		radio_local_address_t src = radio_local_receive_buffer[0] + 256*radio_local_receive_buffer[1];
		recv(radio_local_sockfd, radio_local_receive_buffer, 2, 0); // skip dest
		recv(radio_local_sockfd, radio_local_receive_buffer, length-5, 0); // skip dest
		printf("-------------------> message received from %d, length %d\n", src, length);
		routing_handle_local_message(src, radio_local_receive_buffer, length-5);
	}
}

uint8_t radio_local_send(radio_local_address_t dest, uint8_t *payload, uint8_t length) {
	uint8_t send_buffer[length+5];
	send_buffer[0] = 5+length;
	send_buffer[1] = radio_local_get_node_id() & 0xFF;
	send_buffer[2] = (radio_local_get_node_id() >> 8) & 0xFF;
	send_buffer[3] = dest & 0xFF;
	send_buffer[4] = (dest >> 8) & 0xFF;
	memcpy(send_buffer+5, payload, length);
    int retval = write(radio_local_sockfd, send_buffer, length+5);
	printf("-------------------> message sent to %d, length %d\n", dest, length);
	if (retval != -1)
		return 0;
	else
		return -1;
}
