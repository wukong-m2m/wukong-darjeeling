#include <string.h>
#include "types.h"
#include "debug.h"
#include "panic.h"
#include "wkreprog_impl.h"
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// This function should be in the NRWW section to allow it to write to flash
//void BOOTLOADER_SECTION avr_flash_program_page(uint_farptr_t page, uint8_t *buf);
void __attribute__ ((section (".reprogram_flash_page"))) avr_flash_program_page (uint_farptr_t page, uint8_t *buf);
extern unsigned char di_app_infusion_archive_data[];

// TODO (if we run short on RAM): this can probably be made more
// efficient by splitting this up in two parts and writing directly
// to the flash page buffer while receiving the code using
// boot_page_fill, and then writing the buffer to flash in a separate
// function.
// Alternatively we could try to reclaim heap space from the vm first,
// since it can only reboot once we've started the reprogramming phase.
uint8_t avr_flash_pagebuffer[SPM_PAGESIZE];
uint_farptr_t avr_flash_pageaddress = 0;
uint_farptr_t avr_flash_end_of_safe_region = 0;
uint16_t avr_flash_buf_len = 0;

uint16_t wkreprog_impl_get_page_size() {
	return SPM_PAGESIZE;
}

uint_farptr_t wkreprog_impl_get_raw_position() {
	return avr_flash_pageaddress // The start address of the current page
			+ avr_flash_buf_len; // The offset within the page
}


// Open reprogramming at a position within the app archive
bool wkreprog_impl_open_app_archive(uint16_t start_write_position) {
	void *x = (void *)di_app_infusion_archive_data;
	return wkreprog_impl_open_raw((uint16_t)x + start_write_position, 0); // TODONR: proper end of safe region (restrict writing to the app infusion)
}

// Open reprogramming at any position in flash
// If end_of_safe_region is set, the VM will panic when writing outside of this region.
bool wkreprog_impl_open_raw(uint_farptr_t start_write_position, uint_farptr_t end_of_safe_region) {
avroraStartReprogTimer();
	DEBUG_LOG(DBG_WKREPROG, "AVR: Start writing to flash at address %p.\n", start_write_position);

	// Set the position to start writing at start_write_position
	// avr_flash_pageaddress should point at a page start
	avr_flash_pageaddress = start_write_position - (start_write_position % SPM_PAGESIZE);
	// Set the position within the current page
	avr_flash_buf_len = start_write_position % SPM_PAGESIZE;
	// Fill the buffer with the data currently in the file
	for (int i=0; i<SPM_PAGESIZE; i++)
		avr_flash_pagebuffer[i] = dj_di_getU8(avr_flash_pageaddress+i);
	avr_flash_end_of_safe_region = end_of_safe_region;

avroraStopReprogTimer();
	return true;
}

void avr_flash_program_page_if_not_modified(uint_farptr_t page, uint8_t *buf) {
	// Don't wear out the flash unnecessarily
	for (uint16_t i=0; i<SPM_PAGESIZE; i++) {
		if (avr_flash_pagebuffer[i] != dj_di_getU8(page+i)) {
			// Buffer and flash memory differ. Write the page.
			avr_flash_program_page (page, buf);
			return;
		}
	}
}


