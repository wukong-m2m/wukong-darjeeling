#include <stdint.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "heap.h"

uint8_t __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[], uint8_t output[]);

static const int16_t ecg_samples[] PROGMEM = { -489, -485, -483, -482, -463, -452, -450, -469, -469, -447, -441, -454, -464, -475, -489, -474, -448, -453, -453, -455, -465, -452, -451, -422, -427, -447, -440, -432, -400, -393, -393, -418, -477, -504, -477, -453, -469, -509, -495, -462, -460, -440, -429, -429, -455, -449, -427, -401, -413, -456, -438, -431, -431, -443, -463, -456, -438, -442, -454, -453, -458, -463, -471, -447, -427, -432, -422, -425, -437, -423, -407, -402, -409, -461, -466, -435, -444, -450, -427, -417, -429, -423, -417, -416, -421, -431, -423, -408, -386, -355, -401, -380, -324, -385, -399, -389, -376, -388, -425, -384, -334, -323, -336, -324, -314, -332, -302, -292, -305, -292, -282, -284, -290, -321, -312, -277, -272, -274, -308, -270, -235, -249, -250, -277, -240, -183, -177, -179, -186, -155, -156, -180, -194, -197, -188, -204, -184, -184, -163, -151, -158, -134, -144, -117, -105, -101, -97, -94, -76, -67, -28, -39, -57, -74, -51, -23, -33, -43, -110, -129, -80, -14, -68, -103, -60, -74, -73, -40, 2, 23, 11, -33, -47, -26, 6, -26, -44, -42, -65, -68, -79, -37, -22, -22, 0, -5, -25, -35, -36, -40, -45, -55, -44, -62, -82, -92, -92, -83, -77, -53, -69, -74, -71, -79, -91, -98, -80, -53, -58, -57, -66, -127, -109, -109, -121, -95, -129, -161, -159, -151, -165, -152, -149, -155, -148, -159, -168, -166, -166, -189, -160, -158, -184, -190, -172, -150, -195, -197, -180, -201, -213, -219, -232, -221, -203, -198, -223, -217, -172, -167, -204, -264, -240, -211, -223, -241 };

void javax_rtcbench_RTCBenchmark_void_test_native() {
    // Test with data from paper:
    // di=0,di =+1,di =−1,di =+255and di = −255 are encoded as 00, 010|1, 010|0, 111110|11111111 and 111110|00000000
    // uint16_t NUMNUMBERS = 5;
    // int16_t numbers[] = { 0, 1, 0, 255, 0 };

    // ECG dataset:
    uint16_t NUMNUMBERS = 256;
    int16_t *numbers = dj_mem_checked_alloc(NUMNUMBERS*sizeof(int16_t), CHUNKID_RTCNATIVETESTDATA);
    memcpy_P(numbers, ecg_samples, sizeof(int16_t)*NUMNUMBERS); 

    uint8_t *output = dj_mem_checked_alloc(4*NUMNUMBERS*sizeof(int8_t), CHUNKID_RTCNATIVETESTDATA);
    for (uint16_t i=0; i<4*NUMNUMBERS*sizeof(int8_t); i++) {
        output[i] = 0;
    }

    uint8_t encoded_length = rtcbenchmark_measure_native_performance(NUMNUMBERS, numbers, output);
    // for (uint8_t i=0; i<encoded_length; i++) {
    //     avroraPrintInt16(output[i] >> 7 & 1);
    //     avroraPrintInt16(output[i] >> 6 & 1);
    //     avroraPrintInt16(output[i] >> 5 & 1);
    //     avroraPrintInt16(output[i] >> 4 & 1);
    //     avroraPrintInt16(output[i] >> 3 & 1);
    //     avroraPrintInt16(output[i] >> 2 & 1);
    //     avroraPrintInt16(output[i] >> 1 & 1);
    //     avroraPrintInt16(output[i] >> 0 & 1);
    // }   

    avroraRTCRuntimeBeep(0);

    for (uint8_t i=0; i<encoded_length; i++) {
            avroraPrintInt16(output[i]);
    }
}
