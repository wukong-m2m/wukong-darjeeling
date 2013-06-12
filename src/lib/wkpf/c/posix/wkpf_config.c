#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "wkcomm.h"
#include "wkpf.h"
#include "wkpf_config.h"

typedef struct features_t {
	bool feature_enabled[WKPF_NUMBER_OF_FEATURES];
	char location[LOCATION_MAX_LENGTH];
	wkcomm_address_t master_node_id;
	wkcomm_address_t gid;
} features_t;

features_t features;
bool features_loaded = false;

#define CONFIG_FILE_NAME "config.txt"
#define CONFIG_FILE_LOCATION_STRING "Location (in raw bytes on the next line):\n"
#define CONFIG_FILE_MASTER_ID_STRING "Master: %d\n"
#define CONFIG_FILE_GID_STRING "Gid: %d\n"
#define CONFIG_FILE_ENABLED_FEATURE_STRING "Feature: %d %d\n"

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

void save_features_data() {
	FILE *fp = fopen(CONFIG_FILE_NAME, "w");
	if (fp== NULL) {
		printf("Can't open %s for writing, aborting...\n", CONFIG_FILE_NAME);
		abort();
	}		
	fprintf(fp, CONFIG_FILE_MASTER_ID_STRING, features.master_node_id);
	fprintf(fp, CONFIG_FILE_GID_STRING, features.gid);
	fprintf(fp, CONFIG_FILE_LOCATION_STRING);
	for (int i=0; i<LOCATION_MAX_LENGTH; i++)
		fputc(features.location[i], fp);
	fputc('\n', fp);
	for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
		fprintf(fp, CONFIG_FILE_ENABLED_FEATURE_STRING, i, features.feature_enabled[i]);
	fclose(fp);
}

void load_features_data() {
	FILE *fp = fopen(CONFIG_FILE_NAME, "r");
	if (fp== NULL) {
		// No config file found, create default features.
		for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
			features.feature_enabled[i] = true;
		features.master_node_id = 0;
		memset(features.location, 0, LOCATION_MAX_LENGTH);
	} else {
		for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
			features.feature_enabled[i] = false;

		char *line = NULL;
		size_t len = 0;
		ssize_t read;

        while ((read = getline(&line, &len, fp)) != -1) {
        	if (prefix("Master", line)) {
				int master_node_id;
				if (!sscanf(line, CONFIG_FILE_MASTER_ID_STRING, &master_node_id)) {
					printf("Master node id in %s not in expected format, aborting...\n", CONFIG_FILE_NAME);
					abort();
				}
				if (master_node_id > 255) {
					printf("Master node id in %s too large (%d), aborting...\n", CONFIG_FILE_NAME, master_node_id);
					abort();
				}
				features.master_node_id = master_node_id;
				DEBUG_LOG(DBG_WKPF, "CONFIG: master id = %d\n", features.master_node_id);
        	} else if (prefix("Gid", line)) {
				int gid;
				if (!sscanf(line, CONFIG_FILE_GID_STRING, &gid)) {
					printf("Gid in %s not in expected format, aborting...\n", CONFIG_FILE_NAME);
					abort();
				}
				if (gid > 255) {
					printf("Gid in %s too large (%d), aborting...\n", CONFIG_FILE_NAME, gid);
					abort();
				}
				features.gid = gid;
				DEBUG_LOG(DBG_WKPF, "CONFIG: gid = %d\n", features.gid);
			} else if (prefix("Feature", line)) {
				int feature;
				int is_enabled;
				if(!sscanf(line, CONFIG_FILE_ENABLED_FEATURE_STRING, &feature, &is_enabled) == 1) {
					printf("Feature in %s not in expected format, aborting...\n", CONFIG_FILE_NAME);
					abort();
				}
				features.feature_enabled[feature] = is_enabled;
        		DEBUG_LOG(DBG_WKPF, "CONFIG: feature %d is %s\n", feature, features.feature_enabled[feature] ? "enabled" : "disabled");
			} else if (prefix("Location", line)) {
				for (int i=0; i<LOCATION_MAX_LENGTH; i++)
					features.location[i] = fgetc(fp);
				fgetc(fp); // read \n
        	} else
        		DEBUG_LOG(DBG_WKPF, "CONFIG: ignoring line '%s'\n", line);
        }

		if (line)
			free(line);
		fclose(fp);
	}
	save_features_data();
	features_loaded = true;
}


uint8_t wkpf_config_set_part_of_location_string(char* src, uint8_t offset, uint8_t length) {
	if (!features_loaded)
		load_features_data();

	if (offset + length > LOCATION_MAX_LENGTH)
		return WKPF_ERR_LOCATION_TOO_LONG;
	memcpy(features.location+offset, src, length);
	DEBUG_LOG(DBG_WKPF, "CONFIG: set part of location string. offset:%d length%d\n", offset, length);

	save_features_data();
	return WKPF_OK;
}

uint8_t wkpf_config_get_part_of_location_string(char* dest, uint8_t offset, uint8_t length) {
	if (!features_loaded)
		load_features_data();

	if (offset + length > LOCATION_MAX_LENGTH) {
		if (offset >= LOCATION_MAX_LENGTH)
			length = 0;
		else
			length = LOCATION_MAX_LENGTH - offset;
	}
	memcpy(dest, features.location+offset, length);
	DEBUG_LOG(DBG_WKPF, "CONFIG: get part of location string. offset:%d length%d\n", offset, length);

	return length;
}

uint8_t wkpf_config_set_feature_enabled(uint8_t feature, bool enabled) {
	if (!features_loaded)
		load_features_data();

	if (feature >= WKPF_NUMBER_OF_FEATURES)
		return WKPF_ERR_UNKNOWN_FEATURE;
	features.feature_enabled[feature] = enabled;

	save_features_data();
	return WKPF_OK;
}

bool wkpf_config_get_feature_enabled(uint8_t feature) {
	if (!features_loaded)
		load_features_data();

	return feature < WKPF_NUMBER_OF_FEATURES
			&& features.feature_enabled[feature];
}

wkcomm_address_t wkpf_config_get_master_node_id() {
	if (!features_loaded)
		load_features_data();

	return features.master_node_id;
}

void wkpf_config_set_master_node_id(wkcomm_address_t node_id) {
	if (!features_loaded)
		load_features_data();

	features.master_node_id = node_id;
	
	save_features_data();
}

wkcomm_address_t wkpf_config_get_gid() {
	if (!features_loaded)
		load_features_data();

	return features.gid;
}

void wkpf_config_set_gid(wkcomm_address_t gid) {
	if (!features_loaded)
		load_features_data();

	features.gid = gid;
	
	save_features_data();
}



