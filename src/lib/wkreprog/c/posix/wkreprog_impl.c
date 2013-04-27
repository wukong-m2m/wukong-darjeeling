#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "wkreprog_impl.h"

#include "posix_utils.h"

uint8_t *wkreprog_impl_buffer;
int wkreprog_impl_pos;

uint16_t wkreprog_impl_get_page_size() {
	return 256;
}

bool wkreprog_impl_open(uint16_t size_to_upload) {
	wkreprog_impl_buffer = malloc(size_to_upload);
	memset(wkreprog_impl_buffer, 0, size_to_upload);
	wkreprog_impl_pos = 0;
	return (wkreprog_impl_buffer != NULL);
}

void wkreprog_impl_write(uint8_t size, uint8_t* data) {
	memcpy(wkreprog_impl_buffer+wkreprog_impl_pos, data, size);
	wkreprog_impl_pos += size;
}

void wkreprog_impl_close() {
  FILE *fp = fopen("app_infusion.dja", "w");
  if (fp== NULL) {
	  printf("Error in opening file to write infusion to...\n");
  }
  fwrite(wkreprog_impl_buffer, wkreprog_impl_pos, 1, fp);
  fclose(fp);
}

void wkreprog_impl_reboot() {
	execvp(posix_argv[0], posix_argv);
}
