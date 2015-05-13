#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void wuclass_buzzer_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    //Configure IO6 shield pin as a PWM output.
    if( access( "/sys/class/gpio/gpio20/value", F_OK ) == -1 ) {
      system("echo -n 20 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio21/value", F_OK ) == -1 ) {
      system("echo -n 21 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio68/value", F_OK ) == -1 ) {
      system("echo -n 68 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/pwm/pwmchip0/pwm5/enable", F_OK ) == -1 ) {
      system("echo 5 > /sys/class/pwm/pwmchip0/export");
    }
    system("echo in > /sys/class/gpio/gpio20/direction");
    system("echo -n strong > /sys/class/gpio/gpio21/drive");
    system("echo 1 > /sys/class/gpio/gpio68/value");
    system("echo 1 > /sys/class/pwm/pwmchip0/pwm5/enable");
    #endif
    #ifdef INTEL_EDISON
    //Configure IO6 shield pin as a PWM output.
    if( access( "/sys/class/gpio/gpio182/value", F_OK ) == -1 ) {
      system("echo -n 182 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio254/value", F_OK ) == -1 ) {
      system("echo -n 254 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio222/value", F_OK ) == -1 ) {
      system("echo -n 222 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/gpio/gpio214/value", F_OK ) == -1 ) {
      system("echo -n 214 > /sys/class/gpio/export");
    }
    if( access( "/sys/class/pwm/pwmchip0/pwm2/enable", F_OK ) == -1 ) {
      system("echo 2 > /sys/class/pwm/pwmchip0/export");
    }
    system("echo low > /sys/class/gpio/gpio214/direction");
    system("echo low > /sys/class/gpio/gpio254/direction");
    system("echo in > /sys/class/gpio/gpio222/direction");
    system("echo mode1 > /sys/kernel/debug/gpio_debug/gpio182/current_pinmux");
    system("echo high > /sys/class/gpio/gpio214/direction");
    system("echo 1 > /sys/class/pwm/pwmchip0/pwm2/enable");
    #endif
}

void wuclass_buzzer_update(wuobject_t *wuobject) {
    bool onOff;
    int16_t freq;
    int16_t dutyCycle;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_BUZZER_ON_OFF, &onOff);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_BUZZER_FREQ, &freq);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_BUZZER_DUTY_CYCLE, &dutyCycle);

    int32_t nsecPeriod = 1000000000/freq;
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): nsec %d\n", nsecPeriod);
    int32_t dutyCycleDuration = (dutyCycle/100.0) * nsecPeriod;
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): dutyCycleDuration %d\n", dutyCycleDuration);
      
    #ifdef INTEL_GALILEO_GEN1
    #endif
    #ifdef INTEL_GALILEO_GEN2
    char bufPeriod[128];
    char bufDutyCycle[128];
    sprintf(bufPeriod, "echo %d > /sys/class/pwm/pwmchip0/device/pwm_period", nsecPeriod);
    system(bufPeriod);
    sprintf(bufDutyCycle, "echo %d > /sys/class/pwm/pwmchip0/pwm5/duty_cycle", dutyCycleDuration);
    system(bufDutyCycle);
    if (onOff){
      system("echo out > /sys/class/gpio/gpio20/direction");
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): on\n");
    }else{
      system("echo in > /sys/class/gpio/gpio20/direction");
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): off\n");
    }
    #endif
    #ifdef INTEL_EDISON
    char bufPeriod[256];
    char bufDutyCycle[256];
    sprintf(bufPeriod, "echo %d > /sys/class/pwm/pwmchip0/pwm2/period", nsecPeriod);
    system(bufPeriod);
    sprintf(bufDutyCycle, "echo %d > /sys/class/pwm/pwmchip0/pwm2/duty_cycle", dutyCycleDuration);
    system(bufDutyCycle);
    if(onOff){
      system("echo high > /sys/class/gpio/gpio214/direction");
      system("echo high > /sys/class/gpio/gpio254/direction");
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): on\n");
    }else{
      system("echo high > /sys/class/gpio/gpio214/direction");   
      system("echo low > /sys/class/gpio/gpio254/direction");
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): off\n");
    }
    #endif
}
