#include <stdint.h>
#include "config.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[]) {
    javax_rtc_RTC_void_startBenchmarkMeasurement_Native();

    int16_t toFind = numbers[NUMNUMBERS-1] + 1;

    for (uint16_t i=0; i<1000; i++) {
        uint16_t low = 0;
        uint16_t high = NUMNUMBERS - 1;
        uint16_t mid;
        while (low <= high) {
            mid = (low + high) / 2;
            if (numbers[mid] < toFind) {
                low = mid + 1;
            } else if (numbers[mid] > toFind) {
                high = mid - 1;
            } else {
                break; // Found. Would return from here in a normal search, but for this benchmark we just want to try many numbers.
            }
        }
    }

    javax_rtc_RTC_void_stopBenchmarkMeasurement();
}
