#include "debug.h"
#include "native_wuclasses.h"
#include <avr/io.h>
#include "avr.h"
#include "temperature_humidity_sensor/SHT1x.h"

void wuclass_temperature_humidity_sensor_setup(wuobject_t *wuobject) {}

void wuclass_temperature_humidity_sensor_update(wuobject_t *wuobject) {
    sbi(TCCR0B, CS01);
    sbi(TCCR0B, CS00);	
    sbi(TIMSK0, TOIE0);    
    SHT1x(5,6);
    
    int temperature, temperatureRaw, humidityRaw, humidity;
    temperatureRaw=readTemperatureRaw();
    temperatureRaw=temperatureRaw-4000;
    temperature=temperatureRaw/100;
    
    humidityRaw=readHumidity();
    humidity=(temperature*(100000+800*humidityRaw) + (405000+28*humidityRaw)*humidityRaw - 4*10000000)/10000000 - 11;
    
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(TemperatureHumiditySensor): Sensed temperature value: %d.%dC\n", temperature, temperatureRaw%100);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(TemperatureHumiditySensor): Sensed humidity value: %d %% \n", humidity);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_HUMIDITY_SENSOR_CURRENT_VALUE_TEMPERATURE, temperature);
    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_HUMIDITY_SENSOR_CURRENT_VALUE_HUMIDITY, humidity);
}
