#ifndef POSIX_UTILSH
#define POSIX_UTILSH

#include "types.h"

extern dj_di_pointer posix_load_infusion_archive(char *filename);
extern void posix_parse_command_line(int argc, char* argv[]);
extern void posix_get_node_directory(char* dest, int maxlen);

extern char** posix_argv;
extern char* posix_uart_filenames[4];
extern bool posix_arg_addnode;
extern uint32_t posix_local_network_id;
extern char* posix_pc_network_directory;
extern char* posix_network_server_address;
extern int posix_network_server_port;
extern char* posix_enabled_wuclasses_xml;
extern char posix_config_filename[1024];
extern char posix_app_infusion_filename[1024];

#endif // POSIX_UTILSH
