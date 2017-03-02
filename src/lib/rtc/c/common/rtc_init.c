#include "config.h"
#include "rtc.h"

extern const unsigned char rtc_start_of_compiled_code_marker;

void rtc_init() {
	RTC_SET_START_OF_NEXT_METHOD(RTC_START_OF_COMPILED_CODE_SPACE);
#ifdef EXECUTION_PRINT_CALLS_AND_RETURNS
	avroraRTCPrintAllRuntimeAOTCalls();
#endif
}
