#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "heap.h"
#include "heat_sensor_data.h"
#include "heat_calib.h"
#include "heat_detect.h"
#include "rtc_measure.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t *frame_buffer, uint8_t *color, uint8_t *rColor, int32_t *largestSubset, int32_t *testset, int32_t *result) {
    rtc_startBenchmarkMeasurement_Native();

    for (uint16_t i=0; i<25; i++) {
        get_heat_sensor_data(frame_buffer, 101+i); // detection frames start after 100 calibration frames and 1 check frame.
        heat_detect(frame_buffer, color, rColor, largestSubset, testset, result);
    }

    rtc_stopBenchmarkMeasurement();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
    ACal = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    QCal = dj_mem_checked_alloc(64*sizeof(uint32_t), CHUNKID_RTCNATIVETESTDATA);
    stdCal = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    zscore = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    zscoreWeight = dj_mem_checked_alloc(64*sizeof(bool), CHUNKID_RTCNATIVETESTDATA);

    for (uint16_t i=0; i<64; i++) {
        ACal[i] = QCal[i] = stdCal[i] = 0;
    }
    heat_calib();

    uint16_t *frame_buffer = dj_mem_checked_alloc(64*sizeof(uint16_t), CHUNKID_RTCNATIVETESTDATA);
    uint8_t *color         = dj_mem_checked_alloc(64*sizeof(uint8_t), CHUNKID_RTCNATIVETESTDATA);
    uint8_t *rColor        = dj_mem_checked_alloc(64*sizeof(uint8_t), CHUNKID_RTCNATIVETESTDATA);
    int32_t *largestSubset = dj_mem_checked_alloc(64*sizeof(int32_t), CHUNKID_RTCNATIVETESTDATA);
    int32_t *testset       = dj_mem_checked_alloc(64*sizeof(int32_t), CHUNKID_RTCNATIVETESTDATA);
    int32_t *result       = dj_mem_checked_alloc(64*sizeof(int32_t), CHUNKID_RTCNATIVETESTDATA);

    rtcbenchmark_measure_native_performance(frame_buffer, color, rColor, largestSubset, testset, result);

    avroraRTCRuntimeBeep(0);
    avroraPrintInt16(x_weight_coordinate);
    avroraPrintInt16(y_weight_coordinate);
    avroraPrintInt16(xh_weight_coordinate);
    avroraPrintInt16(yh_weight_coordinate);
    avroraRTCRuntimeBeep(1);
    avroraPrintHex32(yellowGroupH);
    avroraPrintHex32(yellowGroupL);
    avroraPrintHex32(orangeGroupH);
    avroraPrintHex32(orangeGroupL);
    avroraPrintHex32(redGroupH);
    avroraPrintHex32(redGroupL);
    avroraRTCRuntimeBeep(2);
}
