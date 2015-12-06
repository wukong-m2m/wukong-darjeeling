// #include "stackcache.h"
// #include "asm.h"

// #define RTC_STACKCACHE_NUMBER_OF_CACHE_REGS                   10
// #define RTC_STACKCACHE_REG_AVAILABLE                        0xFF
// #define RTC_STACKCACHE_REG_ALLOCATE_BUT_NOT_PUSHED          0xFF

// uint8_t rtc_stackcache_reg_order[] = { R18, R19, R20, R21, R22, R23, R24, R25 };
// uint8_t rtc_stackcache_state[NUMBER_OF_CACHE_REGS];

// void rtc_stackcache_init() {
// 	for (uint8_t i=0; i<NUMBER_OF_CACHE_REGS; i++) {
// 		rtc_stackcache_state[i] = RTC_STACKCACHE_REG_AVAILABLE;
// 	}
// }

// void rtc_stackcache_getfreereg_8bit(uint8_t regs*) {

// }
// void rtc_stackcache_getfreereg_16bit(uint8_t regs*) {

// }
// void rtc_stackcache_getfreereg_32bit(uint8_t regs*) {

// }

// void rtc_stackcache_pop_8bit(uint8_t regs*) {

// }
// void rtc_stackcache_pop_16bit(uint8_t regs*) {

// }
// void rtc_stackcache_pop_32bit(uint8_t regs*) {

// }

// void rtc_stackcache_push_8bit(uint8_t regs*) {

// }
// void rtc_stackcache_push_16bit(uint8_t regs*) {

// }
// void rtc_stackcache_push_32bit(uint8_t regs*) {

// }