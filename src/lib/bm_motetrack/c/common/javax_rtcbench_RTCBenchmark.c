#include <stdint.h>
#include "config.h"
#include "heap.h"

#include "motetrack.h"
#include "Point.h"

Point __attribute__((noinline)) rtcbenchmark_measure_native_performance();

void javax_rtcbench_RTCBenchmark_void_test_native() {
    motetrack_init_benchmark();
	Point p = rtcbenchmark_measure_native_performance();
    avroraPrintInt16(p.x);
    avroraPrintInt16(p.y);
    avroraPrintInt16(p.z);
}
