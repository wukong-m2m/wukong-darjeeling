#include "config.h"
#include "djtimer.h"
#include "execution.h"
#include "rtc_measure.h"

void rtc_startBenchmarkMeasurement_Native() { 
    avroraSetTimerNumber(AVRORA_BENCH_NATIVE_TIMER); 
    avroraProfilerStartCounting(); 
    avroraStartTimer(); 
} 
void rtc_startBenchmarkMeasurement_AOT() {
    avroraSetTimerNumber(AVRORA_BENCH_AOT_TIMER);
    avroraProfilerStartCounting();
    avroraStartTimer();
}
void rtc_stopBenchmarkMeasurement() {
    avroraStopTimer();
    avroraProfilerStopCounting();
}