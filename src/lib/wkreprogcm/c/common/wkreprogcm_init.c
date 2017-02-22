#include "types.h"
#include "hooks.h"
#include "heap.h"
#include "wkcomm.h"
#include "wkreprog_comm.h"

dj_hook wkreprog_comm_handleMessageHook;

void wkreprogcm_init() {
	wkreprog_comm_handleMessageHook.function = wkreprog_comm_handle_message;
	dj_hook_add(&wkcomm_handle_message_hook, &wkreprog_comm_handleMessageHook);
}
