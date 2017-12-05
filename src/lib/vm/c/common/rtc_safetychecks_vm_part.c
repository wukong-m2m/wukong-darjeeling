#include "panic.h"
#include "heap.h"
#include "rtc_safetychecks_vm_part.h"

void rtc_safety_abort_with_error(uint8_t error) {
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintInt16(error);
    dj_panic(DJ_PANIC_UNSAFE_CODE_REJECTED);
}

