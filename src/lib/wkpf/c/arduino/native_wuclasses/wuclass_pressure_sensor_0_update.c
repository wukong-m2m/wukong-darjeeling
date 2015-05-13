#include "debug.h"
#include "native_wuclasses.h"
#include <avr/io.h>

void wuclass_pressure_sensor_0_setup(wuobject_t *wuobject) {

}

void wuclass_pressure_sensor_0_update(wuobject_t *wuobject) {
    // Pieced together from IntelDemoLightSensorV1.java, Adc.java and native_avr.c

  // Adc.setPrescaler(Adc.DIV64);
  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

  // Adc.setReference(Adc.INTERNAL);
  ADMUX = (3 << 6) & 0xc0;              // set reference value

  // light_sensor_reading = Adc.getByte(Adc.CHANNEL0);
  // ADLAR = 1
  uint8_t channel = 2; // NOTE: Adc.CHANNEL0 means a value of 0 for the channel variable, but other ADC channels don't map 1-1. For instance channel 15 is selected by setting the channel variable to 39. See below for a list.
  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
  ADCSRB |= (channel & 0x20)>>2;

  // do conversion
  ADCSRA |= _BV(ADSC);                  // Start conversion
  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
  ADCSRA |= _BV(ADIF);                  // clear ADCIF

  // After this:
  // 0 <= ADCH <= 255
  // We want to recalibrate this to the range between the low and high values for the slider
  int16_t low;
  int16_t high;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PRESSURE_SENSOR_0_LOW, &low);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PRESSURE_SENSOR_0_HIGH, &high);

  int16_t output = (int16_t) ADCH;

  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PRESSURE_SENSOR_0_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Pressure Sensor 0): Sensed %d, low %d, output %d\n", ADCH, output);
}