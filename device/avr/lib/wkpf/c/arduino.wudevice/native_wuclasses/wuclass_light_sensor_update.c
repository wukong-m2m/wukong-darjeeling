#include "debug.h"
#include "native_wuclasses.h"
#include <avr/io.h>

void wuclass_light_sensor_setup(wuobject_t *wuobject) {}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
  // Pieced together from IntelDemoLightSensorV1.java, Adc.java and native_avr.c

  // Adc.setPrescaler(Adc.DIV64);
  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

  // Adc.setReference(Adc.INTERNAL);
  ADMUX = (3 << 6) & 0xc0;              // set reference value

  // light_sensor_reading = Adc.getByte(Adc.CHANNEL0);
  // ADLAR = 1
  uint8_t channel  = 0; // NOTE: Adc.CHANNEL0 means a value of 0 for the channel variable, but other ADC channels don't map 1-1. For instance channel 15 is selected by setting the channel variable to 39. See below for a list.
  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
  ADCSRB |= (channel & 0x20)>>2;

  // do conversion
  ADCSRA |= _BV(ADSC);                  // Start conversion
  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
  ADCSRA |= _BV(ADIF);                  // clear ADCIF
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(LightSensor): Sensed light value: %d\n", ADCH);
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, ADCH);
}

