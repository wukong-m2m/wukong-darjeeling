#include "types.h"
#include "panic.h"
#include "debug.h"
#include "array.h"
#include "hooks.h"
#include "execution.h"
#include "heap.h"
#include "djarchive.h"
#include "core.h"
#include "jlib_base.h"

void javax_rtc_RTCTest_short_SimpleTestNative(void) {
	dj_exec_stackPushShort(39);
}