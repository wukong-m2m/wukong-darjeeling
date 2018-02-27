#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"
#include <stdbool.h>
#include "heap.h"
#include "motetrack.h"
#include "Point.h"

#include "motetrack.h"

Point __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
    rtc_startBenchmarkMeasurement_Native();

    Point p = estLocAndSend();

    rtc_stopBenchmarkMeasurement();

    return p;
}

void MoteTrackMain() {
    motetrack_init_benchmark();
	Point p = rtcbenchmark_measure_native_performance();
    avroraPrintInt16(p.x);
    avroraPrintInt16(p.y);
    avroraPrintInt16(p.z);	
}
