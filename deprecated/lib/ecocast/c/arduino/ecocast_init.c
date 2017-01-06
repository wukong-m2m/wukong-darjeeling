#include "wkcomm.h"
#include "debug.h"
#include "ecocast_comm.h"

dj_hook ecocast_comm_handleMessageHook;

void ecocast_init() {
	ecocast_comm_handleMessageHook.function = ecocast_comm_handle_message;
	dj_hook_add(&wkcomm_handle_message_hook, &ecocast_comm_handleMessageHook);
	DEBUG_LOG(DBG_ECO, "[ECO] Init\n");
}
