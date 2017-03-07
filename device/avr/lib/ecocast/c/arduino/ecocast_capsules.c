#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>
#include <stdint.h>
#include "djarchive.h"
#include "debug.h"
#include "panic.h"
#include "program_mem.h"
#include "wkreprog.h"
#include "ecocast_capsules.h"

uint8_t ecocast_find_capsule_filenr() {
	for (uint8_t i=0; i<dj_archive_number_of_files(di_app_archive); i++) {
		dj_di_pointer file = dj_archive_get_file(di_app_archive, i);
		if (dj_archive_filetype(file) == DJ_FILETYPE_ECOCAST_CAPSULE_BUFFER) {
			return i;
		}
	}
	dj_panic(ECOCAST_PANIC_CANT_FIND_CAPSULE_FILE);
	return 0; // Keep compiler happy
}

dj_di_pointer ecocast_find_capsule_code_address(uint16_t offset_in_capsule_file) {
	// Add an additional 6 bytes to skip the header (lenght+hash)
	return dj_archive_get_file(di_app_archive, ecocast_find_capsule_filenr()) + offset_in_capsule_file + 6;
}

void ecocast_do_execute_with_1byte_retval(uint16_t programme_counter_address, uint8_t* retval_buffer) {
	// Somehow, this won't compile unless I push/pop Y onto the stack (r28/29). Strange, since I'm not changing Y here.
	asm("ICALL\n" \
		"PUSH R28\n" \
		"PUSH R29\n" \
		"ST Y, R24\n" \
		"POP R29\n" \
		"POP R28\n" \
		: \
		: "z"(programme_counter_address), "y"(retval_buffer));
}

void ecocast_do_execute_with_2byte_retval(uint16_t programme_counter_address, uint8_t* retval_buffer) {
	asm("ICALL\n" \
		"PUSH R28\n" \
		"PUSH R29\n" \
		"ST Y+, R24\n" \
		"ST Y+, R25\n" \
		"POP R29\n" \
		"POP R28\n" \
		: \
		: "z"(programme_counter_address), "y"(retval_buffer));
}
void ecocast_do_execute_with_4byte_retval(uint16_t programme_counter_address, uint8_t* retval_buffer) {
	asm("ICALL\n" \
		"PUSH R28\n" \
		"PUSH R29\n" \
		"ST Y+, R22\n" \
		"ST Y+, R23\n" \
		"ST Y+, R24\n" \
		"ST Y+, R25\n" \
		"POP R29\n" \
		"POP R28\n" \
		: \
		: "z"(programme_counter_address), "y"(retval_buffer));
}

void ecocast_execute_code_capsule_at_offset(uint16_t offset_in_capsule_file, uint8_t retval_size, uint8_t* retval_buffer) {
	// According to http://gcc.gnu.org/wiki/avr-gcc
	//  Z (R30, R31) is call used and could be destroyed by the function in the capsule, but since we already did the ICALL then this doesn't matter
	//  Y (R28, R29) is call saved. This means it's still safe after ICALL, but we need to restore the contents if we modify it (for 16 and 32 bit retval)

	// TODONR: this doesn't work if capsule is above 64K
	dj_di_pointer capsule = ecocast_find_capsule_code_address(offset_in_capsule_file);
	uint16_t programme_counter_address = capsule/2; // PC is in words, not bytes.

	switch (retval_size) {
		case 1:
			ecocast_do_execute_with_1byte_retval(programme_counter_address, retval_buffer);
		break;
		case 2:
			ecocast_do_execute_with_2byte_retval(programme_counter_address, retval_buffer);
		break;
		case 4:
			ecocast_do_execute_with_4byte_retval(programme_counter_address, retval_buffer);
		break;
	}
}


uint8_t ecocast_find_capsule_padding_or_empty_space(uint16_t length, uint8_t *hash, uint16_t *offset_in_capsule_file) {
	// This will look for the capsule with the specified length and hash.
	// If found:
	// 		offset_in_capsule_file = the offset of the found capsule (start of header, not code)
	//      return true
	// If not found:
	//		offset_in_capsule = the offset of the first free position in the capsule file, where free space starts
	//		return false
	uint8_t capsule_filenr = ecocast_find_capsule_filenr();
	dj_di_pointer capsule_file_start = dj_archive_get_file(di_app_archive, capsule_filenr);
	dj_di_pointer pos = capsule_file_start;
	if ((capsule_file_start & 1) == 1)
		pos++; // Start on the first even address in the file
	DEBUG_LOG(DBG_ECO, "[ECO] Looking for capsule with length %d, hash 0x%x 0x%x 0x%x 0x%x: ", length, hash[0], hash[1], hash[2], hash[3]);
	uint16_t cur_capsule_length;
	while ((cur_capsule_length = dj_di_getU16(pos)) != 0) {
		if (cur_capsule_length == length
				&& dj_di_getU8(pos+2) == hash[0]
				&& dj_di_getU8(pos+3) == hash[1]
				&& dj_di_getU8(pos+4) == hash[2]
				&& dj_di_getU8(pos+5) == hash[3]) {
			// Capsule found.
			*offset_in_capsule_file = (uint16_t)(pos-capsule_file_start);
			DEBUG_LOG(DBG_ECO, "\n[ECO] Found! Returning offset %d as the start of the capsule's header.\n", *offset_in_capsule_file);
			return ECOCAST_CAPSULE_FOUND;
		} else {
			// Capsule doesn't match. Skip to the next.
			DEBUG_LOG(DBG_ECO, "[no match @ %d, skip %d bytes] ", (uint16_t)(pos-capsule_file_start), cur_capsule_length);
			pos += cur_capsule_length;
		}
	}

	// End of the capsule list. Capsule not found. See if there's enough free space left in the buffer to store the capsule.
	*offset_in_capsule_file = (uint16_t)(pos-capsule_file_start);
	uint16_t capsule_file_size = dj_archive_filesize(dj_archive_get_file(di_app_archive, capsule_filenr));
	uint16_t free_space = capsule_file_size - *offset_in_capsule_file;
	DEBUG_LOG(DBG_ECO, "\n[ECO] Not found. Free space left in buffer %d. Capsule size %d.\n", free_space, length);
	if (free_space < length) {
		*offset_in_capsule_file = (capsule_file_start & 1); // (will be 0 or 1: still need to start at first even address)
		free_space = capsule_file_size - *offset_in_capsule_file;
		DEBUG_LOG(DBG_ECO, "[ECO] Not enough free space. Reset buffer. Free space is now %d.\n", free_space);
	}
	if (free_space < length) {
		DEBUG_LOG(DBG_ECO, "[ECO] Still not enough free space. The buffer's simply too small for this capsule.\n", free_space);
		return ECOCAST_BUFFER_TOO_SMALL;
	} else {
		DEBUG_LOG(DBG_ECO, "[ECO] Receive new capsule at offset %d.\n", *offset_in_capsule_file);
		return ECOCAST_CAPSULE_NOT_FOUND;
	}
	DEBUG_LOG(DBG_ECO, "[ECO] Not found. Returning offset %d as the first free position in the file.\n", *offset_in_capsule_file);
	return false;


}
