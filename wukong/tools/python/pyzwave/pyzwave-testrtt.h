#ifndef TESTRTT_H
#define TESTRTT_H

extern char PyZwave_messagebuffer[1024];
extern int PyZwave_src;
extern int PyZwave_print_debug_info;
extern int verbose;
extern int pyzwave_basic;
extern int pyzwave_generic;
extern int pyzwave_specific;
void zwave_check_state(unsigned char c);
int ZW_AddNodeToNetwork(int mode);
int ZW_RemoveNodeFromNetwork(int mode);
int ZW_SetDefault(void);
int PyZwave_init_usb(char *dev_name);
int PyZwave_init(char *host);
int PyZWave_send(unsigned id,unsigned char *in,int len);
int PyZwave_receive(int wait_msec);
int PyZwave_send(unsigned id,unsigned char *in,int len);
void PyZwave_get_init_data(void);
unsigned long PyZwave_get_addr(void);
int PyZwave_discover(void);
int PyZwave_hard_reset(void);
int PyZwave_basic_set(unsigned int node_id, unsigned char value);
int PyZwave_is_node_fail(unsigned node_id);
void PyZwave_remove_fail(unsigned int node_id);
void PyZwave_check_all_remove_fail(void);
void PyZwave_routing(unsigned node_id);
int PyZwave_get_device_type(unsigned node_id);
int PyZwave_zwavefd(void);
char *PyZwave_status(void);
void PyZwave_clearstatus(void);
#endif
