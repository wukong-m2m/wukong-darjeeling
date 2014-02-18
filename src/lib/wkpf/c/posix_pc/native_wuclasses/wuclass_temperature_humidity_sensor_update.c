#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_temperature_humidity_sensor_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "temperature_sensor");
	posix_property_get(wuobject, "humidity_sensor");
}

void wuclass_temperature_humidity_sensor_update(wuobject_t *wuobject) {
	int temp_value = posix_property_get(wuobject, "temperature_sensor");
	printf("WKPFUPDATE(TemperatureHumiditySensor): Sensed temperature value: %dC\n", temp_value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_HUMIDITY_SENSOR_CURRENT_VALUE_TEMPERATURE, (int16_t)temp_value);

	int hum_value = posix_property_get(wuobject, "humidity_sensor");
	printf("WKPFUPDATE(TemperatureHumiditySensor): Sensed humidity value: %dC\n", hum_value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_HUMIDITY_SENSOR_CURRENT_VALUE_HUMIDITY, (int16_t)hum_value);
}
