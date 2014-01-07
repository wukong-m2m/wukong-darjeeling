#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "core.h"
#include "types.h"
#include "heap.h"
#include "config.h"
#include "hooks.h"
#include "djarchive.h"
#include "pointerwidth.h"

char * ref_t_base_address;

char** posix_argv;
char* posix_uart_filenames[4];
bool posix_arg_addnode = false;

void posix_parse_uart_arg(char *arg) {
	int uart = arg[0];
	uart -= '0';
	if (uart < 0 || uart > 3 || arg[1]!='=') {
		printf("option -u/--uart format: <uart>=<file>, where <uart> is 0, 1, 2, or 3 and <file> is the device to connect to thise uart.\n");
		abort();
	}
	posix_uart_filenames[uart] = arg+2;
	printf("Uart %d at %s\n", uart, posix_uart_filenames[uart]);
}

void posix_parse_command_line(int argc, char* argv[]) {
	posix_argv = argv; // Used by wkpf_reprog code to do a reboot

	int c;
	while (1) {
		static struct option long_options[] = {
			{"uart",      required_argument, 0, 'u'},
			{"zwave_add", no_argument,       0, 'a'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "au:",
		    long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 'a':
				posix_arg_addnode = true;
				break;
			case 'u':
				posix_parse_uart_arg(optarg);
				break;
			default:
				abort ();
		}
	}
}

dj_di_pointer posix_load_infusion_archive(char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		printf("Unable to open the program flash file %s.\n", filename);
		exit(1);
	}

	// Determine the size of the archive
	fseek(fp, 0, SEEK_END);
	size_t length = ftell(fp);

	// Allocate memory for the archive
	char* di_archive_data = malloc(length+4);
	if (!di_archive_data) {
		printf("Unable to allocate memory to load the program flash file.\n");
		exit(1);
	}

	// Read the file into memory
	rewind(fp);
	fread(di_archive_data, sizeof(char), length, fp); // Skip 4 bytes that contain the archive length
	fclose(fp);

	return (dj_di_pointer)di_archive_data;
}


