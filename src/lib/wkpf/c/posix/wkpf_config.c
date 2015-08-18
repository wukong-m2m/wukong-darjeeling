#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "wkcomm.h"
#include "wkpf.h"
#include "wkpf_config.h"
#include "posix_utils.h"

typedef struct features_t {
	bool feature_enabled[WKPF_NUMBER_OF_FEATURES];
	char location[LOCATION_MAX_LENGTH];
	uint8_t uuid[UUID_LENGTH];
	wkcomm_address_t myid;
	wkcomm_address_t gwid;
} features_t;

features_t features;
bool features_loaded = false;

#define CONFIG_FILE_LOCATION_STRING "Location (in raw bytes on the next line):\n"
#define CONFIG_FILE_UUID_STRING "UUID: \n"
#define CONFIG_FILE_MYID_STRING "MyID: %d\n"
#define CONFIG_FILE_GWID_STRING "GwID: %d\n"
#define CONFIG_FILE_ENABLED_FEATURE_STRING "Feature: %d %d\n"

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

void save_features_data() {
	FILE *fp = fopen(posix_config_filename, "w");
	if (fp== NULL) {
		printf("Can't open %s for writing, aborting...\n", posix_config_filename);
		abort();
	}
	fprintf(fp, CONFIG_FILE_MYID_STRING, features.myid);
	fprintf(fp, CONFIG_FILE_GWID_STRING, features.gwid);
	fprintf(fp, CONFIG_FILE_UUID_STRING);
	for (int i=0; i<UUID_LENGTH; i++)
		fputc(features.uuid[i], fp);
	fputc('\n', fp);
	fprintf(fp, CONFIG_FILE_LOCATION_STRING);
	for (int i=0; i<LOCATION_MAX_LENGTH; i++)
		fputc(features.location[i], fp);
	fputc('\n', fp);
	for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
		fprintf(fp, CONFIG_FILE_ENABLED_FEATURE_STRING, i, features.feature_enabled[i]);
	fclose(fp);
}

void load_features_data() {
	FILE *fp = fopen(posix_config_filename, "r");
	if (fp== NULL) {
		// No config file found, create default features.
		for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
			features.feature_enabled[i] = true;

		memset(features.location, 0, LOCATION_MAX_LENGTH);
	} else {
		for (int i=0; i<WKPF_NUMBER_OF_FEATURES; i++)
			features.feature_enabled[i] = false;

		char *line = NULL;
		size_t len = 0;
		ssize_t read;

        while ((read = getline(&line, &len, fp)) != -1) {
        	if (prefix("MyID", line)) {
				int id;
				if (!sscanf(line, CONFIG_FILE_MYID_STRING, &id)) {
					printf("MyID in %s not in expected format, aborting...\n", posix_config_filename);
					abort();
				}
				features.myid = id;
				DEBUG_LOG(DBG_WKPF, "CONFIG: MyID = %d\n", features.myid);
			} else if (prefix("GwID", line)) {
				int id;
				if (!sscanf(line, CONFIG_FILE_GWID_STRING, &id)) {
					printf("GwID in %s not in expected format, aborting...\n", posix_config_filename);
					abort();
				}
				features.gwid = id;
				DEBUG_LOG(DBG_WKPF, "CONFIG: GwID = %d\n", features.gwid);
			} else if (prefix("UUID", line)) {
				DEBUG_LOG(DBG_WKPF, "CONFIG: UUID = ");
				for (int i=0; i<UUID_LENGTH; i++)
				{
					features.uuid[i] = fgetc(fp);
					DEBUG_LOG(DBG_WKPF, " %d", features.uuid[i]);
				}
				fgetc(fp); // read \n
				DEBUG_LOG(DBG_WKPF, "\n");
			} else if (prefix("Feature", line)) {
				int feature;
				int is_enabled;
				if(!sscanf(line, CONFIG_FILE_ENABLED_FEATURE_STRING, &feature, &is_enabled) == 1) {
					printf("Feature in %s not in expected format, aborting...\n", posix_config_filename);
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

	return length;
}

void wkpf_config_set_uuid(uint8_t* src) {
	if (!features_loaded)
		load_features_data();

	memcpy(features.uuid, src, UUID_LENGTH);

	save_features_data();
}

void wkpf_config_get_uuid(uint8_t* dest) {
	if (!features_loaded)
		load_features_data();

	memcpy(dest, features.uuid, UUID_LENGTH);
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

wkcomm_address_t wkpf_config_get_myid() {
	if (!features_loaded)
		load_features_data();

	return features.myid;
}

void wkpf_config_set_myid(wkcomm_address_t id) {
	if (!features_loaded)
		load_features_data();

	features.myid = id;

	save_features_data();
}

wkcomm_address_t wkpf_config_get_gwid() {
	if (!features_loaded)
		load_features_data();

	return features.gwid;
}

void wkpf_config_set_gwid(wkcomm_address_t id) {
	if (!features_loaded)
		load_features_data();

	features.gwid = id;

	save_features_data();
}
