#ifndef WKREPROG_COMM_H
#define WKREPROG_COMM_H

#define WKREPROG_COMM_CMD_REPROG_OPEN                 0x10
#define WKREPROG_COMM_CMD_REPROG_OPEN_R               0x11
#define WKREPROG_COMM_CMD_REPROG_WRITE                0x12
#define WKREPROG_COMM_CMD_REPROG_WRITE_R              0x13
#define WKREPROG_COMM_CMD_REPROG_COMMIT               0x14
#define WKREPROG_COMM_CMD_REPROG_COMMIT_R             0x15
#define WKREPROG_COMM_CMD_REPROG_REBOOT               0x16

#define WKREPROG_OK									  0x00
#define WKREPROG_REQUEST_RETRANSMIT					  0x01
#define WKREPROG_TOOLARGE							  0x02
// Not used yet, but we should add some checksum mechanism at some point.
#define WKREPROG_FAILED								  0x03

extern void wkreprog_comm_handle_message(void *msg); // Will be called with a pointer to a wkcomm_received_msg

#endif // WKREPROG_COMM_H
