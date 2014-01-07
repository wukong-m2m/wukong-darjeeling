#include "config.h" // To get RADIO_USE_ZWAVE
#include "djtimer.h"
#include "posix_utils.h"

#ifdef RADIO_USE_ZWAVE

extern uint32_t zwave_time_btn_push;
extern uint32_t zwave_time_btn_release;
extern bool zwave_btn_is_push;
extern bool zwave_btn_is_release;
extern bool radio_zwave_my_address_loaded;

dj_time_t zwave_time_init = 0;

void radio_zwave_platform_dependent_init(void) {
}

void radio_zwave_platform_dependent_poll(void) {
	if (posix_arg_addnode) {
		// Wait until Zwave has initialised. Then "press" the button for 1 second.
		if (radio_zwave_my_address_loaded) {
			// Zwave is initialised.
			if (zwave_time_init == 0) {
				zwave_time_init = dj_timer_getTimeMillis();
			}
			// Wait 1s before pressing
			if ((dj_timer_getTimeMillis()-zwave_time_init > 1000)
					&& zwave_btn_is_push == false) {
				zwave_btn_is_push = true;
				zwave_time_btn_push = dj_timer_getTimeMillis();
			}
			// And release after 1 s
			if ((dj_timer_getTimeMillis()-zwave_time_btn_push > 1000)
					&& zwave_btn_is_push == true) {
				zwave_btn_is_push = false;
				zwave_btn_is_release = true;
				zwave_time_btn_release = dj_timer_getTimeMillis();
				posix_arg_addnode = false; // or else the next loop will "press" it again
			}
		}
	}
}

#endif // RADIO_USE_ZWAVE