#include "types.h"
#include "wkcomm.h"
#include "wkpf.h"

// Empty dummy implementations for now
uint8_t wkpf_config_set_part_of_location_string(char* src, uint8_t offset, uint8_t length) {
	return WKPF_OK;
}

uint8_t wkpf_config_get_part_of_location_string(char* dest, uint8_t offset, uint8_t length) {
	return WKPF_OK;
}

uint8_t wkpf_config_set_feature_enabled(uint8_t feature, bool enabled) {
	return WKPF_OK;
}

bool wkpf_config_get_feature_enabled(uint8_t feature) {
	return true;
}
