#include "heap.h"
#include "config.h"

extern uint8_t *avr_flash_pagebuffer;

void wkreprog_mem_markRootSet(void *data) {
	if (avr_flash_pagebuffer != NULL) {
		dj_mem_setChunkColor(avr_flash_pagebuffer, TCM_BLACK);
	}
}

void wkreprog_mem_updatePointers(void *data) {
	if (avr_flash_pagebuffer != NULL) {
		avr_flash_pagebuffer = dj_mem_getUpdatedPointer(avr_flash_pagebuffer);
	}
}
