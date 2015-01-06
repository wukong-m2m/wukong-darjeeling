#include <avr/eeprom.h>
#include "types.h"
#include "config.h"
#include "wkcomm.h"
#include "wkpf.h"
#include "wkpf_config.h"

#define WKPF_FEATURE_ARRAY_SIZE           (WKPF_NUMBER_OF_FEATURES/8 + 1)

static char EEMEM eeprom_location[LOCATION_MAX_LENGTH] = ""; // Currently can only handle locations that fit into a single message
static uint8_t EEMEM eeprom_wkpf_features[WKPF_FEATURE_ARRAY_SIZE];
static uint32_t EEMEM eeprom_mydid;
static uint32_t EEMEM eeprom_gwdid;

#define load_location_length() eeprom_read_byte((uint8_t*)&eeprom_location_length)
#define save_location_length(x) eeprom_update_byte((uint8_t*)&eeprom_location_length, (uint8_t)x)
#define load_location(dest, offset, length) eeprom_read_block((void*)dest, (const void*)(eeprom_location+offset), length)
#define save_location(src, offset, length) eeprom_update_block((const void*)src, (void*)(eeprom_location+offset), length)
#define feat_addr(feature) &eeprom_wkpf_features[feature / 8]
#define enable_feature(feature) eeprom_update_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) | (1<<(feature % 8)))
#define disable_feature(feature) eeprom_update_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) & ~(1<<(feature % 8)))
#define get_feature_enabled(feature) (eeprom_read_byte(feat_addr(feature)) & (1<<(feature % 8)))
#define load_mydid() eeprom_read_dword((wkcomm_address_t*)&eeprom_mydid)
#define save_mydid(x) eeprom_update_dword((wkcomm_address_t*)&eeprom_mydid, (wkcomm_address_t)x)
#define load_gwdid() eeprom_read_dword((wkcomm_address_t*)&eeprom_gwdid)
#define save_gwdid(x) eeprom_update_dword((wkcomm_address_t*)&eeprom_gwdid, (wkcomm_address_t)x)

// Stores a part of the location in EEPROM, or returns WKPF_ERR_LOCATION_TOO_LONG if the string is too long.
uint8_t wkpf_config_set_part_of_location_string(char* src, uint8_t offset, uint8_t length) {
  if (offset + length > LOCATION_MAX_LENGTH)
    return WKPF_ERR_LOCATION_TOO_LONG;

  save_location(src, offset, length);
  return WKPF_OK;
}

// Retrieves a part the location from EEPROM and stores it in dest.
uint8_t wkpf_config_get_part_of_location_string(char* dest, uint8_t offset, uint8_t length) {
  // If we're trying to read too many bytes, return the number of bytes that could be read.
  if (offset + length > LOCATION_MAX_LENGTH) {
    if (offset >= LOCATION_MAX_LENGTH)
      length = 0;
    else
      length = LOCATION_MAX_LENGTH - offset;
  }

  load_location(dest, offset, length);
  return length;
}

uint8_t wkpf_config_set_feature_enabled(uint8_t feature, bool enabled) {
  if (feature >= WKPF_NUMBER_OF_FEATURES)
    return WKPF_ERR_UNKNOWN_FEATURE;
  if (enabled)
    enable_feature(feature);
  else
    disable_feature(feature);
  return WKPF_OK;
}

bool wkpf_config_get_feature_enabled(uint8_t feature) {
  return feature < WKPF_NUMBER_OF_FEATURES
          && get_feature_enabled(feature) > 0;
}

wkcomm_address_t wkpf_config_get_mydid() {
  return load_mydid();
}

void wkpf_config_set_mydid(wkcomm_address_t did) {
  save_mydid(did);
}

wkcomm_address_t wkpf_config_get_gwdid() {
  return load_gwdid();
}

void wkpf_config_set_gwdid(wkcomm_address_t did) {
  save_gwdid(did);
}