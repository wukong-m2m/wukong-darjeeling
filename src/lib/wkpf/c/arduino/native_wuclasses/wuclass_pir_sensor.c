#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>

#ifdef ENABLE_WUCLASS_PIR_SENSOR

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

  ADMUX = (3 << 6) & 0xc0;              // set reference value

  uint8_t channel  = 1; // channel 1 for pin A1
  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
  ADCSRB |= (channel & 0x20)>>2;

  ADCSRA |= _BV(ADSC);                  // Start conversion
  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
  ADCSRA |= _BV(ADIF);                  // clear ADCIF
  bool showup = false;
  if(ADCH == 255) {
    showup = true;
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PirSensor): Sensed pir value: true\n");
  }else
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PirSensor): Sensed pir value: false\n");
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, showup);
}

#endif
