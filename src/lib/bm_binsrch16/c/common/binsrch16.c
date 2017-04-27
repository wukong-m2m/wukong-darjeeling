#include <stdint.h>
#include "config.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[]) {
    javax_rtc_RTC_void_startBenchmarkMeasurement_Native();

    int16_t toFind = numbers[NUMNUMBERS-1] + 1;

    uint16_t mid=0;
    for (uint16_t i=0; i<1000; i++) {
        uint16_t low = 0;
        uint16_t high = NUMNUMBERS - 1;
        while (low <= high) {
            mid = ((uint16_t)(low + high)) >> 1;
            int16_t number_mid;
            if ((number_mid=numbers[mid]) < toFind) {
                low = mid + 1;
            } else if (numbers[mid] > toFind) {
                high = mid - 1;
            } else {
                break; // Found. Would return from here in a normal search, but for this benchmark we just want to try many numbers.
            }
        }
    }

    javax_rtc_RTC_void_stopBenchmarkMeasurement();
    numbers[0]=mid; // This is just here to prevent proguard from optimising away the whole method
}
