#include "types.h"
#include "parse_infusion.h"

void dj_di_read_methodImplHeader(dj_methodImplementation *header_data, dj_di_pointer methodimpl) {
	uint8_t *header_data_ptr = (void *)header_data;
	for (uint8_t i=0; i<sizeof(dj_methodImplementation); i++) {
		*header_data_ptr++ = dj_di_getU8(methodimpl++);
	}
}
