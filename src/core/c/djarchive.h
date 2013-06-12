#ifndef DJARCHIVEH
#define DJARCHIVEH

#include "program_mem.h"
#include "types.h"

// Archive format:
// Repeated for each file:
//   4 bytes file size
//   1 byte file type
//   X bytes file data
// 4 bytes: 00 00 00 00

#define DJ_FILETYPE_LIB_INFUSION 			0
#define DJ_FILETYPE_APP_INFUSION 			1
#define DJ_FILETYPE_WKPF_LINK_TABLE   		2
#define DJ_FILETYPE_WKPF_COMPONENT_MAP		3
#define DJ_FILETYPE_WKPF_INITVALUES_TABLE	4

#define dj_archive_filesize(file) (dj_di_getU32(file-3))
#define dj_archive_filetype(file) (dj_di_getU8(file-1))

// Contains the application archive. To be provided by main.c for each platform.
extern dj_di_pointer di_app_archive;

uint8_t dj_archive_number_of_files(dj_di_pointer archive);
dj_di_pointer dj_archive_get_file(dj_di_pointer archive, uint8_t filenumber);

#endif // DJARCHIVEH
