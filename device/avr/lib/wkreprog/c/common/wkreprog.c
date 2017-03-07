#include "types.h"
#include "djarchive.h"
#include "wkreprog_impl.h"

bool wkreprog_open(uint8_t filenumber, uint16_t start_write_position) {
	dj_di_pointer file = dj_archive_get_file(di_app_archive, filenumber);
	if (dj_archive_number_of_files(di_app_archive) <= filenumber
			|| dj_archive_filesize(file) <= start_write_position)
		return false;
	return wkreprog_impl_open(file+start_write_position - di_app_archive);
}
