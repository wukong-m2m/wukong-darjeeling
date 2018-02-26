#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"
#include <stdbool.h>

#include "motetrack.h"

Point __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
    rtc_startBenchmarkMeasurement_Native();

    Point p = estLocAndSend();

    rtc_stopBenchmarkMeasurement();

    return p;
}
