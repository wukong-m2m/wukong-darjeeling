#include "config.h"
#include "djtimer.h"
#include "execution.h"
#include "rtc_measure.h"

void rtc_startBenchmarkMeasurement_Native() { 
    avroraSetTimerNumber(AVRORA_BENCH_NATIVE_TIMER); 
    avroraProfilerStartCounting(); 
    avroraStartTimer(); 
} 

// We save X here, so we don't need to do so in the AOT compiled code.
// We could also write this whole method in assembly, and actually the
// push/pop isn't even necessary at the moment since the generated code
// doesn't touch X, but this is easier than handwriting the whole thing
// and safer in case later avr-gcc does decide to use X for some reason.
void rtc_startBenchmarkMeasurement_AOT() {
    asm volatile("   push XH" "\n\r"
                 "   push XL" "\n\r"
                 ::);
    avroraSetTimerNumber(AVRORA_BENCH_AOT_TIMER);
    avroraProfilerStartCounting();
    avroraStartTimer();
    asm volatile("   pop XL" "\n\r"
                 "   pop XH" "\n\r"
                 ::);
}
void rtc_stopBenchmarkMeasurement() {
    asm volatile("   push XH" "\n\r"
                 "   push XL" "\n\r"
                 ::);
    avroraStopTimer();
    avroraProfilerStopCounting();
    asm volatile("   pop XL" "\n\r"
                 "   pop XH" "\n\r"
                 ::);
}