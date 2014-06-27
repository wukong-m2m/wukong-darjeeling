#include "panic.h"
#include "wkreprog_impl.h"

extern char** posix_argv;

uint8_t *wkreprog_impl_buffer;
int wkreprog_impl_pos;

uint16_t wkreprog_impl_get_page_size() {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	return 0;
}

bool wkreprog_impl_open_app_archive(uint16_t start_write_position) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	return false;
}

bool wkreprog_impl_open_raw(uint16_t start_write_position) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	return false;
}

void wkreprog_impl_write(uint8_t size, uint8_t* data, bool skip) {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
}

void wkreprog_impl_close() {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
}

void wkreprog_impl_reboot() {
	dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
}
