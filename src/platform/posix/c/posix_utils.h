#ifndef POSIX_UTILSH
#define POSIX_UTILSH

#include "types.h"

extern dj_di_pointer posix_load_infusion_archive(char *filename);
extern void posix_parse_command_line(int argc, char* argv[]);

extern char** posix_argv;
extern char* posix_uart_filenames[4];
extern bool posix_arg_addnode;
extern uint16_t posix_local_network_id;
extern char * posix_pc_io_directory;

#endif // POSIX_UTILSH
