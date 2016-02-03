#ifdef MRAA_LIBRARY

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include <mraa.h>

mraa_pwm_context pwm;

void wuclass_buzzer_setup(wuobject_t *wuobject) {
    pwm = mraa_pwm_init(6);
}

void wuclass_buzzer_update(wuobject_t *wuobject) {
    bool onOff;
    int16_t freq = 500;
    int16_t dutyCycle = 50;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_BUZZER_ON_OFF, &onOff);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_BUZZER_FREQ, &freq);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_BUZZER_DUTY_CYCLE, &dutyCycle);

    int32_t nsecPeriod = 1000000000/freq;
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): nsec %d\n", nsecPeriod);
    int32_t dutyCycleDuration = (dutyCycle/100.0) * nsecPeriod;
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): dutyCycleDuration %d\n", dutyCycleDuration);
      
    int32_t usecPeriod = nsecPeriod * 1000;
    mraa_pwm_period_us(pwm, usecPeriod);
    float dutyCycleValue = dutyCycle / 100.0;
    mraa_pwm_write(pwm, dutyCycleValue);
    if(onOff){
      mraa_pwm_enable(pwm, 1);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): on\n");
    }else{
      mraa_pwm_enable(pwm, 0);
      DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Buzzer): off\n");
    }
}

#endif