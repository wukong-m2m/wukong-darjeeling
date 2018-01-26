#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"
#include "heat_sensor_data.h"

int16_t *ACal;
int32_t *QCal;
int16_t *stdCal;
int16_t *zscore;
int16_t z_max, z_min;

void fast_calibration(uint16_t frame_buffer[64], uint16_t frame_number);
uint16_t isqrt (int32_t x);
void zscoreCalculation(uint16_t frame_buffer[64]);

// Split into separate function to avoid the compiler just optimising away the whole test.
void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int32_t numbers[]) {
    rtc_startBenchmarkMeasurement_Native();

    uint16_t frame_buffer[64];

    for (uint16_t i=0; i<100; i++) {
        get_heat_sensor_data(frame_buffer, i);
        fast_calibration(frame_buffer, i);
    }
    get_heat_sensor_data(frame_buffer, 100);
    zscoreCalculation(frame_buffer);

    rtc_stopBenchmarkMeasurement();
}

#define rounding_int_division(dividend, divisor) (((dividend) + ((divisor) / 2)) / (divisor))

void fast_calibration(uint16_t frame_buffer[64], uint16_t frame_number) {
    for(uint16_t i=0; i<64; i++) {
        uint16_t previous_ACal = ACal[i];
        ACal[i] += rounding_int_division((int32_t)frame_buffer[i] - ACal[i], frame_number+1);
        QCal[i] += ((int32_t)frame_buffer[i] - previous_ACal) * ((int32_t)frame_buffer[i] - ACal[i]);
    }
    for(uint16_t i=0; i<64; i++) {
        stdCal[i] = isqrt(QCal[i]/(frame_number+1));
    }
}

// http://www.cc.utah.edu/~nahaj/factoring/isqrt.c.html
uint16_t isqrt (int32_t x) {
    uint32_t   squaredbit, remainder, root;

    if (x<1) return 0;

    /* Load the binary constant 01 00 00 ... 00, where the number
    * of zero bits to the right of the single one bit
    * is even, and the one bit is as far left as is consistant
    * with that condition.)
    */
    squaredbit  = (uint32_t) ((((uint32_t) ~0L) >> 1) & 
                        ~(((uint32_t) ~0L) >> 2));
    /* This portable load replaces the loop that used to be 
    * here, and was donated by  legalize@xmission.com 
    */

    /* Form bits of the answer. */
    remainder = x;  root = 0;
    while (squaredbit > 0) {
        if (remainder >= (squaredbit | root)) {
            remainder -= (squaredbit | root);
            root >>= 1; root |= squaredbit;
        } else {
            root >>= 1;
        }
        squaredbit >>= 2; 
    }

    return root;
}

void zscoreCalculation(uint16_t frame_buffer[64]) {
    int32_t tempMax = -300000;
    int32_t tempMin = 300000;

    for(int i=0; i<64; i++) {
        int16_t score = 100 * ((int32_t)frame_buffer[i] - (int32_t)ACal[i]) / stdCal[i];

        zscore[i] = score;

        if(score > tempMax) {
            tempMax = score;
        }

        if(score < tempMin) {
            tempMin = score;
        }
    }

    z_max = tempMax;
    z_min = tempMin;
}




