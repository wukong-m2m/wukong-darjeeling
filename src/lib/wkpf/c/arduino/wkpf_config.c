#include <avr/eeprom.h>
#include "types.h"
#include "wkcomm.h"
#include "wkpf.h"
#include "wkpf_config.h"

#define WKPF_FEATURE_ARRAY_SIZE           (WKPF_NUMBER_OF_FEATURES/8 + 1)

static char EEMEM eeprom_location[LOCATION_MAX_LENGTH] = ""; // Currently can only handle locations that fit into a single message
static uint8_t EEMEM eeprom_wkpf_features[WKPF_FEATURE_ARRAY_SIZE];
static uint8_t EEMEM eeprom_master_address;
static uint8_t EEMEM eeprom_gid;

#define load_location_length() eeprom_read_byte((uint8_t*)&eeprom_location_length)
#define save_location_length(x) eeprom_write_byte((uint8_t*)&eeprom_location_length, (uint8_t)x)
#define load_location(dest, offset, length) eeprom_read_block((void*)dest, (const void*)(eeprom_location+offset), length)
#define save_location(src, offset, length) eeprom_write_block((const void*)src, (void*)(eeprom_location+offset), length)
#define feat_addr(feature) &eeprom_wkpf_features[feature / 8]
#define enable_feature(feature) eeprom_write_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) | (1<<(feature % 8)))
#define disable_feature(feature) eeprom_write_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) & ~(1<<(feature % 8)))
#define get_feature_enabled(feature) (eeprom_read_byte(feat_addr(feature)) & (1<<(feature % 8)))
#define load_master_node_id() eeprom_read_word((uint16_t*)&eeprom_master_address)
#define save_master_node_id(x) eeprom_write_word((uint16_t*)&eeprom_master_address, (uint16_t)x)
#define load_gid() eeprom_read_word((uint16_t*)&eeprom_gid)
#define save_gid(x) eeprom_write_word((uint16_t*)&eeprom_gid, (uint16_t)x)

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

wkcomm_address_t wkpf_config_get_master_node_id() {
  return (wkcomm_address_t)load_master_node_id();
}

void wkpf_config_set_master_node_id(wkcomm_address_t node_id) {
  if (sizeof(wkcomm_address_t) != 2) {
    // Bit crude: just enter an endless loop when wkcomm_address_t is changed to a bigger datatype because that means we need to change this function as well.
    // This way uploading new code won't work (as long as we call this when receiving NVMCOMM_CMD_REPRG_OPEN) until this function is fixed.
    // The alternative would be hard to find bugs when one byte of the node_id is lost.
    while(1);
  }
  save_master_node_id(node_id);
}

wkcomm_address_t wkpf_config_get_gid() {
  return load_gid();
}

void wkpf_config_set_gid(wkcomm_address_t gid) {
  save_gid(gid);
}
