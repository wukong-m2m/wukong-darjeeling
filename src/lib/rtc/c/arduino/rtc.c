#include "types.h"
#include "rtc.h"
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>

const unsigned char PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) rtc_compiled_code_buffer[RTC_COMPILED_CODE_BUFFER_SIZE];
dj_di_pointer rtc_compiled_code_next_free_byte = &rtc_compiled_code_buffer;

void rtc_compile_lib(dj_infusion *infusion) {

}

