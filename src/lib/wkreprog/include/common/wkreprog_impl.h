#ifndef WKREPROG_IMPL_H
#define WKREPROG_IMPL_H

#include "types.h"

extern uint16_t wkreprog_impl_get_page_size();
extern bool wkreprog_impl_open_app_archive(uint16_t start_write_position);
extern bool wkreprog_impl_open_raw(uint_farptr_t start_write_position, uint_farptr_t end_of_safe_region);
extern void wkreprog_impl_write(uint8_t size, uint8_t* data, bool skip);
extern void wkreprog_impl_close();
extern void wkreprog_impl_reboot();

extern uint_farptr_t wkreprog_impl_get_raw_position();

#endif // WKREPROG_IMPL_H