void wkreprog_impl_write(uint8_t size, uint8_t* data, bool skip) {
avroraStartReprogTimer();
	// TODONR: Check if the size fits in the allocated space for app archive
	if (avr_flash_pageaddress == 0)
		return;
	DEBUG_LOG(DBG_WKREPROG, "AVR: Received %d bytes to flash to page 0x%x.\n", size, avr_flash_pageaddress);
	DEBUG_LOG(DBG_WKREPROG, "AVR: Buffer already contains %d bytes.\n", avr_flash_buf_len);
	DEBUG_LOG(DBG_WKREPROG, "AVR: Writing to 0x%x: ", avr_flash_pageaddress+avr_flash_buf_len);
	for (uint8_t i=0; i<size; i++) {
		DEBUG_LOG(DBG_WKREPROG, " %x", data[i]);
	}
	DEBUG_LOG(DBG_WKREPROG, "\n");


	if ((avr_flash_pageaddress + avr_flash_buf_len + size) > avr_flash_end_of_safe_region) {
		dj_panic(DJ_PANIC_REPROGRAM_OUTSIDE_REGION);
	}

	while(size!=0) {
		uint8_t bytes_on_this_page = size;
		if (avr_flash_buf_len + size > SPM_PAGESIZE) {
			// Only 1 page at a time
			bytes_on_this_page = SPM_PAGESIZE-avr_flash_buf_len;
		}
		if (!skip)
			memcpy(avr_flash_pagebuffer + avr_flash_buf_len, data, bytes_on_this_page); // Copy the data to the page buffer
		if (avr_flash_buf_len + bytes_on_this_page == SPM_PAGESIZE) { // If we filled a whole page, write it to flash
			DEBUG_LOG(DBG_WKREPROG, "AVR: Flashing page at 0x%x.\n", avr_flash_pageaddress);
			avr_flash_program_page_if_not_modified(avr_flash_pageaddress, avr_flash_pagebuffer);
			avr_flash_pageaddress += SPM_PAGESIZE;
			// Fill the buffer with the data currently in the file
			for (int i=0; i<SPM_PAGESIZE; i++)
				avr_flash_pagebuffer[i] = dj_di_getU8(avr_flash_pageaddress+i);
		}
		avr_flash_buf_len = (avr_flash_buf_len + bytes_on_this_page) % SPM_PAGESIZE;
		size -= bytes_on_this_page;
		data += bytes_on_this_page;
	}
avroraStopReprogTimer();
}

void wkreprog_impl_close() {
avroraStartReprogTimer();
	DEBUG_LOG(DBG_WKREPROG, "AVR: Closing flash file.\n");
	if (avr_flash_buf_len != 0) { // If there's any data remaining, write it to flash.
		// Fill the remaining of the buffer with the old data currently in the file
		for (int i=avr_flash_buf_len; i<SPM_PAGESIZE; i++)
			avr_flash_pagebuffer[i] = dj_di_getU8(avr_flash_pageaddress+i);

		DEBUG_LOG(DBG_WKREPROG, "AVR: Flashing page at 0x%x.\n", avr_flash_pageaddress);
		avr_flash_program_page_if_not_modified(avr_flash_pageaddress, avr_flash_pagebuffer);
	}
	avr_flash_pageaddress = 0;
avroraStopReprogTimer();
}

void wkreprog_impl_reboot() {
	// Reset using the watchdog timer.
#if defined(WDTCSR)
	WDTCSR = _BV(WDCE);
	WDTCSR = _BV(WDE);
#else
	WDTCR = _BV(WDCE);
	WDTCR = _BV(WDE);
#endif
	while (1) {
	}
}

// Copied from avr/boot.h example
void avr_flash_program_page (uint_farptr_t page, uint8_t *buf)
{
	uint16_t i;
	uint8_t sreg;

	// Disable interrupts.
	sreg = SREG;
	cli();

	eeprom_busy_wait ();

	boot_page_erase (page);
	boot_spm_busy_wait ();      // Wait until the memory is erased.

	for (i=0; i<SPM_PAGESIZE; i+=2)
	{
		// Set up little-endian word.
		uint16_t w = *buf++;
		w += (*buf++) << 8;
		boot_page_fill (page + i, w);
	}

	boot_page_write (page);     // Store buffer in flash page.
	boot_spm_busy_wait();       // Wait until the memory is written.

	// Reenable RWW-section again. We need this if we want to jump back
	// to the application after bootloading.
	boot_rww_enable ();

	// Re-enable interrupts (if they were ever enabled).
	SREG = sreg;
}
