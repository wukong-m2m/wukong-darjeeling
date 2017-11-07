#include "config.h"

void base_init(void) {
	// Only start counting cycles when the benchmark starts. Turn off profiler during init.
	avroraProfilerStopCounting();
	avroraProfilerReset();
}