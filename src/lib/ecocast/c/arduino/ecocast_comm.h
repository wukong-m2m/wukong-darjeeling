#ifndef ECOCAST_COMM_H
#define ECOCAST_COMM_H

#define ECOCAST_COMM_ECOCOMMAND                 0x20
#define ECOCAST_COMM_ECOCOMMAND_R               0x21

extern void ecocast_comm_handle_message(void *msg); // Will be called with a pointer to a wkcomm_received_msg

#endif // ECOCAST_COMM_H
