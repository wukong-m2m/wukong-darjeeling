#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include "posix_utils.h"
#include "posix_pc_utils.h"
#include "wkcomm.h"

#define POSIX_PC_FILE_DIRECTION_IN  1
#define POSIX_PC_FILE_DIRECTION_OUT 2

FILE* get_property_file(wuobject_t *wuobject, char *property, int direction) {
	char dirname[1024];
	char filename[1024];
	char* mode;
	posix_get_node_directory(dirname, 1024);

	if (direction == POSIX_PC_FILE_DIRECTION_IN) {
		snprintf(filename, 1024, "%s/IN_%s_%d", dirname, property, wuobject->port_number);
		mode = "r";
	} else {
		snprintf(filename, 1024, "%s/OUT_%s_%d", dirname, property, wuobject->port_number);
		mode = "w";		
	}

	FILE* file = fopen(filename, mode);
	if (file == NULL) {
		// Maybe the file doesn't exist yet
		file = fopen(filename, "w");
		if (file != NULL) {
			// Write 0 as a default value in case the file will be read
			fprintf(file, "0\n");
			fclose(file);
			return fopen(filename, mode);
		} else {
			fprintf(stderr, "Can't open file for %s, on port %d", property, wuobject->port_number);
			exit(1);
		}
	}
	return file;
}

void posix_property_put(wuobject_t *wuobject, char *property, int value) {
	FILE* file = get_property_file(wuobject, property, POSIX_PC_FILE_DIRECTION_OUT);
	fprintf(file, "%d\n", value);
	fclose(file);
}

int posix_property_get(wuobject_t *wuobject, char *property) {
	FILE* file = get_property_file(wuobject, property, POSIX_PC_FILE_DIRECTION_IN);
	int value;
	fscanf(file, "%d\n", &value);
	fclose(file);
	return value;
}


