#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>
#include <stdint.h>
#include "wkcomm.h"

// In flash:
const static uint8_t PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) ecocast_data[4096] = {};
// In ram:
const static uint8_t ecocast_buffer[SPM_PAGESIZE] = {};

void ecocast_comm_handle_message(void *data) {
//	wkcomm_received_msg *msg = (wkcomm_received_msg *)data;

}

