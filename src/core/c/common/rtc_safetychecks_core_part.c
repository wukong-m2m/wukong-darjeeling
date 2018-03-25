#ifdef AOT_SAFETY_CHECKS

#include "panic.h"
#include "heap.h"
#include "rtc_safetychecks_core_part.h"

uint16_t rtc_safety_heap_lowbound = 0;
// Initially the whole heap is safe to write.
// Before the application starts a GC run will move all permanent globals to the lower end of the heap,
// and we will raise the lowerbound to be just above the system objects put on the heap during VM init.
// These system objects are things like a VM and thread object, which should be protected, and an infusion
// object, which we can write to using PUTSTATIC, but that will be safe since we verify the static slot
// number exists.
void rtc_safety_mark_heap_bounds() {
   rtc_safety_heap_lowbound = (uint16_t)left_pointer;
   // avroraPrintHex32(0xEAEAEAEA);
   // avroraPrintHex16((int16_t)rtc_safety_heap_lowbound);
   // avroraPrintHex16((int16_t)right_pointer);
   // avroraPrintHex32(0xEAEAEAEA);
}

void rtc_safety_abort_with_error(uint8_t error) {
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintInt16(error);
    dj_panic(DJ_PANIC_UNSAFE_CODE_REJECTED);
}

#endif // AOT_SAFETY_CHECKS