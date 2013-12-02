#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>

void wuclass_numeric_controller_setup(wuobject_t *wuobject) {}

void wuclass_numeric_controller_update(wuobject_t *wuobject) {
  int max, min = 0;
  int device_max, device_min = 0;
  int result = 0;
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_MAX_OUTPUT_VALUE, &max);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_MIN_OUTPUT_VALUE, &min);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_MAX_DEVICE_VALUE, &device_max);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_MIN_DEVICE_VALUE, &device_min);

  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

  // Adc.setReference(Adc.INTERNAL);
  ADMUX = (3 << 6) & 0xc0;              // set reference value

  // light_sensor_reading = Adc.getByte(Adc.CHANNEL0);
  // ADLAR = 1
  uint8_t channel  = 0; // NOTE: Adc.CHANNEL0 means a value of 0 for the channel variable, but other ADC channels don't map 1-1. For instance channel 15 is selected by setting the channel variable to 39. See Adc.Java for a list.
  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
  ADCSRB |= (channel & 0x20)>>2;

  // do conversion
  ADCSRA |= _BV(ADSC);                  // Start conversion
  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
  ADCSRA |= _BV(ADIF);                  // clear ADCIF
  result = ADCH;
  result = (double)result * ((double)max-(double)min) / ((double)device_max-(double)device_min);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Numeric Sensor): Sensed value: %d, converted value:%d\n", ADCH, result);
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_OUTPUT, ADCH);
  
  //DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NumericController): NOP\n");
}
