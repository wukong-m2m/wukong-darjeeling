#ifndef WKREPROG_H
#define WKREPROG_H

#include "types.h"

extern bool wkreprog_open(uint8_t filenumber, uint16_t start_write_position);

// For now these can be the same.
#define wkreprog_write wkreprog_impl_write
#define wkreprog_close wkreprog_impl_close
extern void wkreprog_impl_write(uint8_t size, uint8_t* data);
extern void wkreprog_impl_close();

#endif // WKREPROG_H
