#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"
#include <stdbool.h>

#include "motetrack.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
    rtc_startBenchmarkMeasurement_Native();

    Point p;

    // for (uint8_t i=5; i-->0; ) {
        p = estLocAndSend();

        // avroraPrintStr("MoteTrack estLocAndSend:");
        avroraPrintInt16(p.x);
        avroraPrintInt16(p.y);
        avroraPrintInt16(p.z);
    // }
    rtc_stopBenchmarkMeasurement();
}