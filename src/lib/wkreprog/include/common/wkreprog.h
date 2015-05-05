#ifndef WKREPROG_H
#define WKREPROG_H

#include "types.h"
#include "wkreprog_impl.h"

extern bool wkreprog_open_file(uint8_t filenumber, uint16_t start_write_position);

// For now these can be the same.
// Get the page size
#define wkreprog_get_page_size wkreprog_impl_get_page_size
// Open reprogramming at a position within the app archive
#define wkreprog_open_app_archive wkreprog_impl_open_app_archive
// Open reprogramming at any position in flash
#define wkreprog_open_raw wkreprog_impl_open_raw
// Write a number of bytes to the current position, flushing to flash if we cross a page boundary
#define wkreprog_write(size, data) wkreprog_impl_write(size, data, 0)
// Skip a number of bytes to the current position, flushing to flash if we cross a page boundary
#define wkreprog_skip(size) wkreprog_impl_write(size, 0, 1)
// Finish writing to flash and flush the page currently being written to
#define wkreprog_close wkreprog_impl_close


// Reboot
#define wkreprog_reboot wkreprog_impl_reboot

// Get the position of the next byte to be written
#define wkreprog_get_raw_position wkreprog_impl_get_raw_position



#endif // WKREPROG_H
