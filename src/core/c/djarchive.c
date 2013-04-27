#include "djarchive.h"

uint8_t dj_archive_number_of_files(dj_di_pointer archive) {
	uint8_t count = 0;
	uint32_t size;
	while ((size = dj_di_getU16(archive)) != 0) {
		archive += size; // Skip over this file
		archive += 3; // Skip over the size (2 bytes) and type (1 byte)
		count++;
	}
	return count;
}
dj_di_pointer dj_archive_get_file(dj_di_pointer archive, uint8_t filenumber) {
	while(filenumber != 0) {
		archive += dj_di_getU16(archive); // Skip over this file
		archive += 3; // Skip over the size (2 bytes) and type (1 byte)
		filenumber--;
	}
	return archive+3; // +3 to skip the size and type at the beginning of each file
}
