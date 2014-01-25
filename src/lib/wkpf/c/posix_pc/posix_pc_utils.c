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


FILE* get_property_file(wuobject_t *wuobject, char *property, char* mode) {
	char dirname[1024];
	char filename[1024];
	posix_get_node_directory(dirname, 1024);
	snprintf(filename, 1024, "%s/%s_%d", dirname, property, wuobject->port_number);

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
	FILE* file = get_property_file(wuobject, property, "w");
	fprintf(file, "%d\n", value);
	fclose(file);
}

int posix_property_get(wuobject_t *wuobject, char *property) {
	FILE* file = get_property_file(wuobject, property, "r");
	int value;
	fscanf(file, "%d\n", &value);
	return value;
}


