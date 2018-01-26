#include <stdint.h>
#include "config.h"
#include "heap.h"


extern uint16_t *ACal;
extern uint32_t *QCal;
extern uint16_t *stdCal;
extern uint16_t *zscore;
extern int16_t z_max, z_min;

void __attribute__((noinline)) rtcbenchmark_measure_native_performance();

void javax_rtcbench_RTCBenchmark_void_test_native() {
    ACal = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    QCal = dj_mem_checked_alloc(64*sizeof(uint32_t), CHUNKID_RTCNATIVETESTDATA);
    stdCal = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    zscore = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);

    for (uint16_t i=0; i<64; i++) {
        ACal[i] = QCal[i] = stdCal[i] = 0;
    }

    rtcbenchmark_measure_native_performance();

    avroraRTCRuntimeBeep(0);
    for(int i=0; i<64; i++) {
        avroraPrintInt16(stdCal[i]);
    }
    avroraRTCRuntimeBeep(1);
    for(int i=0; i<64; i++) {
        avroraPrintInt16(zscore[i]);
    }
    avroraRTCRuntimeBeep(2);
    avroraPrintInt16(z_min);
    avroraPrintInt16(z_max);
    avroraRTCRuntimeBeep(3);
}
