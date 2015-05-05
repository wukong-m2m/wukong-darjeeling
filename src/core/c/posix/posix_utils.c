#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

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
uint16_t posix_local_network_id = 0;
char* posix_pc_network_directory = "./djnetwork";
bool posix_pc_network_directory_specified = false;
char* posix_network_server_address = "127.0.0.1";
int posix_network_server_port = 10008;
char* posix_enabled_wuclasses_xml = NULL;
char posix_config_filename[1024];
char posix_app_infusion_filename[1024];

void posix_print_commandline_help() {
	printf(
"OVERVIEW: Darjeeling VM (posix platform)\n"
"\n"
"USAGE: darjeeling [options]\n"
"\n"
"OPTIONS:\n"
"  -h, --help                             Prints this message.\n"
"  -a, --zwave_add                        Tries to add this node to the master when starting the VM(same as pressing the button on the WuDevice).\n"
"  -u, --uart uart_nr=file                Connect VM uart_nr (where uart_nr is 0, 1, 2, or 3) to file.\n"
"                                         For example \"-u 2=/dev/ttyACM0 to conn)\" causes reads and writes to uart 2 to be redirected to /dev/ACM0.\n"
"  -s, --network_server address[:port]    Connect to the WuKongNetworkServer running at address. The port is optional, and is 10008 by default.\n"
"  -i, --network_server_id id             Set the id this client will use to connect to WuKongNetworkServer.java\n"
"\n"
"OPTIONS FOR POSIX_PC PLATFORM ONLY:\n"
"  -d, --network_directory dir            Set the directory to use to simulate sensors and actuators. A subdirectory node_id will be created for each node.\n"
"                                         NOTE:\n"
"                                         If directory IS NOT specified, the IO files for posix_pc will be created in ~/djnetwork,\n"
"                                         but the app_infusion.dja (the application) and config.txt in the current will be used.\n"
"                                         If a directory IS specified, the config.txt and app_infusion.dja in the node's subdirectory will be used. If they\n"
"                                         don't exist yet, they will be copied from the current directory first.\n"
"                                         This is to make sure each node in a simulated network has it's own application and configuration settings.\n"
"  -e, --enabled_wuclasses_xml file       Instead of using the generated wkpf_native_wuclasses_init, read the configuration from file at startup.\n"
"                                         (needs to be enabled in config.h by defining LOAD_ENABLED_WUCLASSES_AT_STARTUP)\n"
	);
}




void posix_parse_uart_arg(char *arg) {
	int uart = arg[0];
	uart -= '0';
	if (uart < 0 || uart > 3 || arg[1]!='=') {
		printf("option -u/--uart format: .\n");
		abort();
	}
	posix_uart_filenames[uart] = arg+2;
	printf("[posix platform parameters] Uart %d at %s\n", uart, posix_uart_filenames[uart]);
}

void posix_parse_networkserver_arg(char *arg) {
	// Should be either "address" or "address:port"
	char* colon = strstr(arg, ":");
	if (colon == NULL) {
		posix_network_server_address = arg;
	} else {
		// copy address and terminate by overwriting ':'' with a 0
		posix_network_server_address = strdup(arg);
		posix_network_server_address[colon-arg] = 0;
		// get the port number
		posix_network_server_port = atoi(colon+1);
	}
	printf("[posix platform parameters] Network server address: %s, port: %d\n", posix_network_server_address, posix_network_server_port);
}

void posix_parse_network_server_id_arg(char *arg) {
	posix_local_network_id = atoi(arg);
	printf("[posix platform parameters] Network id: %d\n", posix_local_network_id);
}

void posix_parse_network_directory_arg(char *arg) {
	posix_pc_network_directory = optarg;
	posix_pc_network_directory_specified = true;
	printf("[posix platform parameters] Sensor IO file system at: %s\n", posix_pc_network_directory);
}

void posix_get_node_directory(char* dest, int maxlen) {
	snprintf(dest, maxlen, "%s/node_%d", posix_pc_network_directory, posix_local_network_id);
	if (access(dest, F_OK) == -1) {
		char cmd[maxlen+10];
		snprintf(cmd, maxlen+10, "mkdir -p %s", dest);
		system(cmd);
	}
}

void posix_determine_app_archive_and_config_file() {
	if (posix_pc_network_directory_specified) {
		char dirname[1024];
		posix_get_node_directory(dirname, 1024);
		snprintf(posix_config_filename, 1024, "%s/config.txt", dirname);
		snprintf(posix_app_infusion_filename, 1024, "%s/app_infusion.dja", dirname);
	} else {
		// No directory specified. Use config and app in current directory
		snprintf(posix_config_filename, 1024, "config.txt");
		snprintf(posix_app_infusion_filename, 1024, "app_infusion.dja");
	}

	printf("[posix platform parameters] Using configuration in %s\n", posix_config_filename);
	printf("[posix platform parameters] Using application in %s\n", posix_app_infusion_filename);

	if (posix_pc_network_directory_specified) {
		if( access(posix_config_filename, F_OK) == -1 ) {
			printf("[posix platform parameters] %s not found, copying default from ./config.txt\n", posix_config_filename);
			char cmd[1024];
			snprintf(cmd, 1024, "cp config.txt %s", posix_config_filename);
			system(cmd);
		}

		if( access(posix_app_infusion_filename, F_OK) == -1 ) {
			printf("[posix platform parameters] %s not found, copying default from ./app_infusion.dja\n", posix_app_infusion_filename);
			char cmd[1024];
			snprintf(cmd, 1024, "cp app_infusion.dja %s", posix_app_infusion_filename);
			system(cmd);
		}
	}
}

void posix_parse_command_line(int argc, char* argv[]) {
	posix_argv = argv; // Used by wkpf_reprog code to do a reboot

	int c;
	while (1) {
		static struct option long_options[] = {
			{"help",      required_argument, 0, 'h'},
			{"zwave_add", no_argument,       0, 'a'},
			{"uart",      required_argument, 0, 'u'},
			{"network_server",      required_argument, 0, 's'},
			{"network_server_id",      required_argument, 0, 'i'},
			{"network_directory",      required_argument, 0, 'd'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hau:s:i:d:e:",
		    long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 'h':
				posix_print_commandline_help();
				break;
			case 'a':
				posix_arg_addnode = true;
				break;
			case 'u':
				posix_parse_uart_arg(optarg);
				break;
			case 's':
				posix_parse_networkserver_arg(optarg);
				break;
			case 'i':
				posix_parse_network_server_id_arg(optarg);
				break;
			case 'd':
				posix_parse_network_directory_arg(optarg);
				break;
			case 'e':
				posix_enabled_wuclasses_xml = optarg;
				printf("[posix platform parameters] Using enabled wuclasses xml in: %s\n", posix_enabled_wuclasses_xml);
				break;
			default:
				abort ();
		}
	}
	posix_determine_app_archive_and_config_file();
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


