#ifndef WKPF_CONFIG_H
#define WKPF_CONFIG_H

#include "wkcomm.h"
#include "types.h"

#define LOCATION_MAX_LENGTH 128
#define UUID_LENGTH 16

// Keep this list in sync with pynvc.py
#define WPKF_FEATURE_LIGHT_SENSOR         			0
#define WPKF_FEATURE_LIGHT_ACTUATOR       			1
#define WPKF_FEATURE_NUMERIC_CONTROLLER   			2
#define WPKF_FEATURE_NATIVE_THRESHOLD     			3
#define WPKF_FEATURE_TEMPERATURE_HUMIDITY_SENSOR 	4
#define WKPF_NUMBER_OF_FEATURES           			5

// Stores a part of the location in EEPROM, or returns WKPF_ERR_LOCATION_TOO_LONG if the string is too long.
extern uint8_t wkpf_config_set_part_of_location_string(char* src, uint8_t offset, uint8_t length);

// Retrieves a part the location from EEPROM and stores it in dest.
// If offset+length is longer than the location string, return the amount of bytes read.
extern uint8_t wkpf_config_get_part_of_location_string(char* dest, uint8_t offset, uint8_t length);

extern uint8_t wkpf_config_set_feature_enabled(uint8_t feature, bool enabled); // Needs reboot
extern bool wkpf_config_get_feature_enabled(uint8_t feature);

extern wkcomm_address_t wkpf_config_get_myid();
extern void wkpf_config_set_myid(wkcomm_address_t id);

extern wkcomm_address_t wkpf_config_get_gwid();
extern void wkpf_config_set_gwid(wkcomm_address_t id);

extern void wkpf_config_set_uuid(uint8_t* src);
extern void wkpf_config_get_uuid(uint8_t* dest);

#endif // WKPF_CONFIG_H
