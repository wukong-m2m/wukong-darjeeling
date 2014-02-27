#ifndef ECOCAST_COMM_H
#define ECOCAST_COMM_H

#define ECOCAST_COMM_ECOCOMMAND                 0x20
#define ECOCAST_COMM_ECOCOMMAND_R               0x21

#define ECOCAST_REPLY_OK						0x01 // Packet received, still waiting for the rest of the capsule
#define ECOCAST_REPLY_EXECUTED					0x02 // Packet received. Capsule was executed and reply contains return value in the following bytes. Sent after first or last message.
#define ECOCAST_REPLY_TOO_BIG					0x03 // Capsule is too big to fit in the capsule file. TODONR: not used yet.

extern void ecocast_comm_handle_message(void *msg); // Will be called with a pointer to a wkcomm_received_msg

#endif // ECOCAST_COMM_H
