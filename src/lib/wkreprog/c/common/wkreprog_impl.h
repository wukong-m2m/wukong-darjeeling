#ifndef WKREPROG_IMPL_H
#define WKREPROG_IMPL_H

#include "types.h"

extern uint16_t wkreprog_impl_get_page_size();
extern bool wkreprog_impl_open(uint16_t start_write_position);
extern void wkreprog_impl_write(uint8_t size, uint8_t* data);
extern void wkreprog_impl_close();
extern void wkreprog_impl_reboot();

#endif // WKREPROG_IMPL_H
