#include <stdint.h>

#define ECOCAST_PANIC_CANT_FIND_CAPSULE_FILE 120

uint8_t ecocast_find_capsule_filenr();
bool ecocast_find_capsule_padding_or_empty_space(uint16_t length, uint8_t *hash, uint16_t *offset_in_capsule_file);
void ecocast_execute_code_capsule_at_offset(uint16_t offset_in_capsule_file, uint8_t retval_size, uint8_t* retval_buffer);

