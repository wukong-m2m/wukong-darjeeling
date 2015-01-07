#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
//#include <windows.h>
#include <winsock2.h>
#else //_WIN32
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif //_WIN32

#ifdef _WIN32
#define snprintf _snprintf
#define read(f,b,n) recv(f,(char*)b,n,0)
#define write(f,b,n) send(f,(char*)b,n,0)
//#define STDIN_FILENO 0
//#define STDOUT_FILENO 1
#define usleep(t) Sleep(t/1000)
#endif //_WIN32

#ifdef _WIN32
#define ERRNO WSAGetLastError()
#define strerror(no) "WSAGetLastError()"
int server_port;
#else // _WIN32
#define ERRNO errno
#define closesocket close
#endif //_WIN32

#define CHECK_ME() printf("%s():%d\n", __FUNCTION__, __LINE__)

#define WAIT_ACK 1
#define WAIT_SOF 2
#define WAIT_LEN 3
#define WAIT_TYPE 4
#define WAIT_COMMAND 5
#define WAIT_DATA 6
#define WAIT_EOF 7
#define WAIT_CRC 8
#define WAIT_REQUEST 9
#define WAIT_RETRANSMIT 10
#define WAIT_INIT 11

unsigned char basic_get_report[]={0x20,2};
int zwavefd = -1;
int zstate = WAIT_INIT;
int zwave_ready = 0;
int testmode=0;
int ack_got=1;
int send_data_fin=1;
int cmd_succ=-1;
int PyZwave_print_debug_info=0;
int rtt_start_ms;
int interval = 500;
int verbose=1;
char *g_host=NULL;
char g_dev_name[256] = "/dev/ttyS1";
int g_instance = -1;
int g_instance_src = -1;
int g_instance_dst = -1;
int exit_time = -1;
int repeat = 0;
int nowait = 0;
int main_ret = 0;
int repeat_cmd;
int repeat_nodeid;
int repeat_value;
int pyzwave_basic=0;
int pyzwave_generic=0;
int pyzwave_specific=0;
char zwave_retransmit_buffer[64];
int zwave_retransmit_ptr=0;

char current_status[1024] = "stop";

void zwave_check_state(unsigned char c);
#define ZW_ACK 0x06
#define ZW_NAK 0x15
#define ZW_CAN 0x18
#define ZW_SOF 0x01
#define ZW_REQ 0x00
#define ZW_RES 0x01

#define GetInitData 0x2
#define ApplicationNodeInformation 0x3
#define GetControllerCapability 0x5
#define GetSerialCapability 0x7
#define ZW_SendData 0x13
#define ZW_SendNodeInformation 0x12
#define RFPowerLevelGet 0xBA
#define RFPowerLevelSet 0x17
#define GetNodeProtocolInfo 0x41
#define SetDefault          0x42
#define RequestNodeNeighborUpdate 0x48
#define AddNodeToNetwork 0x4a
#define RemoveNodeFromNetwork 0x4b
#define FUNC_ID_ZW_CONTROLLER_CHANGE 0x4D
#define SetLearnMode 0x50
#define EnableSUC            0x52
#define RequestNetworkUpdate 0x53
#define SetSUCNodeID         0x54
#define GetSUCNodeID         0x56
#define RequestNodeInfo 0x60
#define RemoveFailedNodeId 0x61
#define IsFailedNodeId 0x62
#define GetRoutingInformation 0x80
#define SendTestFrame 0xBE
#define Type_Library 0xBD
#define GetProtocolStatus 0xBF
#define FUNC_ID_MEMORY_GET_ID 0x20

#define CONTROLLER_CHANGE_START       2
#define CONTROLLER_CHANGE_STOP        5
#define CONTROLLER_CHANGE_STOP_FAILED 6

#define ASSIGN_COMPLETE           0x00
#define ASSIGN_NODEID_DONE        0x01  /*Node ID have been assigned*/
#define ASSIGN_RANGE_INFO_UPDATE  0x02  /*Node is doing Neighbor discovery*/


#define NODEINFO_LISTENING_SUPPORT          0x80
#define NODEINFO_ROUTING_SUPPORT            0x40
#define NODEINFO_OPTIONAL_FUNC_SUPPORT      0x80


#define ADD_NODE_ANY		0x1
#define ADD_NODE_CONTROLLER 	0x2
#define ADD_NODE_SLAVE		0x3
#define ADD_NODE_EXISTING	0x4
#define ADD_NODE_STOP		0x5
#define ADD_NODE_STOP_FAILED	0x6
#define ADD_NODE_OPTION_HIGH_POWER	0x80

#define TRANSMIT_OPTION_ACK                        0x01   //request acknowledge from destination node
#define TRANSMIT_OPTION_LOW_POWER                          0x02   // transmit at low output power level (1/3 of normal RF range)
#define TRANSMIT_OPTION_RETURN_ROUTE                       0x04   // request transmission via return route
#define TRANSMIT_OPTION_AUTO_ROUTE                         0x04   // request retransmission via repeater nodes
// do not use response route - Even if available
#define TRANSMIT_OPTION_NO_ROUTE                           0x10

// Basic class
#define COMMAND_CLASS_BASIC 0x20
#define BASIC_SET   0x1
#define BASIC_GET   0x2
#define BASIC_REPORT 0x3

// Scene controller
//
#define COMMAND_CLASS_SCENE_CONTROLLER 0x2D
#define SCENE_CONTROLLER_VERSION 0x1
#define SCENE_CONTROLLER_SET 0x1
#define SCENE_CONTROLLER_GET 0x2
#define SCENE_CONTROLLER_REPORT 0x3


#define COMMAND_CLASS_SCENE_ACTIVATION 							 0x2B
#define SCENE_ACTIVATION_SET                                                             0x01



// CXlass Association
#define COMMAND_CLASS_ASSOCIATION 0x85
#define ASSOCIATION_SET 0x1
#define ASSOCIATION_GET 0x2
#define ASSOCIATION_REPORT 0x3
#define ASSOCIATION_REMOVE 0x4
#define ASSOCIATION_GROUP_GET 0x5
#define ASSOCIATION_GROUP_REPORT 0x6

//Switch Multilevel command class commands
#define COMMAND_CLASS_SWITCH_MULTILEVEL                    0x26
#define SWITCH_MULTILEVEL_VERSION                          0x01
#define SWITCH_MULTILEVEL_GET                              0x02
#define SWITCH_MULTILEVEL_REPORT                           0x03
#define SWITCH_MULTILEVEL_SET                              0x01
#define SWITCH_MULTILEVEL_START_LEVEL_CHANGE               0x04
#define SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE                0x05

//SensorMultilevel
#define COMMAND_CLASS_SENSOR_MULTILEVEL 0x31
#define SENSOR_MULTILEVEL_GET 0x4
#define SENSOR_MULTILEVEL_REPORT 0x5

// Menufacture specific class
#define COMMAND_CLASS_MANUFACTURER_SPECIFIC             0x72
#define MANUFACTURER_SPECIFIC_VERSION                   0x01
#define MANUFACTURER_SPECIFIC_GET                       0x04
#define MANUFACTURER_SPECIFIC_REPORT                    0x05


// SIMPLE AV class
#define COMMAND_CLASS_SIMPLE_AV_CONTROL			0x94
#define SIMPLE_AV_CONTROL_SET				0x1
#define SIMPLE_AV_CONTROL_GET				0x2
#define SIMPLE_AV_CONTROL_REPORT			0x3
#define SIMPLE_AV_CONTROL_SUPPORTED_GET			0x4
#define SIMPLE_AV_CONTROL_SUPPORTED_REPORT		0x5
#define SIMPLE_AV_CONTROL_LEARN         		0x20
#define SIMPLE_AV_CONTROL_LEARN_REPORT         		0x21
#define SIMPLE_AV_CONTROL_RAW_SET         		0x22

// Configuration
#define COMMAND_CLASS_CONFIGURATION                                                      0x70
#define CONFIGURATION_VERSION                                                            0x01
#define CONFIGURATION_BULK_GET_V2                                                        0x08
#define CONFIGURATION_BULK_REPORT_V2                                                     0x09
#define CONFIGURATION_BULK_SET_V2                                                        0x07
#define CONFIGURATION_GET                                                                0x05
#define CONFIGURATION_REPORT                                                             0x06
#define CONFIGURATION_SET                                                                0x04
/* Values used for Configuration Report command */
#define CONFIGURATION_REPORT_LEVEL_SIZE_MASK                                             0x07
#define CONFIGURATION_REPORT_LEVEL_RESERVED_MASK                                         0xF8
#define CONFIGURATION_REPORT_LEVEL_RESERVED_SHIFT                                        0x03
/* Values used for Configuration Set command */
#define CONFIGURATION_SET_LEVEL_SIZE_MASK                                                0x07
#define CONFIGURATION_SET_LEVEL_RESERVED_MASK                                            0x78
#define CONFIGURATION_SET_LEVEL_RESERVED_SHIFT                                           0x03
#define CONFIGURATION_SET_LEVEL_DEFAULT_BIT_MASK                                         0x80

#define COMMAND_CLASS_INDICATOR                                                          0x87
/* Indicator command class commands */
#define INDICATOR_VERSION                                                                0x01
#define INDICATOR_GET                                                                    0x02
#define INDICATOR_REPORT                                                                 0x03
#define INDICATOR_SET                                                                    0x01

#define COMMAND_CLASS_WAKE_UP                                                            0x84
/* Wakeup command class */
#define WAKE_UP_INTERVAL_GET                                                             0x05
#define WAKE_UP_INTERVAL_REPORT                                                          0x06
#define WAKE_UP_INTERVAL_SET                                                             0x04
#define WAKE_UP_NO_MORE_INFORMATION                                                      0x08
#define WAKE_UP_NOTIFICATION                                                             0x07

#define COMMAND_CLASS_THERMOSTAT_MODE                                                    0x40
#define THERMOSTAT_MODE_VERSION                                                          0x01
#define THERMOSTAT_MODE_GET                                                              0x02
#define THERMOSTAT_MODE_REPORT                                                           0x03
#define THERMOSTAT_MODE_SET                                                              0x01
#define THERMOSTAT_MODE_SUPPORTED_GET                                                    0x04
#define THERMOSTAT_MODE_SUPPORTED_REPORT                                                 0x05

#define COMMAND_CLASS_THERMOSTAT_SETPOINT						 0x43
#define THERMOSTAT_SETPOINT_VERSION                                                      0x01
#define THERMOSTAT_SETPOINT_GET                                                          0x02
#define THERMOSTAT_SETPOINT_REPORT                                                       0x03
#define THERMOSTAT_SETPOINT_SET                                                          0x01
#define THERMOSTAT_SETPOINT_SUPPORTED_GET                                                0x04
#define THERMOSTAT_SETPOINT_SUPPORTED_REPORT                                             0x05



#define COMMAND_CLASS_ACTUATOR_CONF                                                0x2C
#define ACTUATOR_CONF_VERSION                                                      0x01
#define ACTUATOR_CONF_GET                                                          0x02
#define ACTUATOR_CONF_REPORT                                                       0x03
#define ACTUATOR_CONF_SET                                                          0x01


#define COMMAND_CLASS_MULTI_INSTANCE                                                     0x60
#define COMMAND_CLASS_MULTI_CHANNEL                                                     0x60
#define MULTI_INSTANCE_VERSION                                                           0x01
#define MULTI_INSTANCE_CMD_ENCAP                                                         0x06
#define MULTI_INSTANCE_GET                                                               0x04
#define MULTI_INSTANCE_REPORT                                                            0x05
#define MULTI_CHANNEL_END_POINT_GET_V2							 0x7
#define MULTI_CHANNEL_END_POINT_REPORT_V2						 0x8
#define MULTI_CHANNEL_CAPABILITY_GET_V2							 0x9
#define MULTI_CHANNEL_END_POINT_FIND_V2                                                  0x0B
#define MULTI_CHANNEL_END_POINT_FIND_REPORT_V2                                           0x0C

#define MULTI_CHANNEL_CAPABILITY_REPORT_V2						 0xA
#define MULTI_CHANNEL_CMD_ENCAP_V2                                                       0x0D

#define COMMAND_CLASS_METER                                                              0x32
#define METER_GET_V2                                                                     0x01
#define METER_REPORT_V2                                                                  0x02


#define COMMAND_CLASS_BATTERY                                                            0x80
#define BATTERY_GET                                                                      0x02
#define BATTERY_REPORT                                                                   0x03

#define COMMAND_CLASS_PROPRIETARY                                                        0x88
#define PROPRIETARY_GET                                                                  0x02
#define PROPRIETARY_REPORT                                                               0x03
#define PROPRIETARY_SET                                                                  0x01

#define COMMAND_CLASS_VERSION                                                            0x86
#define VERSION_GET                                                                      0x11
#define VERSION_REPORT                                                                   0x12

#define COMMAND_CLASS_MULTI_INSTANCE_V2                                                   0x60
#define MULTI_INSTANCE_GET_V2                                                            0x04
#define MULTI_INSTANCE_CMD_ENCAP_V2                                                      0x0d
#define MULTI_INSTANCE_REPORT_V2                                                         0x05

#define COMMAND_CLASS_MARK                                                               0xEF
#define COMMAND_CLASS_NO_OPERATION                                                       0x00

#define COMMAND_CLASS_SWITCH_ALL                                                         0x27

#define TRANSMIT_WAIT_FOR_ACK                              0xFF
#define TRANSMIT_COMPLETE_OK                               0x00
#define TRANSMIT_COMPLETE_NO_ACK                           0x01 //# retransmission error
#define TRANSMIT_COMPLETE_FAIL                             0x02 //# transmit error
#define TRANSMIT_ROUTING_NOT_IDLE                          0x03 //# transmit error
// Assign route transmit complete but no routes was found
#define TRANSMIT_COMPLETE_NOROUTE                          0x04 //# no route found in assignroute
unsigned char zseq=0x35;
int txoptions=0;
int total_delay=0;
int total_count=0;
int g_flood = 0;

#define NEXT_SEQ() (++zseq?zseq:++zseq) // avoid 0 since SerialAPI won't callback if zseq(funcID) == 0


static int SerialAPI_request(unsigned char *buf, int len);
static void zwavecmd_configuration_set(unsigned int id, unsigned int no, int v);
static void zwavecmd_configuration_get(unsigned int id,unsigned int no);
static void zwavecmd_configuration_dump(unsigned int id,int num);
static void zwavecmd_configuration_dump_next(void *data,void *payload,int len);
static void zwavecmd_configuration_bulk_get(unsigned int id, unsigned int offset,int len);
static void zwavecmd_configuration_bulk_set(unsigned int id, unsigned int offset, char *argv[]);
static void zwavecmd_association_get(unsigned int id,unsigned int group);
static void zwavecmd_association_remove_all(unsigned int id,unsigned int group);
static void zwavecmd_configuration_bulk_set_buf(unsigned int id, unsigned int offset, unsigned char * data, int len);
static void zwavecmd_association_set_buf(unsigned int id,unsigned int group, unsigned char * data, int len);
static void hsk200_key_data_report_check(void * data);
static void hsk200_keymap_report_check(void * data);
static void hsk200_conf_report_check(void * data);
static void hsk200_macro_report_check(void * data);
static void hsk200_assoc_report_check(void * data);
static void hsk200_conf_set_check(void * data);
static void hsk200_macro_set_check(void * data);
static void hsk200_assoc_set_check(void * data);
static void hsk200_key_data_set_finished(void * data, int txStatus);
static void hsk200_assoc_set_finished(void * data, int txStatus);
static void hsk200_assoc_remove_finished(void * data, int txStatus);

unsigned int get_tick_ms(void)
{
#ifdef _WIN32
    return GetTickCount();
#else //_WIN32
    struct timeval now;

    gettimeofday(&now,NULL);
    return now.tv_sec*1000+now.tv_usec/1000;
#endif //_WIN32
}

char * basic_type_string(int v)
{
    static char str[10];

    switch (v) {
        case 1:
            return "Portable controller";
        case 2:
            return "Static controller";
        case 3:
            return "Slave";
        case 4:
            return "Routing slave";
        default:
            sprintf(str, "%02x", v);
            return str;
    }
}
char * cmd_class_string(int cls)
{
    static char str[10];

    if (cls == COMMAND_CLASS_BASIC)
        return "COMMAND_CLASS_BASIC";
    else if (cls == COMMAND_CLASS_SCENE_CONTROLLER)
        return "COMMAND_CLASS_SCENE_CONTROLLER";
    else if (cls == COMMAND_CLASS_SCENE_ACTIVATION)
        return "COMMAND_CLASS_SCENE_ACTIVATION";
    else if (cls == COMMAND_CLASS_ASSOCIATION)
        return "COMMAND_CLASS_ASSOCIATION";
    else if (cls == COMMAND_CLASS_SWITCH_MULTILEVEL)
        return "COMMAND_CLASS_SWITCH_MULTILEVEL";
    else if (cls == COMMAND_CLASS_SENSOR_MULTILEVEL)
        return "COMMAND_CLASS_SENSOR_MULTILEVEL";
    else if (cls == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
        return "COMMAND_CLASS_MANUFACTURER_SPECIFIC";
    else if (cls == COMMAND_CLASS_SIMPLE_AV_CONTROL)
        return "COMMAND_CLASS_SIMPLE_AV_CONTROL";
    else if (cls == COMMAND_CLASS_CONFIGURATION)
        return "COMMAND_CLASS_CONFIGURATION";
    else if (cls == COMMAND_CLASS_INDICATOR)
        return "COMMAND_CLASS_INDICATOR";
    else if (cls == COMMAND_CLASS_WAKE_UP)
        return "COMMAND_CLASS_WAKE_UP";
    else if (cls == COMMAND_CLASS_THERMOSTAT_MODE)
        return "COMMAND_CLASS_THERMOSTAT_MODE";
    else if (cls == COMMAND_CLASS_THERMOSTAT_SETPOINT)
        return "COMMAND_CLASS_THERMOSTAT_SETPOINT";
    else if (cls == COMMAND_CLASS_ACTUATOR_CONF)
        return "COMMAND_CLASS_ACTUATOR_CONF";
    else if (cls == COMMAND_CLASS_MULTI_INSTANCE)
        return "COMMAND_CLASS_MULTI_INSTANCE";
    else if (cls == COMMAND_CLASS_METER)
        return "COMMAND_CLASS_METER";
    else if (cls == COMMAND_CLASS_BATTERY)
        return "COMMAND_CLASS_BATTERY";
    else if (cls == COMMAND_CLASS_PROPRIETARY)
        return "COMMAND_CLASS_PROPRIETARY";
    else if (cls == COMMAND_CLASS_VERSION)
        return "COMMAND_CLASS_VERSION";
    else if (cls == COMMAND_CLASS_MULTI_INSTANCE_V2)
        return "COMMAND_CLASS_MULTI_INSTANCE_V2";
    else if (cls == COMMAND_CLASS_SWITCH_ALL)
        return "COMMAND_CLASS_SWITCH_ALL";
    else if (cls == COMMAND_CLASS_MARK)
        return "COMMAND_CLASS_MARK";
    else if (cls == COMMAND_CLASS_NO_OPERATION)
        return "COMMAND_CLASS_NO_OPERATION";
    else {
        sprintf(str, "%02x", cls);
        return str;
    }
}

void clear_serial_api_queue(void)
{
    unsigned char ack = ZW_ACK;
    unsigned char c;
    struct timeval to;
    fd_set rs;
    int n;

    write(zwavefd, &ack, 1);	// ack previous frames

    while(1) {
        to.tv_sec = 0;
        to.tv_usec = 1000;

        FD_ZERO(&rs);
        FD_SET(zwavefd,&rs);
        n = select(zwavefd+1,&rs,NULL,NULL,&to);
        if (n == 0) {
            break;
        }
        else if (n < 0) {
            printf("ERROR:rtt.c clear_serial_api_queue: select() error\n");
            exit(1);
        }
        n = read(zwavefd, &c,1);
        if (n != 1) {
            printf("ERROR:rtt.c clear_serial_api_queue: read error! n=%d\n", n);
            exit(1);
        }
        zwave_check_state(c);
    }
    zstate = WAIT_SOF;
    /*
       write(zwavefd, &ack, 1);	// ack previous frames

       while(1) {
       FD_ZERO(&rs);
       FD_SET(zwavefd, &rs);
       to.tv_sec = 0;
       to.tv_usec = 1000;
       if ((n=select(zwavefd+1,&rs,NULL,NULL, &to))<=0) {
       break;
       }
       if (FD_ISSET(zwavefd,&rs)) {
       read(zwavefd, buf, sizeof(buf));
       }
       }
       */
    if(verbose) printf("INFO:rtt.c clear_serial_api_queue: done\n");
    fflush(stdout);
}

int ZW_sendData(unsigned id,unsigned char *in,int len)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = ZW_SendData;
    buf[2] = id;
    buf[3] = len;
    memcpy(buf+4,in,len);
    buf[4+len] = txoptions;
    buf[5+len] = NEXT_SEQ();
    send_data_fin = 0;
    return SerialAPI_request(buf, len + 6);
}

static void (*senddata_ack_callback)(void *data,int r)=NULL;
static void *senddata_ack_callback_data=NULL;
static void (*class_callback[256])(void *data, void *payload,int len)={NULL};
static void (*class_persistent_callback[256])(int src, void *payload,int len)={NULL};
static void *class_callback_data[256]={NULL};

void register_senddata_ack_callback(void (*f)(void *data,int r), void *data)
{
    senddata_ack_callback = f;
    senddata_ack_callback_data = data;
}

//Added discover call back function- Sen 2011.8.8
void register_discover_callback(void (*f)(void *data,int r), void *data)
{
    senddata_ack_callback = f;
    senddata_ack_callback_data = data;
}

void register_class_callback(int class,void (*f)(void *data, void *payload,int len), void *data)
{
    class_callback[class] = f;
    class_callback_data[class] = data;
}
void register_persistent_class_callback(int class,void (*f)(int src, void *payload,int len))
{
    class_persistent_callback[class] = f;
}

void execute_class_callback(int src, int class,void *payload,int len)
{
    //if(verbose) printf("class %x: cmd %x\n", class, ((unsigned char *)payload)[0]);
    if (class_callback[class]) {
        void (*cb)(void *data,void *payload,int len) = class_callback[class];
        class_callback[class] = NULL;
        cb(class_callback_data[class],payload,len);
    }
    if (class_persistent_callback[class]) {
        void (*cb)(int src, void *payload,int len) = class_persistent_callback[class];
        cb(src, payload,len);
    }
}

void execute_senddata_ack_callback(int r)
{
    if (senddata_ack_callback) {
        void (*cb)(void *data,int r) = senddata_ack_callback;
        senddata_ack_callback = NULL;
        cb(senddata_ack_callback_data,r);
    }
}

#define MAX_IDLE 50
void (*idle_callback[MAX_IDLE])(void *data)={NULL};
void *idle_callback_data[MAX_IDLE];

void register_idle_callback(void (*f)(void *data),void *data)
{
    int i;
    for(i=0;i<MAX_IDLE;i++)
        if (idle_callback[i] == NULL) {
            idle_callback[i] = f;
            idle_callback_data[i] = data;
            return;
        }
    printf("WARNING:rtt.c register_idle_callback: failed\n");
}

void execute_idle_callback(void)
{
    int i;
    for(i=0;i<MAX_IDLE;i++) {
        if (idle_callback[i]) {
            void (*cb)(void *data) = idle_callback[i];
            idle_callback[i] = NULL;
            cb(idle_callback_data[i]);
        }
    }
}
int ZW_ApplicationNodeInformation(unsigned devmask, unsigned char generic, unsigned char specific, int argc, char *argv[])
{
    unsigned char buf[255];
    int l = 6;

    buf[0] = ZW_REQ;
    buf[1] = ApplicationNodeInformation;
    buf[2] = devmask;
    buf[3] = generic;
    buf[4] = specific;
    buf[5] = 0;
    while(argc > 0) {
        buf[l] = atoi(*argv);
        argv++;
        l++;
        argc--;
    }
    buf[5] = l - 6;
    return SerialAPI_request(buf, l );
}

int ZW_sendNodeInformation(unsigned id,unsigned txOption)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = ZW_SendNodeInformation;
    buf[2] = id;
    buf[3] = txOption;
    buf[4] = NEXT_SEQ();
    return SerialAPI_request(buf, 5 );
}

int ZW_SetDefault(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = SetDefault;	// 0x42
    buf[2] = NEXT_SEQ();
    return SerialAPI_request(buf, 3);
}
int ZW_GetSUCNodeID(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetSUCNodeID;
    return SerialAPI_request(buf, 2);
}


int ZW_EnableSUC(unsigned char state, unsigned char cap )
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = EnableSUC;
    buf[2] = state;
    buf[3] = cap;
    return SerialAPI_request(buf, 4);
}
int ZW_SetSUCNodeID(unsigned char id,unsigned char SUC, unsigned char power, unsigned char cap )
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = SetSUCNodeID;
    buf[2] = id;
    buf[3] = SUC;
    buf[4] = power;
    buf[5] = cap;
    return SerialAPI_request(buf, 6);
}

int ZW_GetSerialCapability(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetSerialCapability;
    return SerialAPI_request(buf, 2);
}
int ZW_ControllerChange(int mode)
{
    unsigned char buf[16];

    buf[0] = ZW_REQ;
    buf[1] = FUNC_ID_ZW_CONTROLLER_CHANGE;
    buf[2] = mode;
    buf[3] = NEXT_SEQ();
    return SerialAPI_request(buf, 4);
}
int ZW_SetLearnMode(int on)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = SetLearnMode;
    buf[2] = on;
    buf[3] = NEXT_SEQ();
    return SerialAPI_request(buf, 4);
}
int ZW_GetControllerCapability(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetControllerCapability;
    return SerialAPI_request(buf, 2);
}
int ZW_RFPowerLevelGet(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RFPowerLevelGet;
    return SerialAPI_request(buf, 2);
}
int ZW_GetInitData(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetInitData;
    return SerialAPI_request(buf, 2);
}
int ZW_GetNodeProtocolInfo(int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetNodeProtocolInfo;
    buf[2] = id;
    return SerialAPI_request(buf, 3);
}
int ZW_GetRoutingInformation(unsigned int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetRoutingInformation;
    buf[2] = id;
    return SerialAPI_request(buf, 3);
}

int ZW_RequestNodeNeighborUpdate(int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RequestNodeNeighborUpdate;
    buf[2] = id;
    buf[3] = NEXT_SEQ();
    return SerialAPI_request(buf, 4);
}
int ZW_RFPowerLevelSet(int v)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RFPowerLevelSet;
    buf[2] = v;
    return SerialAPI_request(buf, 3);
}
int ZW_AddNodeToNetwork(int mode)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = AddNodeToNetwork;
    buf[2] = mode;
    buf[3] = NEXT_SEQ();
    return SerialAPI_request(buf, 4);
}
int ZW_RemoveNodeFromNetwork(int mode)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RemoveNodeFromNetwork;
    buf[2] = mode;
    buf[3] = NEXT_SEQ();
    return SerialAPI_request(buf, 4);
}
int ZW_RequestNetworkUpdate(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RequestNetworkUpdate;
    buf[2] = NEXT_SEQ();
    return SerialAPI_request(buf, 3);
}
int ZW_SendTestFrame(int id,int v)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = SendTestFrame;
    buf[2] = id;
    buf[3] = v;
    buf[4] = NEXT_SEQ();
    return SerialAPI_request(buf, 5);
}
int ZW_Type_Library(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = Type_Library;
    return SerialAPI_request(buf, 2);
}
int ZW_GetProtocolStatus(void)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = GetProtocolStatus;
    return SerialAPI_request(buf, 2);
}
int ZW_MemoryGetID(void)
{
    unsigned char buf[] = {ZW_REQ, FUNC_ID_MEMORY_GET_ID};

    return SerialAPI_request(buf, 2);
}

int ZW_RequestNodeInfo(int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RequestNodeInfo;
    buf[2] = id;
    return SerialAPI_request(buf, 3);
}
int ZW_isFailedNodeId(int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = IsFailedNodeId;
    buf[2] = id;
    return SerialAPI_request(buf, 3);
}
int ZW_removeFailedNodeId(int id)
{
    unsigned char buf[255];

    buf[0] = ZW_REQ;
    buf[1] = RemoveFailedNodeId;
    buf[2] = id;
    return SerialAPI_request(buf, 3);
}

int SerialAPI_soft_reset(void)
{
    unsigned char buf[2];

    buf[0] = ZW_REQ;
    buf[1] = 0x08;	// FUNC_ID_SERIAL_API_SOFT_RESET
    return SerialAPI_request(buf, 2);
}
int SerialAPI_request(unsigned char *buf, int len)
{
    unsigned char c = 1;
    int i,n;
    unsigned char crc;
    fd_set rs;
    struct timeval to;
    int wlen;
    int retry = 5;


    while (1) {
        // read out pending request from Z-Wave
        while(1) {
            to.tv_sec = 0;
            to.tv_usec = 10000;

            FD_ZERO(&rs);
            FD_SET(zwavefd,&rs);
            n = select(zwavefd+1,&rs,NULL,NULL,&to);
            if (n == 0) {
                break;
            }
            else if (n < 0) {
                printf("ERROR:rtt.c SerialAPI_request: 1st select() error\n");
                exit(1);
            }
            n = read(zwavefd, &c,1);
            if (n != 1) {
                printf("ERROR:rtt.c SerialAPI_request: 1st read error ! n=%d\n", n);
                exit(1);
            }
            zwave_check_state(c);
        }
        if (zstate != WAIT_SOF) {	// wait for WAIT_SOF statie (idle state)
            printf("WARNING:rtt.c SerialAPI_request: not in ready state zstate=%d\n", zstate);
            usleep(100*1000);
			// ACK the packet no matter what it is
			c=6;
			write(zwavefd,&c,1);
            //continue;
        }
		zstate = WAIT_SOF;

        // send SerialAPI request
		zwave_retransmit_ptr=0;
        c=1;
        write(zwavefd, &c, 1);	// SOF (start of frame)
		zwave_retransmit_buffer[zwave_retransmit_ptr++] = c;
        c = len+1;
        write(zwavefd, &c, 1);	// len (length of frame)
		zwave_retransmit_buffer[zwave_retransmit_ptr++] = c;
        wlen = write(zwavefd, buf,len);	// REQ, cmd, data
        if (wlen != len) {
            printf("WARNING:rtt.c SerialAPI_request: write fail %d %d\n", len,wlen);
        }
		for(i=0;i<len;i++) {
			zwave_retransmit_buffer[zwave_retransmit_ptr++] = buf[i];
		}

        crc = 0xff;
        crc = crc ^ (len+1);
        for(i=0;i<len;i++)
            crc = crc ^ buf[i];
        write(zwavefd,&crc,1);	// LRC checksum
		zwave_retransmit_buffer[zwave_retransmit_ptr++] = crc;
        if (PyZwave_print_debug_info) {
            printf("DEBUG:rtt.c SerialAPI_request: Send len=%d ", len+1);
            for(i=0;i<len;i++)
                printf("%02x ", buf[i]);
            printf("CRC=0x%x\n", crc);
        }
        zstate = WAIT_ACK;
        ack_got = 0;

        // get SerialAPI ack
        while(1) {
            to.tv_sec = 1;
            to.tv_usec = 0;

            FD_ZERO(&rs);
            FD_SET(zwavefd,&rs);
            n = select(zwavefd+1,&rs,NULL,NULL,&to);
            if (n == 0) {
                printf("WARNING:rtt.c SerialAPI_request: timeout without ACK !!!\n");
                break;
            }
            else if (n < 0) {
                printf("ERROR:rtt.c SerialAPI_request: 2nd select() error\n");
                exit(1);
            }
            n = read(zwavefd, &c,1);
            if (n != 1) {
                printf("ERROR:rtt.c SerialAPI_request: 2nd read error ! n=%d\n", n);
                exit(1);
            }
            zwave_check_state(c);

            if (ack_got) {
                return 0;
            } else {
                printf("WARNING:rtt.c SerialAPI_request: Ack error! zstate=%d ack_got=%d c=%d\n", zstate, ack_got, c);
                //break;
            }
			if (zstate == WAIT_RETRANSMIT) {
				zstate = WAIT_SOF;
				write(zwavefd, zwave_retransmit_buffer, zwave_retransmit_ptr);
			}

        }
        if (!retry--) {
            printf("ERROR:rtt.c SerialAPI_request: cannot retry");
            for (i=0; i<len; i++) {
                printf("%02x ", buf[i]);
            }
            printf("\t");
            printf("%s() error!!!\n", __FUNCTION__);
            exit(1);
        }
        printf("WARNING:rtt.c SerialAPI_request: %s() retry\n", __FUNCTION__);
    }
    return -1;
}

void zwave_sendClassCommand(unsigned char id,unsigned char cls, unsigned char cmd, unsigned char *args, int len)
{
    int i=0,j=0;
    unsigned char buf[256];

    if (g_instance != -1) {
        buf[j++] = COMMAND_CLASS_MULTI_INSTANCE;
        buf[j++] = MULTI_CHANNEL_CMD_ENCAP_V2;
        buf[j++] = 0;
        buf[j++] = g_instance;
    }

    buf[j++] = cls;
    buf[j++] = cmd;
    for(i=0;i<len;i++) {
        buf[j++] = args[i];
    }

    ZW_sendData(id,buf,j);
}

void ZW_sendRaw(char *argv[])
{
    unsigned char buf[255];
    unsigned char id = atoi(argv[0]);
    int i;

    argv++;
    i = 0;
    while(*argv) {
        buf[i] = strtoul(*argv,NULL,16);
        i++;
        argv++;
    }
    ZW_sendData(id,buf,i);
}

void zwavecmd_scene_activation_set(unsigned int id, unsigned char scene)
{
    unsigned char buf[6];

    buf[0] = scene;
    zwave_sendClassCommand(id, COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET,buf,1);
}
void zwavecmd_simple_av_set(unsigned int id, unsigned int itemid, unsigned int key,int seq)
{
    unsigned char buf[6];

    buf[0] = seq; // seq
    buf[1] = 0; // Key up
    buf[2] = itemid/256;
    buf[3] = itemid % 256;
    buf[4] = key/256;
    buf[5] = key % 256;
    zwave_sendClassCommand(id, COMMAND_CLASS_SIMPLE_AV_CONTROL, SIMPLE_AV_CONTROL_SET, buf, 6);
}

unsigned char irbuf[512*4];
int irbuf_len = 0;
int irbuf_ptr = 0;
int irbuf_srcnode;

void zwavecmd_simple_av_send_raw(unsigned int id, char *file)
{
    unsigned char buf[64];
    FILE *fp;
    int len;

    if (irbuf_ptr&&irbuf_len <= 0) return;
    if (file) {
        fp = fopen(file,"r");
        if (fp == NULL) return;
        irbuf_len = fread(irbuf, 1,sizeof(irbuf), fp);
        fclose(fp);
        if(verbose) printf("INFO:rtt.c zwavecmd_simple_av_send_raw: read %d bytes\n", irbuf_len);
    }
    if (id == 0)
        id = irbuf_srcnode;
    else
        irbuf_srcnode = id;

    buf[0] = irbuf_ptr/44;
    if (irbuf_len <= 44) {
        buf[1]=irbuf_len|0x80;
        len = irbuf_len;
    } else {
        buf[1] = 44;
        len = 44;
    }
    if(verbose) printf("INFO:rtt.c zwavecmd_simple_av_send_raw: send %d bytes\n", len);
    memcpy(buf+2,irbuf+irbuf_ptr, len);

    zwave_sendClassCommand(id, COMMAND_CLASS_SIMPLE_AV_CONTROL,SIMPLE_AV_CONTROL_RAW_SET,buf, len+2);
    irbuf_len -= len;
    irbuf_ptr += len;
}

void zwavecmd_simple_av_learn(unsigned int id, unsigned int key)
{
    unsigned char buf[4];

    buf[0] = key; // seq
    zwave_sendClassCommand(id, COMMAND_CLASS_SIMPLE_AV_CONTROL, SIMPLE_AV_CONTROL_LEARN, buf, 1);
}


#define HSK200_N_GROUPS 10
#define HSK200_N_ASSOC_IN_GROUP 4

#define HSK200_N_CONFIG 15
#define HSK200_N_KEY 64
#define HSK200_N_CHANNEL 8

#define MAX_SCENES 4
#define MAX_IR_MACRO 8
#define MACRO_UNIT_SIZE 2
#define MACRO_TYPE_MAX 2
#define IR_MACRO_SPACE_SIZE (MAX_SCENES*MAX_IR_MACRO*MACRO_UNIT_SIZE*MACRO_TYPE_MAX)

unsigned char hsk200_assoc[HSK200_N_GROUPS*HSK200_N_ASSOC_IN_GROUP];

unsigned char ir_macro[IR_MACRO_SPACE_SIZE];
unsigned char ir_macro_flag[IR_MACRO_SPACE_SIZE];

int hsk200_config[HSK200_N_CONFIG];

unsigned char hsk200_keymap[HSK200_N_CHANNEL][HSK200_N_KEY/8];
unsigned char hsk200_keymap_flag[HSK200_N_CHANNEL][HSK200_N_KEY];
unsigned char hsk200_database[HSK200_N_CHANNEL][HSK200_N_KEY][80];

int hsk200_node_id;
int g_cur_key;
int g_cur_conf;
int hsk200_backup_all = 0;

int hsk200_config_load(char * filename)
{
    FILE * fp;
    int ret;

    fp = fopen(filename, "rb");
    if (!fp)
        return -1;
    ret = fread(hsk200_config, sizeof(hsk200_config), 1, fp);
    ret = fread(hsk200_assoc, sizeof(hsk200_assoc), 1, fp);
    ret = fread(ir_macro, sizeof(ir_macro), 1, fp);
    ret = fread(hsk200_keymap, sizeof(hsk200_keymap), 1, fp);
    ret = fread(hsk200_database, sizeof(hsk200_database), 1, fp);
    fclose(fp);
    return 0;
}
int hsk200_config_save(char * filename)
{
    FILE * fp;
    int ret;

    fp = fopen(filename, "wb");
    if (!fp)
        return -1;
    ret = fwrite(hsk200_config, sizeof(hsk200_config), 1, fp);
    ret = fwrite(hsk200_assoc, sizeof(hsk200_assoc), 1, fp);
    ret = fwrite(ir_macro, sizeof(ir_macro), 1, fp);
    ret = fwrite(hsk200_keymap, sizeof(hsk200_keymap), 1, fp);
    ret = fwrite(hsk200_database, sizeof(hsk200_database), 1, fp);
    fclose(fp);

    return 0;
}

void hsk200_association_report_cb(int src, void * payload, int len)
{
    unsigned char * buf = (unsigned char *)payload;
    int group;
    int n = 0;
    int i;

    if (buf[0] == ASSOCIATION_REPORT) {
        if (buf[1] && (buf[1] <= HSK200_N_GROUPS)) {
            group = buf[1]-1;
            memset(&hsk200_assoc[group*HSK200_N_ASSOC_IN_GROUP], 0, HSK200_N_ASSOC_IN_GROUP);
            for (i=0; i<HSK200_N_ASSOC_IN_GROUP; i++) {
                if ((i+4)<len && buf[i+4]) {
                    hsk200_assoc[group*HSK200_N_ASSOC_IN_GROUP+n] = buf[i+4];
                    n++;
                }
            }
            hsk200_assoc_report_check(NULL);
        }
    }
}
void hsk200_configuration_report_cb(int src, void * payload, int len)
{
    unsigned char * buf = (unsigned char *)payload;
    unsigned char addr;
    unsigned char size;
    int i;

    if (buf[0] == CONFIGURATION_REPORT) {
        if ((buf[1]) && (buf[1] <= HSK200_N_CONFIG))
            hsk200_config[buf[1]-1] = buf[3];
        hsk200_conf_report_check(NULL);
    }
    if (buf[0] == CONFIGURATION_BULK_REPORT_V2) {
        size = buf[3] * (buf[5] & 0x7);
        if ((g_instance_src != g_instance) || (g_instance <= 0)) {
            printf("WARNING:rtt.c hsk200_configuration_report_cb: Error g_instance=%d, g_instance_src=%d\n", g_instance, g_instance_src);
            return;
        }
        if ((buf[1]==0) && (buf[2]==23)) {	// key map
            memcpy(hsk200_keymap[g_instance_src-1], buf+6, HSK200_N_KEY/8);
            hsk200_keymap_report_check(NULL);
        }
        else if ((buf[1]==0) && (buf[2]>=25) && (buf[2]<57)) {	// IR macro
            addr = (buf[2] - 25)*4;
            if ((addr + size) > (unsigned char)sizeof(ir_macro))
                size = sizeof(ir_macro) - addr;
            memcpy(ir_macro+addr, buf+6, size);
            memset(ir_macro_flag+addr, 0xff, size);
            hsk200_macro_report_check(NULL);
        }
        else if ((buf[1]==0) && (buf[2]==21)) {	// current key
            g_cur_key = 0;
            for (i=0; i<(buf[5]&0x7); i++) {
                g_cur_key <<= 8;
                g_cur_key += buf[6+i];
            }
            if (g_cur_key < HSK200_N_KEY)
                hsk200_keymap_flag[g_instance-1][g_cur_key] = 0x1;
            hsk200_key_data_report_check(NULL);
        }
        else if ((buf[1]==0) && (buf[2]==1)) {	// key data
            if (hsk200_keymap_flag[g_instance-1][g_cur_key] != 0x1)
                return;
            memcpy(&hsk200_database[g_instance-1][g_cur_key][0], buf+6, size);
            hsk200_keymap_flag[g_instance-1][g_cur_key] |= 0x2;
            hsk200_key_data_report_check(NULL);
        }
        else if ((buf[1]==0) && (buf[2]==11)) {	// key data
            if (hsk200_keymap_flag[g_instance-1][g_cur_key] != 0x3)
                return;
            memcpy(&hsk200_database[g_instance-1][g_cur_key][40], buf+6, size);
            hsk200_keymap_flag[g_instance-1][g_cur_key] |= 0x4;
            hsk200_key_data_report_check(NULL);
        }
    }
}

void hsk200_key_data_set_check(void * data)
{
    unsigned char buf[2];
    while (1) {
        if ((hsk200_keymap[g_instance-1][g_cur_key/8] & (1 << (g_cur_key%8)))) { // key exist
            //idle_callback[0] = hsk200_key_data_set_check;
            register_senddata_ack_callback(hsk200_key_data_set_finished, NULL);
            if (!(hsk200_keymap_flag[g_instance-1][g_cur_key] & 0x1)) {
                buf[0] = 0;
                buf[1] = g_cur_key;
                g_cur_conf = 21;
                zwavecmd_configuration_bulk_set_buf(hsk200_node_id, g_cur_conf, buf, 2);
                return;
            }
            else if (!(hsk200_keymap_flag[g_instance-1][g_cur_key] & 0x2)) {
                g_cur_conf = 1;
                zwavecmd_configuration_bulk_set_buf(hsk200_node_id, g_cur_conf, &hsk200_database[g_instance-1][g_cur_key][0], 40);
                return;
            }
            else if (!(hsk200_keymap_flag[g_instance-1][g_cur_key] & 0x4)) {
                g_cur_conf = 11;
                zwavecmd_configuration_bulk_set_buf(hsk200_node_id, g_cur_conf, &hsk200_database[g_instance-1][g_cur_key][40], 40);
                return;
            }
            //idle_callback[0] = NULL;
            register_senddata_ack_callback(NULL, NULL);
        }
        g_cur_key++;
        if (g_cur_key == HSK200_N_KEY) {
            g_cur_key = 0;
            g_instance++;
        }
        if (g_instance > HSK200_N_CHANNEL) {
            break;
        }
    }
    printf("INFO:rtt.c hsk200_key_data_set_check: ok\n");
}
void hsk200_assoc_remove_check(void * data)
{
    if (g_cur_conf < HSK200_N_GROUPS) {
        g_instance = -1;
        //idle_callback[0] = hsk200_assoc_remove_check;
        register_senddata_ack_callback(hsk200_assoc_remove_finished, NULL);
        zwavecmd_association_remove_all(hsk200_node_id, g_cur_conf+1);
        return;
    }
    printf("INFO:rtt.c hsk200_assoc_remove_check: ok\n");
    g_cur_conf = 0;
    hsk200_assoc_set_check(NULL);
}
void hsk200_assoc_set_check(void * data)
{
    int len;
    while (g_cur_conf < HSK200_N_GROUPS) {
        g_instance = -1;
        len = HSK200_N_ASSOC_IN_GROUP;
        while (len) {
            if (hsk200_assoc[g_cur_conf*HSK200_N_ASSOC_IN_GROUP+len-1])
                break;
            len--;
        }
        if (!len) {
            g_cur_conf++;
            continue;
        }
        //idle_callback[0] = hsk200_assoc_set_check;
        register_senddata_ack_callback(hsk200_assoc_set_finished, NULL);
        zwavecmd_association_set_buf(hsk200_node_id, g_cur_conf+1, hsk200_assoc+HSK200_N_ASSOC_IN_GROUP*g_cur_conf, len);
        return;
    }
    printf("INFO:rtt.c hsk200_assoc_set_check: ok\n");
    g_cur_key = 0;
    g_instance = 1;
    hsk200_key_data_set_check(NULL);
}
void hsk200_assoc_report_check(void * data)
{
    int i, j;
    for (i=0; i<HSK200_N_GROUPS; i++) {
        if (hsk200_assoc[i*HSK200_N_ASSOC_IN_GROUP] == 0xff) {
            g_instance = -1;
            if (send_data_fin)
                zwavecmd_association_get(hsk200_node_id, i+1);
            else
                idle_callback[0] = hsk200_assoc_report_check;
            return;
        }
    }

    for (i=0; i<HSK200_N_GROUPS; i++) {
        printf("INFO:rtt.c hsk200_assoc_report_check: group %d: ", i+1);
        for (j=0; j<HSK200_N_ASSOC_IN_GROUP; j++) {
            if (hsk200_assoc[i*HSK200_N_ASSOC_IN_GROUP+j])
                printf("%d ", hsk200_assoc[i*HSK200_N_ASSOC_IN_GROUP+j]);
        }
        printf("\n");
    }
    hsk200_keymap_report_check(NULL);
}
void hsk200_macro_report_check(void * data)
{
    int delay_ms, ch, key;
    int addr;
    int i, j;
    for (i=0; i<(int)sizeof(ir_macro_flag); i++) {
        if (ir_macro_flag[i] != 0xff) {
            break;
        }
    }
    if (i != sizeof(ir_macro_flag)) {
        g_instance = 1;
        if (send_data_fin)
            zwavecmd_configuration_bulk_get(hsk200_node_id, 25+i/4, 8);
        else
            idle_callback[0] = hsk200_macro_report_check;
        return;
    }
    for (i=0; i<MAX_SCENES*2; i++) {
        if (i<MAX_SCENES)
            printf("INFO:rtt.c hsk200_macro_report_check: scene %d:\n    ", i+1);
        else
            printf("INFO:rtt.c hsk200_macro_report_check: Leave scene %d:\n    ", i-MAX_SCENES+1);
        addr = i*MAX_IR_MACRO*MACRO_UNIT_SIZE;
        for (j=0; j<MAX_IR_MACRO; j++) {
            if ((ir_macro[addr+j*2]==0) && (ir_macro[addr+j*2+1]==0))
                break;
            delay_ms = ir_macro[addr+j*2];
            ch = (ir_macro[addr+j*2+1]>>6) | ((ir_macro[addr+j*2]&0x1)<<2);
            key = ir_macro[addr+j*2+1] & 0x3f;
            printf("(%d)[%d,%d] ", delay_ms, ch, key);
        }
        printf("\n");
    }
    hsk200_assoc_report_check(NULL);
}

void hsk200_dump_key(int ch, int key)
{
    int i;
    printf("INFO:rtt.c hsk200_dump_key: ch:%d key:%d\n", ch+1, key);
    for (i=0; i<80; i++) {
        printf("%02x ", hsk200_database[ch][key][i]);
        if (!((i+1)%16)) {
            printf("\n");
        }
    }
}
void hsk200_dump_keymap(void)
{
    int i, j, k;

    for (i=0; i<HSK200_N_CHANNEL; i++) {
        printf("INFO:rtt.c hsk200_dump_keymap: ch %d:\t", i+1);
        for (j=0; j<HSK200_N_KEY/8; j++) {
            for (k=0; k<8; k++) {
                printf("%d", (hsk200_keymap[i][j] & (1<<k)) ? 1 : 0);
            }
            printf(" ");
            if (!((j+1)%4))
                printf("\n");
        }
    }
}
void hsk200_get_key_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
    }
    else {
        hsk200_key_data_report_check(NULL);
    }
}
void hsk200_set_key_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        //register_senddata_ack_callback(hsk200_get_key_finished, NULL);
        //zwavecmd_configuration_bulk_get(hsk200_node_id, 21, 1);
        hsk200_keymap_flag[g_instance-1][g_cur_key] = 0x1;
    }
    //else {
    //	hsk200_key_data_report_check(NULL);
    //}
    hsk200_key_data_report_check(NULL);
}
void hsk200_key_data_report_check(void * data)
{
    int i, j;
    unsigned char buf[2];

    for (i=0; i<HSK200_N_CHANNEL; i++) {
        for (j=0; j<HSK200_N_KEY; j++) {
            if ((hsk200_keymap[i][j/8] & (1 << (j%8)))) {
                if (!(hsk200_keymap_flag[i][j] & 0x1)) {
                    g_instance = i+1;
                    g_cur_key = j;
                    buf[0] = 0;
                    buf[1] = j;
                    // set key number
                    if (send_data_fin) {
                        register_senddata_ack_callback(hsk200_set_key_finished, NULL);
                        zwavecmd_configuration_bulk_set_buf(hsk200_node_id, 21, buf, 2);
                    }
                    else {
                        idle_callback[0] = hsk200_key_data_report_check;
                    }
                    return;
                }
                else if (!(hsk200_keymap_flag[i][j] & 0x2)) {
                    if (g_cur_key != j) {
                        printf("WARNING:rtt.c hsk200_key_data_report_check: 1st ERROR! cur_key=%d j=%d\n", g_cur_key, j);
                        hsk200_keymap_flag[i][j] = 0;
                        return;
                    }
                    g_instance = i+1;
                    if (send_data_fin)
                        zwavecmd_configuration_bulk_get(hsk200_node_id, 1, 10);
                    idle_callback[0] = hsk200_key_data_report_check;
                    return;
                }
                else if (!(hsk200_keymap_flag[i][j] & 0x4)) {
                    if (g_cur_key != j) {
                        printf("WARNING:rtt.c hsk200_key_data_report_check: 2nd ERROR! cur_key=%d j=%d\n", g_cur_key, j);
                        hsk200_keymap_flag[i][j] = 0;
                        return;
                    }
                    g_instance = i+1;
                    if (send_data_fin)
                        zwavecmd_configuration_bulk_get(hsk200_node_id, 11, 10);
                    idle_callback[0] = hsk200_key_data_report_check;
                    return;
                }
                else if (!hsk200_backup_all) {
                    hsk200_dump_key(i, j);
                }
            }
        }
    }
    if (hsk200_backup_all) {
        hsk200_dump_keymap();
        hsk200_config_save("hsk200z.cfg");
    }
}
void hsk200_keymap_report_check(void * data)
{
    int i, j;
    for (i=0; i<HSK200_N_CHANNEL; i++) {
        for (j=0; j<HSK200_N_KEY/8; j++) {
            if (hsk200_keymap[i][j] != 0xff) {
                break;
            }
        }
        if (j==HSK200_N_KEY/8) {	// no key found (all 0xff)
            g_instance = i+1;
            if (send_data_fin)
                zwavecmd_configuration_bulk_get(hsk200_node_id, 23, HSK200_N_KEY/8);
            else
                idle_callback[0] = hsk200_keymap_report_check;
            return;
        }
    }
    hsk200_dump_keymap();
    if (hsk200_backup_all) {
        hsk200_key_data_report_check(NULL);
    }
}
void hsk200_conf_report_check(void * data)
{
    int i;
    for (i=0; i<HSK200_N_CONFIG; i++) {
        if (hsk200_config[i] == -1) {
            if (send_data_fin)
                zwavecmd_configuration_get(hsk200_node_id, i+1);
            else
                idle_callback[0] = hsk200_conf_report_check;
            return;
        }
    }
    printf("INFO:rtt.c hsk200_conf_report_check:\n");
    for (i=0; i<HSK200_N_CONFIG; i++) {
        printf("\tconfiguration %d : 0x%x\n", i+1, hsk200_config[i]);
    }
    hsk200_macro_report_check(NULL);
}
void hsk200_key_data_set_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        if (g_cur_conf == 21) {
            hsk200_keymap_flag[g_instance-1][g_cur_key] |= 0x1;
        }
        else if (g_cur_conf == 1) {
            hsk200_keymap_flag[g_instance-1][g_cur_key] |= 0x2;
        }
        else if (g_cur_conf == 11) {
            hsk200_keymap_flag[g_instance-1][g_cur_key] |= 0x4;
        }
    }
    hsk200_key_data_set_check(NULL);
}
void hsk200_assoc_remove_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        if (g_cur_conf < HSK200_N_GROUPS) {
            g_cur_conf++;
        }
    }
    hsk200_assoc_remove_check(NULL);
}
void hsk200_assoc_set_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        if (g_cur_conf < HSK200_N_GROUPS) {
            g_cur_conf++;
        }
    }
    hsk200_assoc_set_check(NULL);
}
void hsk200_conf_set_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        if (g_cur_conf < HSK200_N_CONFIG) {
            g_cur_conf++;
        }
    }
    hsk200_conf_set_check(NULL);
}
void hsk200_macro_set_finished(void * data, int txStatus)
{
    if (txStatus == TRANSMIT_COMPLETE_OK) {
        if (g_cur_conf < 57) {
            g_cur_conf += 8;
        }
    }
    hsk200_macro_set_check(NULL);
}
void hsk200_macro_set_check(void * data)
{
    if ((g_cur_conf >= 25) && (g_cur_conf < 57)) {
        g_instance = 1;
        //idle_callback[0] = hsk200_macro_set_check;
        register_senddata_ack_callback(hsk200_macro_set_finished, NULL);
        zwavecmd_configuration_bulk_set_buf(hsk200_node_id, g_cur_conf, ir_macro+(g_cur_conf-25)*4, 32);
        return;
    }
    printf("INFO:rtt.c hsk200_macro_set_check: IR ok\n");
    g_cur_conf = 0;
    hsk200_assoc_remove_check(NULL);
}
void hsk200_conf_set_check(void * data)
{
    if (g_cur_conf < HSK200_N_CONFIG) {
        g_instance = -1;
        //idle_callback[0] = hsk200_conf_set_check;
        register_senddata_ack_callback(hsk200_conf_set_finished, NULL);
        zwavecmd_configuration_set(hsk200_node_id, g_cur_conf+1, hsk200_config[g_cur_conf]);
        return;
    }
    g_cur_conf = 25;
    hsk200_macro_set_check(NULL);
}
void hsk200_restore(int id)
{
    int ret;

    txoptions |= TRANSMIT_OPTION_ACK;
    interval = 1000;
    hsk200_node_id = id;
    memset(hsk200_keymap_flag, 0, sizeof(hsk200_keymap_flag));
    memset(ir_macro_flag, 0, sizeof(ir_macro_flag));
    g_cur_conf = 0;

    ret = hsk200_config_load("hsk200z.cfg");
    if (ret) {
        printf("WARNING:rtt.c hsk200_restore: load hsk200z.cfg error\n");
        return;
    }
    hsk200_conf_set_check(NULL);
}
void hsk200_backup(int id)
{
    int i;

    txoptions |= TRANSMIT_OPTION_ACK;
    interval = 1000;
    hsk200_node_id = id;
    register_persistent_class_callback(COMMAND_CLASS_CONFIGURATION, hsk200_configuration_report_cb);
    register_persistent_class_callback(COMMAND_CLASS_ASSOCIATION, hsk200_association_report_cb);
    for (i=0; i<HSK200_N_CONFIG; i++)
        hsk200_config[i] = -1;
    memset(hsk200_keymap, 0xff, sizeof(hsk200_keymap));
    memset(hsk200_keymap_flag, 0, sizeof(hsk200_keymap_flag));
    memset(ir_macro, 0, sizeof(ir_macro));
    memset(ir_macro_flag, 0, sizeof(ir_macro_flag));
    memset(hsk200_assoc, 0xff, sizeof(hsk200_assoc));
    memset(hsk200_database, 0, sizeof(hsk200_database));
    hsk200_backup_all = 1;
    hsk200_conf_report_check(NULL);
}
void hsk200_get_key_map(int id)
{
    txoptions |= TRANSMIT_OPTION_ACK;
    interval = 1000;
    hsk200_node_id = id;
    register_persistent_class_callback(COMMAND_CLASS_CONFIGURATION, hsk200_configuration_report_cb);
    memset(hsk200_keymap, 0xff, sizeof(hsk200_keymap));
    memset(hsk200_keymap_flag, 0, sizeof(hsk200_keymap_flag));
    hsk200_backup_all = 0;
    hsk200_keymap_report_check(NULL);
}
void hsk200_get_key_data(int id, int ch, int key)
{
    txoptions |= TRANSMIT_OPTION_ACK;
    interval = 1000;
    hsk200_node_id = id;
    register_persistent_class_callback(COMMAND_CLASS_CONFIGURATION, hsk200_configuration_report_cb);
    memset(hsk200_keymap, 0x0, sizeof(hsk200_keymap));
    memset(hsk200_keymap_flag, 0, sizeof(hsk200_keymap_flag));
    memset(hsk200_database, 0, sizeof(hsk200_database));
    hsk200_keymap[ch][key/8] |= (1<<(key%8));
    hsk200_backup_all = 0;
    hsk200_key_data_report_check(NULL);
}


void zwavecmd_basic_get(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_BASIC, BASIC_GET,NULL, 0);
}

void version_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;

    if (pp[0] != VERSION_REPORT) {
        printf("WARNING:rtt.c version_dump: Unknown command %d\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c VERSION_REPORT:\n");
    printf("  Z-Wave library type: %d\n", pp[1]);
    printf("  Z-Wave protocol version: %d\n", pp[2]);
    printf("  Z-Wave protocol sub version: %d\n", pp[3]);
    printf("  Application version: %d\n", pp[4]);
    printf("  Application sub version: %d\n", pp[5]);
}
void zwavecmd_version_get(unsigned int id)
{
    zwave_sendClassCommand(id, COMMAND_CLASS_VERSION, VERSION_GET,NULL, 0);
    register_class_callback(COMMAND_CLASS_VERSION, version_dump, NULL);
}
void battery_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;

    if (pp[0] != BATTERY_REPORT) {
        printf("WARNING:rtt.c battery_dump: Unknown command %d\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c battery_dump: Battery Level is %d\n", pp[1]);
}
void zwavecmd_battery_get(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_BATTERY, BATTERY_GET,NULL, 0);
    register_class_callback(COMMAND_CLASS_BATTERY,battery_dump,NULL);

}
#define SENSOR_TYPE_Temperature  0x01
#define SENSOR_TYPE_General_purpose_value  0x02
#define SENSOR_TYPE_Luminance  0x03
#define SENSOR_TYPE_Power  0x04
#define SENSOR_TYPE_Relative_humidity  0x05
#define SENSOR_TYPE_Velocity  0x06
#define SENSOR_TYPE_Direction  0x07
#define SENSOR_TYPE_Atmospheric_pressure  0x08
#define SENSOR_TYPE_Barometric_pressure  0x09
#define SENSOR_TYPE_Solar_radiation  0x0A
#define SENSOR_TYPE_Dew_point  0x0B
#define SENSOR_TYPE_Rain_rate  0x0C
#define SENSOR_TYPE_Tide_level  0x0D
#define SENSOR_TYPE_Weight  0x0E
#define SENSOR_TYPE_Voltage  0x0F
#define SENSOR_TYPE_Current  0x10
#define SENSOR_TYPE_CO2_level  0x11
#define SENSOR_TYPE_Air_flow  0x12
#define SENSOR_TYPE_Tank_capacity  0x13
#define SENSOR_TYPE_Distance  0x14

char * get_sensor_type_string(int type)
{
    static char buf[32];

    if (type == SENSOR_TYPE_Temperature)
        return "Temperature";
    if (type == SENSOR_TYPE_Power)
        return "Power";
    if (type == SENSOR_TYPE_Voltage)
        return "Voltage";
    if (type == SENSOR_TYPE_Current)
        return "Current";
    if (type == SENSOR_TYPE_Luminance)
        return "Luminance";
    sprintf(buf, "(unknown sensor type %d)", type);
    return buf;
}
char * get_sensor_scale_string(int type, int scale)
{
    static char buf[32];

    if (type == SENSOR_TYPE_Temperature) {
        if (scale == 0)
            return "C";
        if (scale == 1)
            return "F";
    }
    if (type == SENSOR_TYPE_Power) {
        if (scale == 0)
            return "W";
        if (scale == 1)
            return "Btu/h";
    }
    if (type == SENSOR_TYPE_Voltage) {
        if (scale == 0)
            return "V";
        if (scale == 1)
            return "mV";
    }
    if (type == SENSOR_TYPE_Current) {
        if (scale == 0)
            return "A";
        if (scale == 1)
            return "mA";
    }
    if (type == SENSOR_TYPE_Luminance) {
        if (scale == 1)
            return "Lux";
    }
    sprintf(buf, "(unknown meter scale %d)", scale);
    return buf;
}
void multilevel_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int type,size,scale,precision,i;
    unsigned long val;
    unsigned long div = 1;
    //printf("xxxx payload=%x len=%d\n",payload,len);
    if (pp[0] != SENSOR_MULTILEVEL_REPORT) {
        printf("WARNING:rtt.c multilevel_dump: Impossible command %x\n", pp[0]);
        return;
    }
    type = pp[1];
    size = pp[2]&0x7;
    scale = (pp[2]>>3)&0x3;
    precision = (pp[2]>>5)&0x7;
    printf("INFO:rtt.c multilevel_dump: \n");
    printf("sensor type: 0x%02x (%s)\n", type, get_sensor_type_string(type));
    printf("precision: %d\n", precision);
    printf("scale: %d (%s)\n", scale, get_sensor_scale_string(type, scale));
    printf("size: %d\n", size);
    val = 0;
    for (i=0; i<size; i++)
        val = val<<8 | pp[3+i];
    for (i=0; i<precision; i++)
        div = div * 10;
    printf("val: %lu\n", val);
    printf("%s is %lu.%lu %s\n", get_sensor_type_string(type), val/div, val%div, get_sensor_scale_string(type, scale));
}
void multilevel_sensor_persistent_dump(int src, void *payload,int len)
{
    multilevel_dump(NULL,payload,len);
}

void thermostat_mode_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    if (pp[0] != THERMOSTAT_SETPOINT_REPORT) {
        printf("WARNING:rtt.c thermostat_mode_dump: Impossible command %x\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c thermostat_mode_dump:\n");
    switch(pp[1]) {
        case 0:
            printf("Mode is idle\n");
            break;
        case 1:
            printf("Mode is heat\n");
            break;
        case 2:
            printf("Mode is cool\n");
            break;
        case 3:
            printf("Mode is auto\n");
            break;
        case 4:
            printf("Mode is aux\n");
            break;
        case 5:
            printf("Mode is resume\n");
            break;
        case 6:
            printf("Mode is fan\n");
            break;
        case 7:
            printf("Mode is furance\n");
            break;
        case 8:
            printf("Mode is dryair\n");
            break;
        case 9:
            printf("Mode is moistair\n");
            break;
        case 10:
            printf("Mode is autochangeover\n");
            break;
        default:
            printf("Mode is unknown(%d)\n", pp[1]);
    }
}

void zwavecmd_thermostat_getmode(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_MODE, THERMOSTAT_MODE_GET,NULL, 0);
    register_class_callback(COMMAND_CLASS_THERMOSTAT_MODE,thermostat_mode_dump,NULL);
}

void zwavecmd_thermostat_setmode(unsigned int id,int mode)
{
    unsigned char buf[256];

    printf("mode is %d\n", mode);

    buf[0] = mode;
    zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_MODE, THERMOSTAT_MODE_SET,buf,1);
}

void multi_instance_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    if (pp[0] != THERMOSTAT_SETPOINT_REPORT) {
        printf("WARNING:rtt.c multi_instance_dump: Impossible command %x\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c multi_instance_dump: class %x has %d instance(s)\n", pp[1],pp[2]);
}

void zwavecmd_multi_instance_get(unsigned int id,unsigned class)
{
    unsigned char buf[255];

    buf[0] = class;
    zwave_sendClassCommand(id,COMMAND_CLASS_MULTI_INSTANCE_V2, MULTI_INSTANCE_GET_V2,buf,1);
    register_class_callback(COMMAND_CLASS_MULTI_INSTANCE,multi_instance_dump,NULL);
}

void multi_channel_get_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    if (pp[0] != MULTI_CHANNEL_END_POINT_REPORT_V2) {
        printf("WARNING:rtt.c multi_channel_get_dump: Impossible command %x\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c multi_channel_get_dump: \n");
    if (pp[1]&0x80) {
        printf("The number of channnels are dynamic\n");
    } else {
        printf("The number of channnels are fixed\n");
    }
    if (pp[1]&0x40) {
        printf("All channels are identical\n");
    } else {
        printf("Each channel has its own type\n");
    }
    printf("There are %d end points\n", pp[2]&0x7f);

}
void zwavecmd_multi_channel_end_point_get(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_MULTI_CHANNEL, MULTI_CHANNEL_END_POINT_GET_V2,NULL,0);
    register_class_callback(COMMAND_CLASS_MULTI_CHANNEL,multi_channel_get_dump,NULL);
}
void multi_channel_capability_get_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int i;
    if (pp[0] != MULTI_CHANNEL_CAPABILITY_REPORT_V2) {
        printf("WARNING:rtt.c multi_channel_capability_get_dump: Impossible command %x\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c multi_channel_capability_get_dump: \n");
    if (pp[1]&0x80) {
        printf("The number of channnels are dynamic\n");
    } else {
        printf("The number of channnels are fixed\n");
    }
    printf("end point %d\n", pp[1]&0x7f);
    printf("generic type is %x\n", pp[2]);
    printf("specific type is %x\n", pp[3]);
    printf("Supported command classes:\n");
    for(i=4;i<len-1;i++) {
        printf("\t%x\n", pp[i]);
    }
}
void zwavecmd_multi_channel_capability_get(unsigned int id,int endp)
{
    unsigned char buf[255];

    buf[0] = endp;
    zwave_sendClassCommand(id,COMMAND_CLASS_MULTI_CHANNEL, MULTI_CHANNEL_CAPABILITY_GET_V2,buf,1);
    register_class_callback(COMMAND_CLASS_MULTI_CHANNEL,multi_channel_capability_get_dump,NULL);
}
void multi_channel_find_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int i;
    if (pp[0] != MULTI_CHANNEL_END_POINT_FIND_REPORT_V2) {
        printf("WARNING:rtt.c multi_channel_find_dump: Impossible command %x\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c multi_channel_find_dump: matched end point for %d %d:\n",pp[2],pp[3]);
    for(i=4;i<len-1;i++) {
        printf("\t%x\n", pp[i]);
    }
    if (pp[1]) {
        register_class_callback(COMMAND_CLASS_MULTI_CHANNEL,multi_channel_find_dump,NULL);
    }

}
void zwavecmd_multi_channel_find(unsigned int id,int generic, int specific)
{
    unsigned char buf[255];

    buf[0] = generic;
    buf[1] = specific;
    zwave_sendClassCommand(id,COMMAND_CLASS_MULTI_CHANNEL, MULTI_CHANNEL_END_POINT_FIND_V2,buf,2);
    register_class_callback(COMMAND_CLASS_MULTI_CHANNEL,multi_channel_find_dump,NULL);
}

void thermostat_setpoint_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int size,scale,precision,i;
    double val;
    if (pp[0] != THERMOSTAT_SETPOINT_REPORT) {
        printf("WARNING:rtt.c thermostat_setpoint_dump: Impossible command %x\n", pp[0]);
        return;
    }
    size = pp[2]&7;
    scale = (pp[2]>>3)&3;
    precision = (pp[2]>>5)&3;
    val = 0;
    for(i=0;i<size;i++)
        val = val * 256 + pp[3+i];
    for(i=0;i<precision;i++)
        val = val /10;
    printf("INFO:rtt.c thermostat_setpoint_dump: type %d value %g\n", pp[1], val);
}
void zwavecmd_thermostat_getpoint(unsigned int id,int type)
{
    unsigned char buf[256];

    buf[0] = type;
    zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_SETPOINT, THERMOSTAT_SETPOINT_GET,buf,1);
    register_class_callback(COMMAND_CLASS_THERMOSTAT_SETPOINT,thermostat_setpoint_dump,NULL);
}

void zwavecmd_thermostat_setpoint(unsigned int id,int type,double value)
{
    unsigned char buf[256];
    int v = value*10;

    buf[0] = type;
    if (v < 127) {
        buf[1] = (1<<5) | (1<<3) | 1;
        buf[2] = v;
        zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_SETPOINT, THERMOSTAT_SETPOINT_SET,buf,3);
    } else if (v < 32768) {
        buf[1] = (1<<5) | (1<<3) | 2;
        buf[2] = v>>8;
        buf[3] = v%256;
        zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_SETPOINT, THERMOSTAT_SETPOINT_SET,buf,4);
    } else {
        buf[1] = (1<<5) | (1<<3) | 4;
        buf[2] = v>>24;
        buf[3] = (v>>16)%256;
        buf[4] = (v>>8)%256;
        buf[5] = v%256;
        zwave_sendClassCommand(id,COMMAND_CLASS_THERMOSTAT_SETPOINT, THERMOSTAT_SETPOINT_SET,buf,6);
    }
}



void zwavecmd_multilevel_get(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_SENSOR_MULTILEVEL, SENSOR_MULTILEVEL_GET,NULL, 0);
    register_class_callback(COMMAND_CLASS_SENSOR_MULTILEVEL,multilevel_dump,NULL);
}

void zwavecmd_multilevel_start_level_change(unsigned int id, unsigned int is_down)
{
    unsigned char buf[2];

    if (is_down)
        buf[0] = 0x60;
    else
        buf[0] = 0x20;
    buf[1] = 0;
    zwave_sendClassCommand(id,COMMAND_CLASS_SWITCH_MULTILEVEL, SWITCH_MULTILEVEL_START_LEVEL_CHANGE, buf, 2);
}

void zwavecmd_multilevel_stop_level_change(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_SWITCH_MULTILEVEL, SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE, NULL, 0);
}

#define METER_TYPE_ELECTRIC 0x1
#define METER_TYPE_GAS      0x2
#define METER_TYPE_WATER    0x3

char * get_meter_type_string(int type)
{
    static char buf[32];

    if (type == METER_TYPE_ELECTRIC)
        return "Electric meter";
    if (type == METER_TYPE_GAS)
        return "Gas meter";
    if (type == METER_TYPE_WATER)
        return "Water meter";
    sprintf(buf, "(unknown meter type %d)", type);
    return buf;
}
char * get_meter_scale_string(int type, int scale)
{
    static char buf[32];

    if (type == METER_TYPE_ELECTRIC) {
        if (scale == 0)
            return "kWh";
        if (scale == 1)
            return "kVAh";
        if (scale == 2)
            return "W";
        if (scale == 3)
            return "Pulse count";
    }
    if (type == METER_TYPE_GAS) {
        if (scale == 0)
            return "Cubic metersh";
        if (scale == 1)
            return "Cublic feet";
        if (scale == 3)
            return "Pulse count";
    }
    if (type == METER_TYPE_WATER) {
        if (scale == 0)
            return "Cubic metersh";
        if (scale == 1)
            return "Cublic feet";
        if (scale == 2)
            return "US gallons";
        if (scale == 3)
            return "Pulse count";
    }
    sprintf(buf, "(unknown meter scale %d)", scale);
    return buf;
}
void meter_monitor_dump(int src, void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int type,size,scale,precision,i;
    unsigned long val;
    unsigned long div = 1;
    if (pp[0] != METER_REPORT_V2) {
        return;
    }
    type = pp[1]&0x1f;
    size = pp[2]&7;
    scale = (pp[2]>>3)&3;
    precision = (pp[2]>>5)&7;
    printf("INFO:rtt.c meter_monitor_dump: \n");
    printf("Meter type: %s ", get_meter_type_string(type));
    printf("Rate type %x ", (pp[1]>>5)&0x3);
    printf("scale: %d ", scale);
    val = 0;
    for (i=0; i<size; i++)
        val = val<<8 | pp[3+i];
    for (i=0; i<precision; i++)
        div = div * 10;
    printf("val: %lu (%lu.%lu %s)\n", val, val/div, val%div, get_meter_scale_string(type, scale));

}
void meter_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int type,size,scale,precision,i;
    unsigned long val;
    unsigned long div = 1;
    if (pp[0] != METER_REPORT_V2) {
        return;
    }
    type = pp[1]&0x1f;
    size = pp[2]&7;
    scale = (pp[2]>>3)&3;
    precision = (pp[2]>>5)&7;
    printf("INFO:rtt.c meter_dump: \n");
    printf("Meter type: %s\n", get_meter_type_string(type));
    printf("Rate type %x\n", (pp[1]>>5)&0x3);
    printf("precision: %d\n", precision);
    printf("scale: %d\n", scale);
    printf("size: %d\n", size);
    val = 0;
    for (i=0; i<size; i++)
        val = val<<8 | pp[3+i];
    for (i=0; i<precision; i++)
        div = div * 10;
    printf("val: %lu (%lu.%lu %s)\n", val, val/div, val%div, get_meter_scale_string(type, scale));

    if (len < (3+size+2+1))
        return;
    printf("Delta time is %d\n", pp[3+size]*256+pp[3+size+1]);
    if (len < (3+size+2+size+1))
        return;
    val = 0;
    for(i=0;i<size;i++)
        val = val<<8 | pp[3+size+2+i];
    printf("previous val: %lu (%lu.%lu %s)\n", val, val/div, val%div, get_meter_scale_string(type, scale));
}

void zwavecmd_meter_get_v2(unsigned int id,int type)
{
    unsigned char buf[1];
    buf[0] = (type&7)<<3;

    zwave_sendClassCommand(id,COMMAND_CLASS_METER, METER_GET_V2,buf,1);
    register_class_callback(COMMAND_CLASS_METER,meter_dump,NULL);
}

void zwavecmd_basic_set(unsigned int id,unsigned char v)
{
    unsigned char buf[1];

    buf[0] = v;
    zwave_sendClassCommand(id,COMMAND_CLASS_BASIC, BASIC_SET,buf, 1);
}

void zwavecmd_indicator_set(unsigned int id,unsigned char v)
{
    unsigned char buf[1];

    buf[0] = v;
    zwave_sendClassCommand(id,COMMAND_CLASS_INDICATOR, INDICATOR_SET,buf, 1);
}
void zwavecmd_indicator_get(unsigned int id)
{
    zwave_sendClassCommand(id,COMMAND_CLASS_INDICATOR, INDICATOR_GET,NULL, 0);
}

struct setv_data{
    int id;
    int *pars;
    int *vals;
    int cur;
    int max;
};


void zwavecmd_configuration_set_next(void *d,int r)
{
    unsigned char buf[255];
    struct setv_data *data = (struct setv_data *) d;
    struct timeval to;
    if (r != 0) {
        printf("WARNING:rtt.c zwavecmd_configuration_set_next: Failed\n");
        return;
    }
    to.tv_sec = 0;
    to.tv_usec = 1000*interval;
    select(0,NULL,NULL,NULL,&to);
    if (data->cur == data->max) {
        zwavecmd_configuration_dump(data->id,5);
        return;
    }
    data->cur++;
    buf[0] =data->cur;
    buf[1] =data->cur;
    zwavecmd_configuration_set(data->id, data->pars[data->cur],data->vals[data->cur]);
    register_senddata_ack_callback(zwavecmd_configuration_set_next,(void *)data);
}
void zwavecmd_configuration_setv(unsigned int id, char *nos, char *vs)
{
    int i,n;
    static unsigned pars[10];
    static unsigned vals[10];
    static struct setv_data data;

    for(i=0;i<10;i++) {
        pars[i] = 0;
        vals[i] = 0;
        while(*nos&&*nos!=',') {
            pars[i] = pars[i]*10+(*nos-'0');
            nos++;
        }
        if (*nos)
            nos++;
        while(*vs&&*vs!=',') {
            vals[i]=vals[i]*10 + (*vs-'0');
            vs++;
        }
        if (*vs)
            vs++;
        if ((*nos==0)||(*vs==0)) break;
    }
    n = i;
    data.id = id;
    data.pars = (int*)pars;
    data.vals = (int*)vals;
    data.cur = 0;
    data.max = n;
    zwavecmd_configuration_set(id, pars[0],vals[0]);
    register_senddata_ack_callback(zwavecmd_configuration_set_next,&data);
}
void zwavecmd_configuration_bulk_clear(unsigned int id, unsigned int offset, int len)
{
    unsigned char buf[255];

    buf[0] = offset/256;
    buf[1] = offset % 256;
    buf[2] = len;
    buf[3] = 0x84;
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_BULK_SET_V2,buf,4);
}

void zwavecmd_configuration_bulk_set_buf(unsigned int id, unsigned int offset, unsigned char * data, int len)
{
    unsigned char buf[255];

    buf[0] = offset / 256;
    buf[1] = offset % 256;
    if ((len % 4) == 0) {
        buf[2] = len/4;
        buf[3] = 4;
    }
    else if ((len % 2) == 0) {
        buf[2] = len/2;
        buf[3] = 2;
    }
    else {
        buf[2] = len;
        buf[3] = 1;
    }
    memcpy(buf+4, data, len);
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_BULK_SET_V2, buf, len+4);
}
void zwavecmd_configuration_bulk_set(unsigned int id, unsigned int offset, char *argv[])
{
    unsigned char buf[255];
    int ptr;
    int v;

    buf[0] = offset/256;
    buf[1] = offset % 256;
    buf[2] = 0;
    buf[3] = 4;
    ptr = 3;
    while(*argv) {
        v = atoi(*argv);
        argv++;
        buf[ptr+1] = (v>>24) & 0xff;
        buf[ptr+2] = (v >> 16) & 0xff;
        buf[ptr+3] = (v >> 8) & 0xff;
        buf[ptr+4] = v & 0xff;
        ptr += 4;
        buf[2]++;
    }
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_BULK_SET_V2,buf,ptr+1);
}

void zwavecmd_configuration_bulk_get(unsigned int id, unsigned int offset,int len)
{
    unsigned char buf[255];
    static char data[16];

    buf[0] = offset/256;
    buf[1] = offset%256;
    buf[2] = len;
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_BULK_GET_V2,buf,3);
    register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_configuration_dump_next,data);

}


void zwavecmd_configuration_set4(unsigned int id, unsigned int no, int v)
{
    unsigned char buf[255];
    printf("INFO:rtt.c zwavecmd_configuration_set4: Set parameter %d to be %d\n",no,v);

    buf[0] = no;
    buf[1] = 4;
    buf[2] = v / (1<<24);
    buf[3] = (v >> 16) & 0xff;
    buf[4] = (v >> 8) & 0xff;
    buf[5] = v & 0xff;

    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_SET,buf,buf[1]+2);
}
void zwavecmd_configuration_set(unsigned int id, unsigned int no, int v)
{
    char buf[255];
    printf("INFO:rtt.c zwavecmd_configuration_set: Set parameter %d to be %d\n",no,v);

    buf[0] = no;
    if (v < 128 && v >= -128) {
        buf[1] = 1;
        buf[2] = v;
    } else if (v >= -32768 && v <32768) {
        buf[1] = 2;
        buf[2] = v / 256;
        buf[3] = v &0xff ;
    } else {
        buf[1] = 4;
        buf[2] = v / (1<<24);
        buf[3] = (v >> 16) & 0xff;
        buf[4] = (v >> 8) & 0xff;
        buf[5] = v & 0xff;
    }

    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_SET,(unsigned char *)buf,buf[1]+2);
}
void zwavecmd_configuration_get(unsigned int id,unsigned int no)
{
    unsigned char buf[255];

    buf[0] =no;
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_GET,buf,1);
}

void zwavecmd_configuration_dump_next(void *data,void *payload,int len)
{
    unsigned char buf[255];
    int *par = (int *) data;
    unsigned char *pp = (unsigned char *) payload;
    if (pp[0] == CONFIGURATION_REPORT) {
        if (par[1] == par[2]) return;
        par[2]++;
        buf[0] =par[2];
        zwave_sendClassCommand(par[0], COMMAND_CLASS_CONFIGURATION, CONFIGURATION_GET,buf,1);
        register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_configuration_dump_next,data);
    } else if (pp[0] == CONFIGURATION_BULK_REPORT_V2) {
        int offset = pp[1]*256+pp[2];
        int n = pp[3];
        int i;
        int size;

        if (pp[4] != 0) {
            register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_configuration_dump_next,data);
        }
        size = pp[5]&7;
        printf("INFO:rtt.c zwavecmd_configuration_dump_next: [%d] ", offset);
        for(i=0;i<n*size;i++) {
            printf("%02x ", pp[6+i]);
        }
        printf("\n");
    }
}

void zwavecmd_configuration_resend(void *data)
{
    int *par = (int *) data;
    unsigned char buf[255];
    int nowms;
    if (par[1] == par[2]) return;

    register_idle_callback(zwavecmd_configuration_resend,data);
    nowms = get_tick_ms();
    if (nowms < par[3]) return;
    par[3] = nowms+1000;


    buf[0] =par[2];
    zwave_sendClassCommand(par[0], COMMAND_CLASS_CONFIGURATION, CONFIGURATION_GET,buf,1);
    register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_configuration_dump_next,data);

}
void zwavecmd_configuration_dump(unsigned int id,int num)
{
    unsigned char buf[255];
    static int dumpdata[4];

    buf[0] =1;
    zwave_sendClassCommand(id, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_GET,buf,1);
    dumpdata[0] = id;
    dumpdata[1] = num;
    dumpdata[2] = 1;
    dumpdata[3] = get_tick_ms();
    register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_configuration_dump_next,dumpdata);
    register_idle_callback(zwavecmd_configuration_resend,dumpdata);
}

void zwavecmd_proprietary_learn_start(unsigned int id, int itemid, int keyid)
{
    unsigned char buf[256];
    buf[0] = 0x20;
    buf[1] = (itemid>>8)&0xff;
    buf[2] = (itemid)&0xff;
    buf[3] = (keyid>>8)&0xff;
    buf[4] = (keyid)&0xff;
    zwave_sendClassCommand(id,COMMAND_CLASS_PROPRIETARY, PROPRIETARY_SET,buf,5);
}
void zwavecmd_proprietary_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;
    if (pp[0] == PROPRIETARY_REPORT) {
        int i;
        printf("INFO:rtt.c zwavecmd_proprietary_dump: ");
        for(i=1;i<len;i++) {
            printf("%x ", pp[i]);
        }
        printf("\n");
    }
}

void zwavecmd_proprietary_get(unsigned int id, int offset, int size)
{
    unsigned char buf[256];
    buf[0] = 0x22;
    buf[1] = (offset>>8)&0xff;
    buf[2] = (offset)&0xff;
    buf[3] = size;
    buf[4] = 4;
    zwave_sendClassCommand(id,COMMAND_CLASS_PROPRIETARY, PROPRIETARY_GET,buf,5);
    register_class_callback(COMMAND_CLASS_CONFIGURATION,zwavecmd_proprietary_dump,NULL);
}

void zwavecmd_proprietary_set(unsigned int id, int offset, char *argv[])
{
    unsigned char buf[256];
    int l;
    buf[0] = 0x22;
    buf[1] = (offset>>8)&0xff;
    buf[2] = (offset)&0xff;
    buf[3] = 1;
    buf[4] = 4;
    l = 5;
    while(*argv) {
        char *endptr;
        int value = strtol(*argv, &endptr,16);
        buf[l] = (value>>24)&0xff;
        buf[l+1] = (value>>16)&0xff;
        buf[l+2] = (value>>8)&0xff;
        buf[l+3] = value&0xff;
        l += 4;
        argv++;
    }
    zwave_sendClassCommand(id,COMMAND_CLASS_PROPRIETARY, PROPRIETARY_SET,buf,l);
}

void zwavecmd_proprietary_set_raw(unsigned int id, char * args[])
{
    unsigned char buf[256];
    int i;
    for (i=0; args[i]; i++) {
        buf[i] = atoi(args[i]);
    }
    zwave_sendClassCommand(id,COMMAND_CLASS_PROPRIETARY, PROPRIETARY_SET,buf,i);
}

void zwavecmd_wakeup_dump(void *data, void *payload, int len)
{
    unsigned char *pp = (unsigned char *) payload;
    int v;

    if (pp[0] != WAKE_UP_INTERVAL_REPORT) {
        printf("WARNING:rtt.c zwavecmd_wakeup_dump: Unknown command(%x)\n", pp[0]);
        return;
    }
    v = (pp[1]<<16)+(pp[2]<<8)+pp[3];
    printf("INFO:rtt.c zwavecmd_wakeup_dump: Send wakeup notification to node %d at interval of %d\n", pp[4],v);
}


void zwavecmd_wakeup_get(unsigned int id)
{
    zwave_sendClassCommand(id, COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_GET,NULL,0);
    register_class_callback(COMMAND_CLASS_WAKE_UP, zwavecmd_wakeup_dump,NULL);
}
void zwavecmd_wakeup_nomore(unsigned int id)
{
    zwave_sendClassCommand(id, COMMAND_CLASS_WAKE_UP, WAKE_UP_NO_MORE_INFORMATION,NULL,0);
}


void zwavecmd_wakeup_set(unsigned int id,int v,int n)
{
    unsigned char buf[255];

    buf[0] = (v>>16)&0xff;
    buf[1] = (v>>8)&0xff;
    buf[2] = v&0xff ;
    buf[3] = n;
    zwave_sendClassCommand(id, COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_SET,buf,4);
}

void zwavecmd_actuator_dump(void *data, void *payload, int len)
{
    unsigned char *pp = (unsigned char *) payload;

    if (pp[0] != ACTUATOR_CONF_REPORT) {
        printf("WARNING:rtt.c zwavecmd_actuator_dump: Unknown command(%x)\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c zwavecmd_actuator_dump: Scene %d level %d duration %d\n", pp[1],pp[2],pp[3]);
}
void zwavecmd_actuator_get(unsigned int id,unsigned int scene)
{
    unsigned char buf[255];

    buf[0] = scene;
    zwave_sendClassCommand(id, COMMAND_CLASS_ACTUATOR_CONF, ACTUATOR_CONF_GET,buf,1);
    register_class_callback(COMMAND_CLASS_ACTUATOR_CONF, zwavecmd_actuator_dump,NULL);
}
void zwavecmd_actuator_set(unsigned int id,unsigned int scene,int level,int duration)
{
    unsigned char buf[255];

    buf[0] = scene;
    buf[1] = duration;
    if (level==-1) {
        buf[2] = 0;
        buf[3] = 0;
    } else {
        buf[2] = 0x80;
        buf[3] = level;
    }
    zwave_sendClassCommand(id, COMMAND_CLASS_ACTUATOR_CONF, ACTUATOR_CONF_SET,buf,4);
}
void zwavecmd_scene_conf_dump(void *data, void *payload, int len)
{
    unsigned char *pp = (unsigned char *) payload;

    if (pp[0] != SCENE_CONTROLLER_REPORT) {
        printf("WARNING:rtt.c zwavecmd_scene_conf_dump: Unknown command(%x)\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c zwavecmd_scene_conf_dump: group %d scene %d duration %d\n", pp[1],pp[2],pp[3]);
}

void zwavecmd_scene_controller_get(unsigned int id,unsigned int group)
{
    unsigned char buf[255];

    buf[0] = group;
    zwave_sendClassCommand(id, COMMAND_CLASS_SCENE_CONTROLLER, SCENE_CONTROLLER_GET,buf,1);
    register_class_callback(COMMAND_CLASS_SCENE_CONTROLLER, zwavecmd_scene_conf_dump,NULL);
}
void zwavecmd_scene_controller_set(unsigned int id,unsigned int group, unsigned int scene,unsigned int duration)
{
    unsigned char buf[255];

    buf[0] = group;
    buf[1] = scene;
    buf[2] = duration;

    zwave_sendClassCommand(id, COMMAND_CLASS_SCENE_CONTROLLER, SCENE_CONTROLLER_SET,buf,3);
}

static void zwavecmd_association_get(unsigned int id,unsigned int group)
{
    unsigned char buf[255];

    buf[0] = group;
    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_GET,buf,1);
}

void zwavecmd_association_set_buf(unsigned int id,unsigned int group, unsigned char * data, int len)
{
    unsigned char buf[255];

    buf[0] = group;
    memcpy(buf+1, data, len);

    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_SET, buf, len+1);
}
void zwavecmd_association_set(unsigned int id,unsigned int group, char *ids)
{
    unsigned char buf[255];
    int n,v;

    n = 1;
    v = 0;
    buf[0] = group;
    while(*ids) {
        if (*ids >='0' && *ids <='9') {
            v = v*10 + (*ids - '0');
        } else {
            buf[n++] = v;
            v = 0;
        }
        ids++;
    }
    if (v)
        buf[n++] = v;


    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_SET,buf,n);
}

void zwavecmd_association_remove_all(unsigned int id,unsigned int group)
{
    unsigned char buf[255];

    buf[0] = group;
    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_REMOVE,buf,1);
}
void zwavecmd_association_remove(unsigned int id,unsigned int group, char *ids)
{
    unsigned char buf[255];
    int n = 0,v = 0;

    if (*ids) {
        while(*ids) {
            if (*ids >='0' && *ids <='9') {
                v = v*10 + (*ids - '0');
            } else {
                buf[n++] = v;
                v = 0;
            }
            ids++;
        }
    }
    buf[0] = group;
    buf[1] = v;
    n = 2;

    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_REMOVE,buf,n);
}

void zwavecmd_association_group_get(unsigned int id)
{
    zwave_sendClassCommand(id, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_GROUP_GET,NULL,0);
}
void manufacture_dump(void *data,void *payload,int len)
{
    unsigned char *pp = (unsigned char *) payload;

    if (pp[0] != MANUFACTURER_SPECIFIC_REPORT) {
        printf("WARNING:rtt.c manufacture_dump: Unknown command %d\n", pp[0]);
        return;
    }
    printf("INFO:rtt.c MANUFACTURER_SPECIFIC_REPORT:\n");
    printf("  Manufacturer ID 1: 0x%02x\n", pp[1]);
    printf("  Manufacturer ID 2: 0x%02x\n", pp[2]);
    printf("  Product type ID 1: 0x%02x\n", pp[3]);
    printf("  Product type ID 2: 0x%02x\n", pp[4]);
    printf("  Product ID 1: 0x%02x\n", pp[5]);
    printf("  Product ID 2: 0x%02x\n", pp[6]);
}
void zwavecmd_manufacture_get(unsigned int id)
{
    zwave_sendClassCommand(id, COMMAND_CLASS_MANUFACTURER_SPECIFIC, MANUFACTURER_SPECIFIC_GET,NULL,0);
    register_class_callback(COMMAND_CLASS_MANUFACTURER_SPECIFIC, manufacture_dump, NULL);
}
int zlen;
unsigned char zdata[256];
char init_data_buf[256]={0};
char zwave_check_node_isfail=0;
int zdataptr;
int curcmd;

/** Returns true on success, or false if there was an error */
int SetSocketBlockingEnabled(int fd, int blocking)
{
    if (fd < 0) return 0;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? 1 : 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return 0;
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 1 : 0;
#endif
}

int zwave_init(void)
{
    fflush(stdout);
#ifndef _WIN32
    struct termios newtio;
#endif //_WIN32
    struct sockaddr_in server_addr;
    struct hostent *host;
    if (g_host) {
        if(verbose) printf("INFO:rtt.c zwave_init: is host\n");
        if((zwavefd=socket(AF_INET,SOCK_STREAM,0)) == -1) {
            perror("ERROR:rtt.c zwave_init: socket\n");
            return -1;
        }
        if((host=gethostbyname(g_host)) == NULL) {
            perror("ERROR:rtt.c zwave_init: gethostbyname\n");
            return -1;
        }

        server_addr.sin_family=AF_INET;
        server_addr.sin_port=htons(1001);
        server_addr.sin_addr=*((struct in_addr *)host->h_addr);
        //	int ret_val = SetSocketBlockingEnabled(zwavefd, 0);
        //	printf("blocking or not? %d\n", ret_val);

        if(connect(zwavefd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr)) < 0) {
            perror("ERROR:rtt.c zwave_init: connect\n");
            return -1;
        }
    } else {
#ifdef _WIN32
        printf("ERROR:rtt.c zwave_init: windows cannot handle %s\n", g_dev_name);
        return 0;
#else //_WIN32
        if(verbose) printf("INFO:rtt.c zwave_init: opening %s\n", g_dev_name);
        zwavefd = open(g_dev_name, O_RDWR | O_NOCTTY);
        if (zwavefd < 0) {
            printf("ERROR:rtt.c zwave_init: open %s\n", g_dev_name);
            return -1;
        }
        if (tcgetattr(zwavefd, &newtio) < 0) {
            printf("ERROR:rtt.c zwave_init: tcgetattr\n");
            return -1;
        }
        cfmakeraw(&newtio);
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);

        tcflush(zwavefd, TCIFLUSH);
        if (tcsetattr(zwavefd,TCSANOW,&newtio) < 0) {
            printf("ERROR:rtt.c zwave_init: tcsetattr\n");
            return -1;
        }
#endif //_WIN32
    }
    int PyZwave_print_debug_info_old = PyZwave_print_debug_info;
    PyZwave_print_debug_info = 0;
    clear_serial_api_queue();

    zwave_ready = 0;
    //if (SerialAPI_soft_reset()) {
    if (ZW_MemoryGetID()) {
        printf("ERROR:rtt.c zwave_init: Cannot init Z-Wave !\n");
        exit(1);
    }
    else {
        clear_serial_api_queue();
        if(verbose) printf("INFO:rtt.c zwave_init: ready.\n");
        //usleep(100*1000);	// wait z-wave ready after reset
    }
    PyZwave_print_debug_info = PyZwave_print_debug_info_old;

    return zwavefd;
}
char *toPowerLevel(int v)
{
    if (v == 0)
        return "normal";
    else if (v == 1)
        return "-1 db";
    else if (v == 2)
        return "-2 db";
    else if (v == 3)
        return "-3 db";
    else if (v == 4)
        return "-4 db";
    else if (v == 5)
        return "-5 db";
    else if (v == 6)
        return "-6 db";
    else if (v == 7)
        return "-7 db";
    else if (v == 8)
        return "-8 db";
    else if (v == 9)
        return "-9 db";
    return "unknown";
}

void capability_string(int v)
{
    if (v & 1)
        printf("Secondary ");
    if (v & 2)
        printf("other ");
    if (v & 4)
        printf("sis ");
    if (v & 8)
        printf("primary ");
    if (v & 0x10)
        printf("suc");
    if (v == 0)
        printf("old(id=0xef)");
    printf("\n");
}

void dumpSerialCapability(void)
{
    int i;
    printf("INFO:rtt.c Serial API capability:\n");
    printf("\tversion %d\n", zdata[0]);
    printf("\trevision %d\n", zdata[1]);
    printf("\tManufacture id %x %x\n", zdata[2],zdata[3]);
    printf("\tProduct type %x %x\n",zdata[4],zdata[5]);
    printf("\tProduct is %x %x\n", zdata[6],zdata[7]);
    printf("       0 1 2 3 4 5 6 7\n");
    for(i=8;i<zdataptr-1;i++) {
        int j;
        printf("%02x-%02x: ", (i-8)*8,(i-8)*8+7);
        for(j=0;j<8;j++)
            if (zdata[i]&(1<<j))
                printf("X ");
            else
                printf("O ");
        printf("\n");
    }
}
void dumpInitData(void)
{
    int len = 0;
    int i;
    printf("INFO:rtt.c dumpInitData: \n\tZdata is [");
    for(i=0;i<10;i++) {
        printf("%x ",zdata[i]);
    }
    printf("]\n\tVersion is %d.\n\t", zdata[0]);
    if (zdata[1]&1)
        printf("This is a slave with ");
    else {
        if (zdata[1]&4)
            printf("This is a secondary controller ");
        else
            printf("This is a primary controller ");
        if (zdata[1]&8)
            printf("with SUC enabled and ");
        else
            printf("with ");
    }
    if (zdata[1]&2)
        printf("Timer implemented\n");
    else
        printf("Timer unavailable\n");
    len=zdata[2];
    //init_data_buf, init_data_buf_ptr added for node discovery Sen 12.8.8
    fflush(stdout);
    int init_data_buf_ptr=1;
    for(i=0;i<len;i++) {
        bool printed = false;
        int mask = 1,j;
        // printf("%03d: ",i*8);
        for(j=0;j<8;j++) {
            if (zdata[3+i]&mask){
                if(!printed) {
                    printf("\tNode ids: ");
                    printed = true;
                }
                printf("%d ", i*8+j+1);
                init_data_buf[init_data_buf_ptr]=i*8+j+1;
                init_data_buf_ptr+=1;
            }
            mask <<=1;
        }
        if(printed) printf("\n\t");
    }
    printf("\n");
    init_data_buf[0]=init_data_buf_ptr-1;	//init_data_buf[0] stores the number of nodes(including self) in zwave
}
void dumpBasicType(int bt)
{
    printf("INFO:rtt.c dumpBasicType: %s\n", basic_type_string(bt));
}

void dumpGenericAndSpecificType(int gt,int st)
{
    char gtbuf[100];
    char stbuf[100];
    char *sst, *sgt;

    switch(gt) {
        case 0x03:
            sgt = "AV Control Point";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x04:
            sgt = "Display";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x040:
            sgt = "Entry Control(Lock)";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x1:
            sgt = "Generic Controller";
            if (st == 1)
                sst = "Portable remote cotnroller";
            else if (st == 2)
                sst = "Portable Scene controller";
            else if (st == 3)
                sst = "Portable Install Tool";
            else {
                sst = stbuf;
                snprintf(stbuf,sizeof(stbuf),"%d", st);
            }
            break;
        case 0x31:
            sgt = "Meter";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0xA1:
            sgt = "Sensor Alarm";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x21:
            sgt = "Sensor Mutilevel";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x2:
            sgt = "Static Controller";
            if (st == 1)
                sst = "Static remote cotnroller";
            else if (st == 2)
                sst = "Static Scene controller";
            else if (st == 3)
                sst = "Static Install Tool";
            else {
                sst = stbuf;
                snprintf(stbuf,sizeof(stbuf),"%d", st);
            }
            break;
        case 0x10:
            sgt = "Switch Binary";
            if (st == 1)
                sst = "Power Switch";
            else if (st == 3)
                sst = "Scewne Switch";
            else {
                sst = stbuf;
                snprintf(stbuf,sizeof(stbuf),"%d", st);
            }
            break;
        case 0x11:
            sgt = "Switch Multilevel";
            if (st == 1)
                sst = "Power Switch";
            else if (st == 3)
                sst = "Motor multiposition";
            else if (st == 4)
                sst = "Scene switch";
            else if (st == 5)
                sst = "Class A Motor";
            else if (st == 6)
                sst = "Class B Motor";
            else if (st == 7)
                sst = "Class C Motor";
            else {
                sst = stbuf;
                snprintf(stbuf,sizeof(stbuf),"%d", st);
            }
            break;
        case 0x12:
            sgt = "Switch Remote";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x13:
            sgt = "Switch Toggle";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        case 0x8:
            sgt = "ThermoStat";
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
        default:
            sgt = gtbuf;
            snprintf(stbuf,sizeof(gtbuf),"%d", gt);
            sst = stbuf;
            snprintf(stbuf,sizeof(stbuf),"%d", st);
            break;
    }
    printf("INFO:rtt.c dumpGenericAndSpecificType: GENERIC_TYPE is %s and SPECIFIC_TYPE is %s\n", sgt, sst);
}

void dumpNodeProtocolInfo(void)
{
    if (zdata[4] == 0)
        printf("WARNING:rtt.c dumpNodeProtocolInfo: node is not available\n");
    else {
        printf("INFO:rtt.c dumpNodeProtocolInfo:\n");
        if (zdata[0] & NODEINFO_LISTENING_SUPPORT)
            printf("\tlistening client\n");
        if (zdata[0] & NODEINFO_ROUTING_SUPPORT)
            printf("\trouting support\n");
        if (zdata[1] & NODEINFO_OPTIONAL_FUNC_SUPPORT)
            printf("\toptional function support\n");
        dumpBasicType(zdata[3]);
        dumpGenericAndSpecificType(zdata[4],zdata[5]);
		pyzwave_basic = zdata[3];
		pyzwave_generic = zdata[4];
		pyzwave_specific = zdata[5];
    }
}

void dump_node_info(unsigned char * buf, int len)
{
    int i;
    printf("INFO:rtt.c dump_node_info:\n");
    printf("  Basic Device Class = %02x\n", buf[0]);
    printf("  Generic Device Class = %02x\n", buf[1]);
    printf("  Specific Device Class = %02x\n", buf[2]);
    printf("  Command Classes:\n");
    for(i=3; i<len; i++) {
        printf("\t%s\n", cmd_class_string(buf[i]));
    }
}

void ApplicationCommandHandler(unsigned char * buf, int len)
{
    int rxStatus = buf[0];
    int src      = buf[1];
    //	int cmd_len  = buf[2];
    int class    = buf[3];
    int cmd      = buf[4];
    int delay;
    int i;
    //  printf("LENGTH======TMPNR====== %d %d\n", len, len-4-1);
    execute_class_callback(src, class, buf+4, len-4-1);
    printf("INFO:rtt.c ApplicationCommandHandler:\n");
    if (class == COMMAND_CLASS_BASIC) {
        if      (cmd == BASIC_SET) {
            printf("\tNode %d, BASIC_SET: %02x\n", src, buf[5]);
        }
        else if (cmd == BASIC_GET) {
            printf("\tNode %d, BASIC_GET\n", src);
        }
        else if (cmd == BASIC_REPORT) {
            printf("\tNode %d, BASIC_REPORT: %02x\n", src, buf[5]);
        }
        else if (cmd == 0xff) {
            printf("\tDebug: %02x %02x", buf[5],buf[6]);
        }
    }
    else if (class == COMMAND_CLASS_ASSOCIATION ) {
        if (cmd == ASSOCIATION_REPORT) {
            int gid = buf[5];
            int maxnod = buf[6];
            int nreport = buf[7];
            printf("\tAssociation report: group=%d, max_node=%d, report_left=%d\n", gid,maxnod,nreport);
            for(i=8;i<len-1;i++) {
                printf("   %d\n", buf[i]);
            }
        }
    }
    else if (class == COMMAND_CLASS_CONFIGURATION) {
        if (cmd == CONFIGURATION_REPORT) {
            int no = buf[5];
            int size = buf[6]&7;
            int v;

            if (size == 1) {
                v = buf[7];
                if (v > 0x7f)
                    v = -(0xff-v+1);
            } else if (size == 2) {
                v = (buf[7]<<8)+buf[8];
                if (v > 0x7fff)
                    v = -(0xffff-v+1);
            } else if (size == 4) {
                v = (buf[7]<<24)+(buf[8]<<16)+(buf[9]<<8)+buf[10];
                if ( v > 0x7fffffff)
                    v = -(0xffffffff-v+1);
            }
            printf("\tParameter %d = %d\n", no,v);
        }
    }
    else if (class == COMMAND_CLASS_WAKE_UP) {
        if (cmd == WAKE_UP_NOTIFICATION) {
            printf("\tnode %d is wakeup\n", src);
        }
    }
    else if (class == COMMAND_CLASS_SIMPLE_AV_CONTROL) {
        if (cmd == SIMPLE_AV_CONTROL_LEARN_REPORT) {
            printf("\tLearn feedback is %c\n", buf[5]);
        } else if (cmd == SIMPLE_AV_CONTROL_RAW_SET) {
            usleep(50*1000);
            zwavecmd_simple_av_send_raw(0,NULL);
        }
    }
    else if (class == COMMAND_CLASS_MULTI_INSTANCE) {
        if (cmd == MULTI_INSTANCE_CMD_ENCAP_V2) {
            printf("\tsrc=%d dest=%d\n", buf[5],buf[6]);
            g_instance_src = buf[5];
            g_instance_dst = buf[6];
            execute_class_callback(src, buf[7], buf+8,len-8-1);
        }
    }
    else if (class == 0x20 && cmd == 0xff) {
        int k;
        printf("\tDebug: ");
        for(k=5;k<len-1;k++)
            printf("%02x ", buf[k]);
        printf("\n");
    }
    else {
        if (PyZwave_print_debug_info) {
            printf("\trxStatus = %d\n", rxStatus);
            printf("\tsrc_node = %d\n", src);
            printf("\tclass = %x\n", class);
            printf("\tcommand = %x\n", cmd);
            printf("\textra=");
            for(i=5;i<len;i++)
                printf("%x ", buf[i]);
            printf("\n");
        }
    }

    delay = get_tick_ms() - rtt_start_ms;
    if (abs(delay) < 5000) {
        total_delay += delay;
        total_count++;
        printf("\trtt time %d/%d ms\n", delay,total_delay/total_count);
    }
    rtt_start_ms = 0;
}

void dumpRouteInformation(void)
{
    int i,j,k;

    k=0;
    printf("INFO:rtt.c dumpRouteInformation: connected to [");
    for(i=0;i<29;i++) {
        for(j=0;j<8;j++)
            if (zdata[i]& (1<<j)) {
                init_data_buf[k+1] = i*8+j+1;
                printf("%d " , i*8+j+1);
                ++k;
            }
    }
    init_data_buf[0] = k;
    printf("]\n");
}

void zwave_check_state(unsigned char c)
{
    int i;
    unsigned char ack = ZW_ACK;
    static unsigned char cksum;

    fflush(stdout);
    if (verbose) printf("INFO:rtt.c zwave_check_state: cur state %d token %x\n", zstate,c);

    switch(zstate) {
        case WAIT_ACK:
            if (c == ZW_ACK) {
                zstate = WAIT_SOF;
                ack_got=1;
            }
            else if (c == ZW_NAK) {
                // The NAK frame is used to de-acknowlege an
                // unsuccessful transmission of a data frame.
                // Only a frame with a LRC checksum error is
                // de-acknowledged with a NAK frame.
                printf("WARNING:rtt.c zwave_check_state: [NAK] SerialAPI LRC checksum\n");
                zstate = WAIT_SOF;
            }
            else if (c == ZW_CAN) {
                // The CAN frame is used by the ZW to instruct
                // the host that a host transmitted data frame
                // has been dropped.
                printf("WARNING:rtt.c zwave_check_state: [CAN] SerialAPI frame is dropped by ZW\n");
                usleep(50*1000);
                zstate = WAIT_RETRANSMIT;
            }
            else if (c == ZW_SOF) {
                printf("WARNING:rtt.c zwave_check_state: WAIT_ACK: SerialAPI got SOF without ACK\n");
                zstate = WAIT_LEN;
            }
            else {
                printf("WARNING:rtt.c zwave_check_state: WAIT_ACK: SerialAPI got unexpected byte 0x%x\n", c);
            }
            break;
        case WAIT_SOF:
            if (c == ZW_SOF) {
                zstate = WAIT_LEN;
                ack_got=1;
            } else if (c == ZW_ACK) {
                printf("WARNING:rtt.c zwave_check_state: WAIT_SOF: SerialAPI got unknown ACK \n");
                ack_got = 1;
            } else if (c == ZW_CAN) {
                printf("WARNING:rtt.c zwave_check_state: WAIT_SOF: SerialAPI got CAN, we should wait for ACK\n");
                zstate = WAIT_RETRANSMIT;
            }
            else {
                printf("WARNING:rtt.c zwave_check_state: WAIT_SOF: SerialAPI got unexpected byte 0x%x\n", c);
            }
            break;
        case WAIT_LEN:
            zlen = c-2;
            //		printf("WAIT_LEN======TMPNR====== %d\n", zlen);
            cksum = 0xff;
            cksum ^= c;
            zstate = WAIT_TYPE;
            break;
        case WAIT_TYPE:
            if (c == 1)
                zstate = WAIT_COMMAND;	// response
            else if (c == 0)
                zstate = WAIT_REQUEST;	// request
            cksum ^= c;
            break;
        case WAIT_COMMAND:	// get the response command
            curcmd = c;
            zstate = WAIT_DATA;
            zdataptr=0;
            cksum ^= c;
            break;
        case WAIT_REQUEST:	// get the request command
            curcmd = c;
            zstate = WAIT_EOF;
            zdataptr = 0;
            cksum ^= c;
            break;
        case WAIT_DATA:		// get the data of response
            zlen--;
            zdata[zdataptr] = c;
            zdataptr++;
            if (zlen == 0) {
                if (c != cksum) {
                    printf("WARNING:rtt.c zwave_check_state: CRC!!! crc1=%x crc2=%x\n", c, cksum);
                }
                write(zwavefd, &ack,1);
                zstate = WAIT_SOF;
                if (curcmd == GetControllerCapability) {
                    printf("INFO:rtt.c zwave_check_state: controller capabiity is\n\t");
                    capability_string(zdata[0]);
                } else if (curcmd == GetSerialCapability) {
                    dumpSerialCapability();
                } else if (curcmd == GetNodeProtocolInfo) {
                    dumpNodeProtocolInfo();
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else if (curcmd == GetInitData) {
                    dumpInitData();
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else if (curcmd == GetRoutingInformation) {
                    dumpRouteInformation();
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else if (curcmd == ZW_SendData) {
                    if (zdata[0] == 0) {	// zdata[0] is RetVal
                        printf("INFO:rtt.c zwave_check_state: queue overflow\n");
                        usleep(10000);
                    } else if (zdata[0] == 1) {
                        //printf(" command accepted\n");
                    }
                } else if (curcmd == FUNC_ID_MEMORY_GET_ID) {
                    zwave_ready = 1;
                    printf("INFO:rtt.c zwave_check_state: HomeID=%02x%02x%02x%02x, My address=%x\n", zdata[0], zdata[1], zdata[2], zdata[3], zdata[4]);
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else if (curcmd == IsFailedNodeId) {
                    if(zdata[0]==1)
                        zwave_check_node_isfail=1;
                    else
                        zwave_check_node_isfail=0;
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else if (curcmd == RemoveFailedNodeId) {
                    if (PyZwave_print_debug_info) {
                        printf("DEBUG:rtt.c zwave_check_state: Get response for RemoveFailedNodeId [");
                        for(i=0;i<zdataptr;i++) {
                            printf("%x ", zdata[i]);
                        }
                        printf("]\n");
                    }
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                } else {
                    if (PyZwave_print_debug_info) {
                        printf("DEBUG:rtt.c zwave_check_state: Get response for command %x [", curcmd);
                        for(i=0;i<zdataptr;i++) {
                            printf("%x ", zdata[i]);
                        }
                        printf("]\n");
                    }
                }
            }
            else {
                cksum ^= c;
            }
            break;
        case WAIT_EOF:		// get the data of request
            zlen--;
            zdata[zdataptr] = c;
            zdataptr++;
            if (zlen == 0) {
                write(zwavefd, &ack,1);
                zstate = WAIT_SOF;
                if (curcmd == 4) {	// ApplicationCommandHandler
                    ApplicationCommandHandler(zdata, zdataptr);
                } else if (curcmd == 0x49) {	// ApplicationSlaveUpdate
                    if (PyZwave_print_debug_info) {
                        printf("DEBUG:rtt.c zwave_check_state: ApplicationSlaveUpdate: ");
                        printf("  rxStatus = %02x", zdata[0]);
                        printf("  src_node = %d", zdata[1]);
                        dump_node_info(zdata+3, zdata[2]);
                    }
                } else if (curcmd == RFPowerLevelGet) {
                    printf("INFO:rtt.c zwave_check_state: power level is %s\n", toPowerLevel(zdata[0]));
                } else if (curcmd == SendTestFrame) {
                    if (zdataptr == 2) {
                        printf("INFO:rtt.c zwave_check_state: Command is accepted retVal=%d\n", zdata[0]);
                    } else {
                        printf("INFO:rtt.c zwave_check_state: txStatus = %x\n", zdata[1]);
                    }
                } else if (curcmd == RequestNetworkUpdate) {
                    if (zdataptr== 2) {
                        printf("INFO:rtt.c zwave_check_state: Command is accepted retVal=%d\n", zdata[0]);
                    } else {
                        printf("INFO:rtt.c zwave_check_state: network update status is %d\n", zdata[1]);
                    }
                } else if (curcmd == ZW_SendData || curcmd == ZW_SendNodeInformation) {
                    if (zdata[1] == TRANSMIT_COMPLETE_OK)
                        cmd_succ=1;
                    else
                        cmd_succ=0;
                    if (PyZwave_print_debug_info) {
                        if (zdata[1] == TRANSMIT_COMPLETE_OK) {
                            printf("DEBUG:rtt.c zwave_check_state: Transmit complete ok.\n");
                        } else if (zdata[1] == TRANSMIT_COMPLETE_NO_ACK) {
                            printf("DEBUG:rtt.c zwave_check_state: Transmit complete NO_ACK.\n");
                        } else if (zdata[1] == TRANSMIT_COMPLETE_FAIL) {
                            printf("DEBUG:rtt.c zwave_check_state: TRANSMIT_COMPLETE_FAIL.\n");
                        } else {
                            printf("DEBUG:rtt.c zwave_check_state: TRANSMIT error(%d)\n", zdata[1]);
                        }
                    }
                    if (g_flood) {
                        ZW_sendNodeInformation(g_flood,txoptions);
                        printf("WARNING:rtt.c zwave_check_state: flood\n");
                    } else {
                        //printf("done\n");
                    }
                    if (irbuf_ptr > 0) {
                        usleep(50*1000);
                        zwavecmd_simple_av_send_raw(0,NULL);
                    }
                    if (curcmd == ZW_SendData) {
                        if (zdata[0] == zseq) {
                            send_data_fin = 1;
                        }
                        else {
                            printf("WARNING:rtt.c zwave_check_state: ACKed SEQ != last SEQ ? ack_seq=%d, last_seq=%d\n", zdata[0], zseq);
                        }
                        execute_senddata_ack_callback(zdata[1]);
                    }

                } else if ((curcmd == AddNodeToNetwork) || (curcmd == RemoveNodeFromNetwork)){
                    char * type;

                    if (curcmd == AddNodeToNetwork)
                        type = "Add";
                    else
                        type = "Remove";
                    /*
                       printf("Get command for %x\n [", curcmd);
                       for(i=0;i<zdataptr;i++) {
                       printf("%x ", zdata[i]);
                       }
                       printf("]\n");
                       */
                    char tmp[256];
                    if (zdata[1] == 1) {
                        printf("INFO:rtt.c zwave_check_state: %s: learn ready\t", type);
                        sprintf(tmp, "%s: learn ready", type);
                        strcat(current_status, "\n");
                        strcat(current_status, tmp);
                    } else if (zdata[1] == 2) {
                        printf("INFO:rtt.c zwave_check_state: %s: node found\n", type);
                        sprintf(tmp, "%s: node found\n", type);
                        strcat(current_status, "\n");
                        strcat(current_status, tmp);
                    } else if (zdata[1] == 3) {
                        printf("INFO:rtt.c zwave_check_state: %sing slave node.....\n", type);
                    } else if (zdata[1] == 4) {
                        printf("INFO:rtt.c zwave_check_state: %sing controller node.....\n", type);
                    } else if (zdata[1] == 5) {
                        printf("INFO:rtt.c zwave_check_state: %s: protocol done. Wait for replication\n", type);
                    } else if (zdata[1] == 6) {
                        printf("INFO:rtt.c zwave_check_state: %s: done.\n", type);
                        sprintf(tmp, "%s: done.", type);
                        strcat(current_status, "\n");
                        strcat(current_status, tmp);
                    } else if (zdata[1] == 7) {
                        printf("WARNING:rtt.c zwave_check_state: %s: failed.\n", type);
                        sprintf(tmp, "%s: failed.", type);
                        strcat(current_status, "\n");
                        strcat(current_status, tmp);
                    } else {
                        printf("%s: Unknown retval = %d\n", type, zdata[1]);
                    }
                    if (zdata[2])
                        printf("INFO:rtt.c zwave_check_state: node_id: %d\t", zdata[2]);
                        sprintf(tmp, "  node_id: %d", zdata[2]);
                        strcat(current_status, "\n");
                        strcat(current_status, tmp);
                    if (zdata[3]){
                        dump_node_info(zdata+4, zdata[3]);
                    }
                } else if (curcmd == FUNC_ID_ZW_CONTROLLER_CHANGE) {
                    if(verbose) {
                        printf("INFO:rtt.c zwave_check_state: Get command for ZW_CONTROLLER_CHANGE %x\t [", curcmd);
                        for(i=0;i<zdataptr;i++) {
                            printf("%x ", zdata[i]);
                        }
                        printf("]\n");
                    }
                }
                else if (curcmd == SetDefault) {
                    printf("INFO:rtt.c zwave_check_state: Z-Wave controller is back to factory default\n");
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                }
                else if (curcmd == SetLearnMode) {
                    printf("INFO:rtt.c zwave_check_state: Learn on/off done");
                    execute_senddata_ack_callback(TRANSMIT_COMPLETE_OK);
                }
                else {
                    if (PyZwave_print_debug_info) {
                        printf("DEBUG:rtt.c zwave_check_state: Get command for %x\t [", curcmd);
                        for(i=0;i<zdataptr;i++) {
                            printf("%x ", zdata[i]);
                        }
                        printf("]\n");
                    }
                }
            }
            break;
        case WAIT_INIT:
            if(verbose) printf("INFO:rtt.c zwave_check_state: WAIT_INIT\n");
            break;
        default:
            printf("WARNING:rtt.c zwave_check_state: Unknown state %d\n", zstate);
            break;
    }
    fflush(stdout);
}


void usage(void)
{
    printf("Z-wave unit test utility\n");
    printf("testrtt [-d dev_file] [<cmd> [args]]+\n");
    printf("    serial: Dump the supported command the Z-wave module\n");
    printf("    ack: Set the ACK option when we send the data\n");
    printf("    autoroute: Set the AUTOROUTE option when we send data\n");
    printf("    lowpower: Set the LOWPOWER option when we send data\n");
    printf("    nowait: Exit after command is sent\n");
    printf("    seq <seq>: Set the initial sequence number\n");
    printf("    repeat:\n");
    printf("        Send the BASIC or MULTILEVEL SWITCH copmmand repeatly. This is used to test the signal strength\n");
    printf("    interval <t>: Set the interval between packet to be <t> ms. This works with the repeat command\n");
    printf("    exit_time <t>: End this utility after <t> seconds.\n");
    printf("    returnroute: Set the RETURNROUTE option when we send data\n");
    printf("    basic set <id> <val> |get <id>\n");
    printf("	    Send BASIC_SET or BASIC_GET command\n");
    printf("    multilevel set <id> <val>| get <id>\n");
    printf("	    Send Multilevel Switch command\n");
    printf("    start_level_change <id> <is_down>\n");
    printf("	    Start to dim with specified direction\n");
    printf("    stop_level_change <id>\n");
    printf("	    Stop to dimming\n");
    printf("    scene set <id> <button> <scene id> | get <id> <button>\n");
    printf(" 	    Send SCENE_CONF command\n");
    printf("    association set <id> <group> <dest id>\n");
    printf("        Add <dest id> into <group>-th group\n");
    printf("    association get <id> <group>\n");
    printf("        Fetch the content of the <group>-th group\n");
    printf("    association group_get <id>\n");
    printf("        Return the number of supported group\n");
    printf("    association remove <id> <group> <dest id>\n");
    printf("	    Remove the <dest id> from <group>-th group\n");
    printf("    manufacture get <id>\n");
    printf("	    Get manufacture ID\n");
    printf("    configuration get <id> <no>\n");
    printf("        Get parameter #<no>\n");
    printf("    configuration set <id> <no> <value>\n");
    printf("        Set the parameter <no> to be <value>\n");
    printf("    configuration dump <id> <num>\n");
    printf("        Dump parameters from 1 to <num>\n");
    printf("    controller_change [start | stop]\n");
    printf("	    Add a controller and shift the primary role to the new controller\n");
    printf("    network add\n");
    printf("	    Enter the learning mode to learn a new device\n");
    printf("    network addhigh\n");
    printf("	    The same as 'network add'. However, the RF signal in the full power instead of reduced power\n");
    printf("    network update\n");
    printf("	    Refresh the routing table in the SUC\n");
    printf("    network stop\n");
    printf("   	    Leave the learning mode\n");
    printf("    network delete\n");
    printf("	    Enter the learning mode to delete a device.\n");
    printf("    nodeupdate <id>\n");
    printf("	    Update the routing table for node <id>\n");
    printf("    simpleav set <id> <itemid> <key> <seq>\n");
    printf("        Send IR command <key> for device <dev>\n");
    printf("    simpleav learn <key>\n");
    printf("        Learn an IR code at <key>\n");
    printf("    simpleav raw <file>\n");
    printf("        send raw IR data from <file>\n");
    printf("    hsk200_backup\n");
    printf("        backup/dump hsk200 settings\n");
    printf("    wakeup get <id>\n");
    printf("        get the wake up interval of node <id>\n");
    printf("    wakeup set <id> <v> <n>\n");
    printf("        set the wakeup notification sent to <n> at interval of <v>\n");
    printf("    wakeup nomore <id>\n");
    printf("	    tell the node <id> that it can enter the sleep mode now\n");
    printf("    battery get <id>\n");
    printf("        Get battery level of node <id>\n");
    printf("    controller type\n");
    printf("    controller SUC get\n");
    printf("    controller SUC set <id> <suc> <power> <cap>\n");
    printf("        Set the SUCNode ID of the controller\n");
    printf("        <suc>: 0 | 1\n");
    printf("        <power>: high | low\n");
    printf("        <cap>: SUC | SIS\n");
    printf("    controller enable <type>\n");
    printf("        Enable the controller as SUC or SIS\n");
    printf("        <type>: suc|sis|none\n");
    printf("    controller initdata\n");
    printf("        dump all node information in the controller's EEPROM\n");
    printf("    controller initnodeinfo <devmask> <generic> <specific> <class1> <class 2> ....\n");
    printf("        setup the node info frame for the controller\n");
    printf("    controller reset\n");
    printf("        reset all node information in controller's EEPROM\n");
    printf("    actuator set <id> <scene> <level> <duration>\n");
    printf("    actuator get <id> <scene>\n");
    printf("    route print <id>\n");
    printf("        dump the routing table for node <id>\n");
    printf("    thermostat setmode  <id> <mode>\n");
    printf("    thermostat getmode <id>\n");
    printf("    thermostat setpoint <id> <type> <value>\n");
    printf("    thermostat getpoint <id> <type>\n");
    printf("    sceneactivation set <id> <scene>\n");
    printf("    multichannel wuobject get <id>\n");
    printf("    multichannel capability get <id>\n");
    printf("    multichannel find <id> <generic> <specific>\n");
    printf("    proprietary learn <id> <itemid> <key>\n");
    printf("    proprietary get <id> <itemid> <key>\n");
    printf("    proprietary set_raw <id> <data>... \n");
    printf("    raw <id> <d0> <d1> ....\n");
    printf("    isfail <id>\n");
    printf("    removefail <id>\n");


}
void do_test(int id,int v,int loop)
{
    int succ=0,fail=0;
    struct timeval to;
    fd_set rs;
    int i;

    txoptions |= TRANSMIT_OPTION_ACK;

    for(i=0;i<loop;i++) {
        zwavecmd_basic_set(id,v);
        cmd_succ=-1;
        while(1) {
            char c;
            int n;

            FD_ZERO(&rs);
            FD_SET(zwavefd, &rs);
            to.tv_sec = 2;
            to.tv_usec = 0;
            if ((n=select(zwavefd+1,&rs,NULL,NULL, &to))<=0) {
                fail++;
                break;
            }
            if (FD_ISSET(zwavefd,&rs)) {
                int len=read(zwavefd,&c,1);
                if (len > 0) {
                    zwave_check_state(c);
                }
            }
            if (cmd_succ==1) {
                succ++;
                break;
            }
            if (cmd_succ==0) {
                fail++;
                break;
            }
        }
        printf("INFO:rtt.c do_test: succ=%d fail=%d\n", succ,fail);
    }
    if ((succ*100/(succ+fail)) >= 90) {	// > 90% successful
        main_ret = 0;
    }
    else {
        main_ret = 1;
    }
}
int process_cmd(int argc, char * argv[])
{
    int cmd = 0, id = 0, v = 0;
    int i;

    for(i=0;i<argc;i++) {
        if (strcmp(argv[i],"help")==0) {
            usage();
        }
        else if (strcmp(argv[i],"basic")==0) {
            if (i < argc-2) {
                if (strcmp(argv[i+1],"get")==0) {
                    if (i <= argc-3) {
                        cmd = BASIC_GET;
                        id = atoi(argv[i+2]);
                        zwavecmd_basic_get(id);
                        i += 2;
                    }
                } else if (strcmp(argv[i+1],"set")==0) {
                    if ( i <= argc-4) {
                        cmd = BASIC_SET;
                        id = atoi(argv[i+2]);
                        v = atoi(argv[i+3]);
                        zwavecmd_basic_set(id,v);
                        i += 3;
                    }
                }
            }
        } else if (strcmp(argv[i],"multilevel")==0) {
            if (i < argc-2) {
                if (strcmp(argv[i+1],"get")==0) {
                    if (i <= argc-3) {
                        cmd = BASIC_GET;
                        id = atoi(argv[i+2]);
                        zwavecmd_multilevel_get(id);
                        i += 2;
                    }
                } else if (strcmp(argv[i+1],"set")==0) {
                    if ( i <= argc-4) {
                        cmd = BASIC_SET;
                        id = atoi(argv[i+2]);
                        v = atoi(argv[i+3]);
                        //zwavecmd_multilevel_set(id,v);
                        i += 3;
                    }
                }
            }
        } else if (strcmp(argv[i],"removefail")==0) {
            ZW_removeFailedNodeId(atoi(argv[i+1]));
            i++;
        } else if (strcmp(argv[i],"isfail")==0) {
            ZW_isFailedNodeId(atoi(argv[i+1]));
            i++;
        } else if (strcmp(argv[i],"sceneactivation")==0) {
            if (strcmp(argv[i+1],"set")==0) {
                zwavecmd_scene_activation_set(atoi(argv[i+2]),atoi(argv[i+3]));
                i += 3;
            }
        } else if (strcmp(argv[i],"start_level_change")==0) {
            if (i < argc-2) {
                id = atoi(argv[i+1]);
                v = atoi(argv[i+2]);
                zwavecmd_multilevel_start_level_change(id,v);
                i += 3;
            }
        } else if (strcmp(argv[i],"stop_level_change")==0) {
            if (i < argc-1) {
                id = atoi(argv[i+1]);
                zwavecmd_multilevel_stop_level_change(id);
                i += 2;
            }
        } else if (strcmp(argv[i],"scene")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_scene_controller_get(atoi(argv[i+2]),atoi(argv[i+3]));
                i += 3;
            } else if (strcmp(argv[i+1],"set") == 0) {
                zwavecmd_scene_controller_set(atoi(argv[i+2]), atoi(argv[i+3]), atoi(argv[i+4]),atoi(argv[i+5]));
                i += 5;
            }
        } else if (strcmp(argv[i],"association")==0) {
            if (strcmp(argv[i+1],"set")==0) {
                zwavecmd_association_set(atoi(argv[i+2]), atoi(argv[i+3]),argv[i+4]);
                i += 4;
            } else if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_association_get(atoi(argv[i+2]),atoi(argv[i+3]));
                i += 3;
            } else if (strcmp(argv[i+1],"group_get")==0) {
                zwavecmd_association_group_get(atoi(argv[i+2]));
                i += 2;
            } else if (strcmp(argv[i+1],"remove")==0) {
                zwavecmd_association_remove(atoi(argv[i+2]),atoi(argv[i+3]), argv[i+4]);
                i +=4;
            } else if (strcmp(argv[i+1],"remove_all")==0) {
                zwavecmd_association_remove_all(atoi(argv[i+2]),atoi(argv[i+3]));
                i +=3;
            }
        } else if (strcmp(argv[i],"manufacture")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_manufacture_get(atoi(argv[i+2]));
                i+=2;
            }
        } else if (strcmp(argv[i],"version")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_version_get(atoi(argv[i+2]));
                i+=2;
            }
        } else if (strcmp(argv[i],"autoroute")==0) {
            txoptions |= TRANSMIT_OPTION_AUTO_ROUTE;
        } else if (strcmp(argv[i],"ack")==0) {
            txoptions |= TRANSMIT_OPTION_ACK;
        } else if (strcmp(argv[i],"noack")==0) {
            txoptions &= ~TRANSMIT_OPTION_ACK;
        } else if (strcmp(argv[i],"lowpower")==0) {
            txoptions |= TRANSMIT_OPTION_LOW_POWER;
        } else if (strcmp(argv[i],"returnroute")==0) {
            txoptions |= TRANSMIT_OPTION_RETURN_ROUTE;
        } else if (strcmp(argv[i],"nowait")==0) {
            nowait = 1;
        } else if (strcmp(argv[i],"repeat")==0) {
            repeat = 1;
        } else if (strcmp(argv[i],"norepeat")==0) {
            repeat = 0;
        } else if (strcmp(argv[i],"noprint")==0) {
            PyZwave_print_debug_info = 0;
        } else if (strcmp(argv[i],"print")==0) {
            PyZwave_print_debug_info = 1;
        } else if (strcmp(argv[i],"interval")==0) {
            interval = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "exit_time")==0) {
            exit_time = atoi(argv[++i]);
        } else if (strcmp(argv[i],"flood")==0) {
            ZW_sendNodeInformation(atoi(argv[i+1]),txoptions);
            g_flood = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i],"initnodeinfo")==0) {
            int devmask = atoi(argv[i+1]);
            int generic = atoi(argv[i+2]);
            int specific = atoi(argv[i+3]);
            ZW_ApplicationNodeInformation(devmask,generic,specific, argc-i-4, &argv[i+4]);
            break;
        } else if (strcmp(argv[i],"nodeinfo")==0) {
            ZW_sendNodeInformation(atoi(argv[i+1]),txoptions);
            i++;
        } else if (strcmp(argv[i],"meter")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                if (strcmp(argv[i+3],"watt")==0)
                    zwavecmd_meter_get_v2(atoi(argv[i+2]),4);
                else
                    zwavecmd_meter_get_v2(atoi(argv[i+2]),atoi(argv[i+3]));
                i+=3;
            } else if (strcmp(argv[i+1],"monitor")==0) {
                register_persistent_class_callback(COMMAND_CLASS_METER,meter_monitor_dump);
                i++;
            }
        } else if (strcmp(argv[i],"actuator")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                int id = atoi(argv[i+2]);
                int scene = atoi(argv[i+3]);
                i+=3;
                zwavecmd_actuator_get(id,scene);
            } else if (strcmp(argv[i+1],"set")==0) {
                int id = atoi(argv[i+2]);
                int scene = atoi(argv[i+3]);
                int level = atoi(argv[i+4]);
                int duration = atoi(argv[i+5]);
                i+=5;
                zwavecmd_actuator_set(id,scene,level,duration);
            }

        } else if (strcmp(argv[i],"powerlevel")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                i++;
                ZW_RFPowerLevelGet();
            } else if (strcmp(argv[i+1],"set")==0) {
                v = atoi(argv[i+2]);
                ZW_RFPowerLevelSet(v);
                i+=2;
            }
        } else if (strcmp(argv[i],"network") == 0) {
            if (strcmp(argv[i+1],"update")==0) {
                i++;
                ZW_RequestNetworkUpdate();
            }else if ((strcmp(argv[i+1],"add")==0) || (strcmp(argv[i+1],"addhigh")==0)) {
                i++;
                ZW_AddNodeToNetwork(ADD_NODE_ANY|ADD_NODE_OPTION_HIGH_POWER);
            }else if ((strcmp(argv[i+1],"addlow")==0)) {
                i++;
                ZW_AddNodeToNetwork(ADD_NODE_ANY);
            }else if ((strcmp(argv[i+1],"del")==0) || (strcmp(argv[i+1],"delhigh")==0) || (strcmp(argv[i+1],"delete")==0)) {
                i++;
                ZW_RemoveNodeFromNetwork(ADD_NODE_ANY|ADD_NODE_OPTION_HIGH_POWER);
            }else if ((strcmp(argv[i+1],"dellow")==0)) {
                i++;
                ZW_RemoveNodeFromNetwork(ADD_NODE_ANY);
            }else if ((strcmp(argv[i+1],"stop")==0) || (strcmp(argv[i+1],"stopadd")==0)) {
                i++;
                ZW_AddNodeToNetwork(ADD_NODE_STOP);
            }else if (strcmp(argv[i+1],"stopdel")==0) {
                i++;
                ZW_RemoveNodeFromNetwork(ADD_NODE_STOP);
            }
        } else if (strcmp(argv[i],"test")==0) {
            PyZwave_print_debug_info = 0;
            id = atoi(argv[i+1]);
            v = atoi(argv[i+2]);
            do_test(id,v,atoi(argv[i+3]));
            i+=3;
        } else if (strcmp(argv[i],"instance")==0) {
            g_instance = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i],"multiinstance")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_multi_instance_get(atoi(argv[i+2]),atoi(argv[i+3]));
                i+=3;
            }
        } else if (strcmp(argv[i],"multichannel")==0) {
            if (strcmp(argv[i+1],"wuobject")==0) {
                if (strcmp(argv[i+2],"get")==0) {
                    zwavecmd_multi_channel_end_point_get(atoi(argv[i+3]));
                    i += 3;
                }
            } else if (strcmp(argv[i+1],"capability")==0) {
                if (strcmp(argv[i+2],"get")==0) {
                    zwavecmd_multi_channel_capability_get(atoi(argv[i+3]),atoi(argv[i+4]));
                    i += 4;
                }

            } else if (strcmp(argv[i+1],"find")==0) {
                int id = atoi(argv[i+2]);
                int generic = atoi(argv[i+3]);
                int specific = atoi(argv[i+4]);
                zwavecmd_multi_channel_find(id,generic,specific);
                i += 4;
            }
        } else if (strcmp(argv[i],"type")==0) {
            ZW_Type_Library();
        } else if (strcmp(argv[i],"getprotocolstatus")==0) {
            ZW_GetProtocolStatus();
        } else if (strcmp(argv[i],"controller")==0) {
            if (strcmp(argv[i+1],"type")==0) {
                i++;
                ZW_GetControllerCapability();
            } else if (strcmp(argv[i+1],"nodeinfo")==0) {
                id = atoi(argv[i+2]);
                i+=2;
                ZW_RequestNodeInfo(id);
            } else if (strcmp(argv[i+1],"SUC")==0) {
                if (strcmp(argv[i+2],"set")==0) {
                    unsigned char id = atoi(argv[i+3]);
                    unsigned char suc = atoi(argv[i+4]);
                    unsigned char power,type = 0;
                    if (strcmp(argv[i+5],"high")==0) {
                        power = 0;
                    } else {
                        power = 1;
                    }
                    if (strcmp(argv[i+6],"suc")==0) {
                        type = 0;
                    } else if (strcmp(argv[i+6],"sis")==0) {
                        type = 1;
                    }
                    ZW_SetSUCNodeID(id,suc,power,type);
                    i += 7;
                } else if (strcmp(argv[i+2],"get")==0) {
                    ZW_GetSUCNodeID();
                    i+=3;
                }
            } else if (strcmp(argv[i+1], "enable")==0) {
                if (strcmp(argv[i+2],"suc")==0) {
                    ZW_EnableSUC(1,0);
                } else if (strcmp(argv[i+2],"sis")==0) {
                    ZW_EnableSUC(1,1);
                } else {
                    ZW_EnableSUC(0,0);
                }
                i+=3;
            } else if (strcmp(argv[i+1],"initdata")==0) {
                ZW_GetInitData();
                i++;
            } else if (strcmp(argv[i+1],"reset")==0) {
                ZW_SetDefault();
                i++;
            }
        } else if (strcmp(argv[i],"get_id")==0) {
            ZW_MemoryGetID();
        } else if (strcmp(argv[i],"reset")==0) {
            SerialAPI_soft_reset();
        } else if (strcmp(argv[i],"serial")==0) {
            ZW_GetSerialCapability();
        } else if (strcmp(argv[i],"verbose")==0) {
            verbose = 1;
        } else if (strcmp(argv[i],"getnodeinfo")==0) {
            id = atoi(argv[i+1]);
            i++;
            ZW_GetNodeProtocolInfo(id);
        } else if (strcmp(argv[i],"nodeupdate")==0) {
            id = atoi(argv[i+1]);
            i++;
            ZW_RequestNodeNeighborUpdate(id);
        } else if (strcmp(argv[i],"controller_change")==0) {
            if (strcmp(argv[i+1],"start")==0)
                ZW_ControllerChange(CONTROLLER_CHANGE_START);
            else if (strcmp(argv[i+1],"stop")==0)
                ZW_ControllerChange(CONTROLLER_CHANGE_STOP);
            i++;
        } else if (strcmp(argv[i],"learn")==0) {
            if (strcmp(argv[i+1],"on")==0)
                ZW_SetLearnMode(1);
            else
                ZW_SetLearnMode(0);
            i++;
        } else if (strcmp(argv[i],"simpleav")==0) {
            if (strcmp(argv[i+1],"set")==0) {
                int id = atoi(argv[i+2]);
                int itemid = atoi(argv[i+3]);
                int key = atoi(argv[i+4]);
                int seq = atoi(argv[i+5]);
                i+=5;
                zwavecmd_simple_av_set(id,itemid,key,seq);
            } else if (strcmp(argv[i+1],"learn")==0) {
                int id = atoi(argv[i+2]);
                int key = atoi(argv[i+3]);
                i+=3;
                zwavecmd_simple_av_learn(id,key);
            } else if (strcmp(argv[i+1],"raw")==0) {
                int id = atoi(argv[i+2]);
                zwavecmd_simple_av_send_raw(id,argv[i+3]);
                i += 3;
            }
        } else if (strcmp(argv[i],"hsk200_get_keymap")==0) {
            hsk200_get_key_map(atoi(argv[i+1]));
            i+=1;
        } else if (strcmp(argv[i],"hsk200_get_key")==0) {
            hsk200_get_key_data(atoi(argv[i+1]), atoi(argv[i+2])-1, atoi(argv[i+3]));
            i+=3;
        } else if (strcmp(argv[i],"hsk200_backup")==0) {
            hsk200_backup(atoi(argv[i+1]));
            i+=1;
        } else if (strcmp(argv[i],"hsk200_restore")==0) {
            hsk200_restore(atoi(argv[i+1]));
            i+=1;
        } else if (strcmp(argv[i],"raw")==0) {
            ZW_sendRaw(argv+i+1);
            break;
        } else if (strcmp(argv[i],"configuration")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_configuration_get(atoi(argv[i+2]), atoi(argv[i+3]));
                i += 3;
            } else if (strcmp(argv[i+1],"set4")==0) {
                zwavecmd_configuration_set4(atoi(argv[i+2]), atoi(argv[i+3]),atoi(argv[i+4]));
                i += 4;
            } else if (strcmp(argv[i+1],"set")==0) {
                zwavecmd_configuration_set(atoi(argv[i+2]), atoi(argv[i+3]),atoi(argv[i+4]));
                i += 4;
            } else if (strcmp(argv[i+1],"setv")==0) {
                zwavecmd_configuration_setv(atoi(argv[i+2]), argv[i+3],argv[i+4]);
                i += 4;
            } else if (strcmp(argv[i+1],"bulkset")==0) {
                zwavecmd_configuration_bulk_set(atoi(argv[i+2]), atoi(argv[i+3]), &argv[i+4]);
                break;
            } else if (strcmp(argv[i+1],"bulkset1")==0) {
                unsigned char data[256];
                int len;
                int val;
                int j;
                len = argc-i-4;
                for (j=0; j<len; j++) {
                    sscanf(argv[i+4+j], "%x", &val);
                    data[j] = val;
                }
                zwavecmd_configuration_bulk_set_buf(atoi(argv[i+2]), atoi(argv[i+3]), data, len);
                break;
            } else if (strcmp(argv[i+1],"bulkclear")==0) {
                zwavecmd_configuration_bulk_clear(atoi(argv[i+2]), atoi(argv[i+3]),atoi(argv[i+4]));
                i += 4;
            } else if (strcmp(argv[i+1],"bulkget")==0) {
                zwavecmd_configuration_bulk_get(atoi(argv[i+2]), atoi(argv[i+3]),atoi(argv[i+4]));
                i+=4;
            } else if (strcmp(argv[i+1],"dump")==0) {
                zwavecmd_configuration_dump(atoi(argv[i+2]),atoi(argv[i+3]));
                i += 3;
            }
        } else if (strcmp(argv[i],"wakeup")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_wakeup_get(atoi(argv[i+2]));
                i+=2;
            } else if (strcmp(argv[i+1],"set")==0) {
                zwavecmd_wakeup_set(atoi(argv[i+2]), atoi(argv[i+3]),atoi(argv[i+4]));
                i+=4;
            } else if (strcmp(argv[i+1],"nomore")==0) {
                zwavecmd_wakeup_nomore(atoi(argv[i+2]));
                i+=2;
            }
        } else if (strcmp(argv[i],"proprietary")==0) {
            if (strcmp(argv[i+1],"learn")==0) {
                zwavecmd_proprietary_learn_start(atoi(argv[i+2]), atoi(argv[i+3]), atoi(argv[i+4]));
                i += 4;
            } else if (strcmp(argv[i+1],"get")==0) {
                char *endptr;
                int l = strtol(argv[i+3],&endptr,16);
                zwavecmd_proprietary_get(atoi(argv[i+2]), l, atoi(argv[i+4]));
                i += 5;
            } else if (strcmp(argv[i+1],"set")==0) {
                char *endptr;
                int l = strtol(argv[i+3],&endptr,16);
                zwavecmd_proprietary_set(atoi(argv[i+2]), l, &argv[i+4]);
                break;
            } else if (strcmp(argv[i+1],"set_raw")==0) {
                zwavecmd_proprietary_set_raw(atoi(argv[i+2]), &argv[i+3]);
                break;
            }

        } else if (strcmp(argv[i],"indicator")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_indicator_get(atoi(argv[i+2]));
                i += 2;
            } else {
                zwavecmd_indicator_set(atoi(argv[i+2]),atoi(argv[i+3]));
                i += 3;
            }
        } else if (strcmp(argv[i],"battery")==0) {
            if (strcmp(argv[i+1],"get")==0) {
                zwavecmd_battery_get(atoi(argv[i+2]));
                i +=2;
            }
        } else if (strcmp(argv[i],"route")==0) {
            if (strcmp(argv[i+1],"print")==0) {
                ZW_GetRoutingInformation(atoi(argv[i+2]));
                i+=2;
            }
        } else if (strcmp(argv[i],"thermostat")==0) {
            if (strcmp(argv[i+1],"getmode")==0) {
                zwavecmd_thermostat_getmode(atoi(argv[i+2]));
                i += 2;
            } else if (strcmp(argv[i+1],"setmode")==0) {
                int m = 0;
                if (strcmp(argv[i+3],"off")==0)
                    m = 0;
                else if (strcmp(argv[i+3],"heat")==0)
                    m = 1;
                else if (strcmp(argv[i+3],"cool")==0)
                    m = 2;
                else if (strcmp(argv[i+3],"auto")==0)
                    m = 3;
                else if (strcmp(argv[i+3],"aux")==0)
                    m = 4;
                else if (strcmp(argv[i+3],"resume")==0)
                    m = 5;
                else if (strcmp(argv[i+3],"fan")==0)
                    m = 6;
                else if (strcmp(argv[i+3],"furance")==0)
                    m = 7;
                else if (strcmp(argv[i+3],"dryair")==0)
                    m = 8;
                else if (strcmp(argv[i+3],"moistair")==0)
                    m = 9;
                else if (strcmp(argv[i+3],"autochangeover")==0)
                    m = 10;

                zwavecmd_thermostat_setmode(atoi(argv[i+2]),m);
                i += 3;
            } else if (strcmp(argv[i+1],"setpoint")==0) {
                int type = 0;

                if (strcmp(argv[i+3],"heat")==0)
                    type = 1;
                else if (strcmp(argv[i+3],"cool")==0)
                    type = 2;
                else if (strcmp(argv[i+3],"furance")==0)
                    type = 7;
                else if (strcmp(argv[i+3],"dryair")==0)
                    type = 8;
                else if (strcmp(argv[i+3],"moistair")==0)
                    type = 9;
                else if (strcmp(argv[i+3],"autochangeover")==0)
                    type = 10;

                zwavecmd_thermostat_setpoint(atoi(argv[i+2]), type, atof(argv[i+4]));
                i+=4;

            } else if (strcmp(argv[i+1],"getpoint")==0) {
                int type = 0;

                if (strcmp(argv[i+3],"heat")==0)
                    type = 1;
                else if (strcmp(argv[i+3],"cool")==0)
                    type = 2;
                else if (strcmp(argv[i+3],"furance")==0)
                    type = 7;
                else if (strcmp(argv[i+3],"dryair")==0)
                    type = 8;
                else if (strcmp(argv[i+3],"moistair")==0)
                    type = 9;
                else if (strcmp(argv[i+3],"autochangeover")==0)
                    type = 10;

                zwavecmd_thermostat_getpoint(atoi(argv[i+2]), type);
                i+=3;
            }


        } else if (strcmp(argv[i],"q")==0) {
            printf("INFO:rtt.c process_cmd: End\n");
            return -1;
        } else {
            printf("INFO:rtt.c process_cmd: unknown token %s\n", argv[i]);
        }
    }
    repeat_cmd = cmd;
    repeat_nodeid = id;
    repeat_value = v;
    return 0;
}

int process_cmd_line(char * buf)
{
    char * argv[64];
    int argc = 0;
    int l;

    while (*buf == ' ')
        buf++;

    l = strlen(buf);
    if ((buf[l-1] == '\r') || (buf[l-1] == '\n'))
        buf[l-1] = '\0';
    if ((buf[l-2] == '\r') || (buf[l-2] == '\n'))
        buf[l-2] = '\0';

    while ((*buf!='\0') && (*buf!='\r') && (*buf!='\n')) {
        argv[argc++] = buf;
        buf = strchr(buf, ' ');
        if (!buf)
            break;
        *buf++ = '\0';
        while (*buf == ' ')
            buf++;
    }
    argv[argc] = NULL;

    return process_cmd(argc, argv);
}

#ifdef _WIN32
//DWORD WINAPI read_stdin_thread(void * data)
void read_stdin_thread(void * data)
{
    char buf[512];
    struct sockaddr_in remote;
    int sfd;
    int ret;

    if ((sfd=(int)socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("INFO:rtt.c read_stdin_thread: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        //return -1;
        return;
    }
    remote.sin_family = AF_INET;
    remote.sin_port = htons(server_port);
    remote.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sfd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0) {
        printf("INFO:rtt.c read_stdin_thread: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        //return -1;
        return;
    }

    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("INFO:rtt.c read_stdin_thread: fgets error\n");
            break;
        }
        if (write(sfd, buf, strlen(buf)+1) < 0) {
            printf("INFO:rtt.c read_stdin_thread: write error\n");
            break;
        }
    }
    close(sfd);
    printf("INFO:rtt.c read_stdin_thread: end of thread\n");
    _endthread();
    //return 0;
    return;
}
#endif //_WIN32


//// 20111025 Niels Reijers: for PyZwave
int PyZwave_bytesReceived = 0;
int PyZwave_src = 0;
unsigned char PyZwave_messagebuffer[1024];
int PyZwave_senddataAckReceived = 0;

void PyZwave_proprietary_class_cb(int src, void * payload, int len) {
    if (len>1024) {
        printf("INFO:rtt.c PyZwave_proprietary_class_cb: Received too much data. :-(");
        exit(0);
    }

    memcpy(PyZwave_messagebuffer, (unsigned char *)payload, len);
    PyZwave_bytesReceived = len;
    PyZwave_src = src;

    /*  int i;
        printf("Received %i bytes: ", len);
        for (i=0; i<len; i++)
        printf("[%x] ", PyZwave_messagebuffer[i]);
        printf("\n");*/
}

int PyZwave_init_usb(char *dev_name) {
    strcpy(g_dev_name, dev_name);
    txoptions |= TRANSMIT_OPTION_ACK + TRANSMIT_OPTION_AUTO_ROUTE;
    register_persistent_class_callback(COMMAND_CLASS_PROPRIETARY, PyZwave_proprietary_class_cb);
    if(verbose) printf("INFO:rtt.c PyZwave_init_usb: device name is %s\n", g_dev_name);
    return zwave_init();
}

int PyZwave_init(char *host) {
    g_host = host;
    txoptions |= TRANSMIT_OPTION_ACK + TRANSMIT_OPTION_AUTO_ROUTE;
    register_persistent_class_callback(COMMAND_CLASS_PROPRIETARY, PyZwave_proprietary_class_cb);
    if(verbose) printf("INFO:rtt.c PyZwave_init: host name is %s\n", g_host);
    return zwave_init();
}

int PyZwave_receiveByte(int wait_msec) {
    int n;
    unsigned char c;
    struct timeval to;
    fd_set rs;

    to.tv_sec = wait_msec/1000;
    to.tv_usec = 1000*(wait_msec%1000);

    FD_ZERO(&rs);
    FD_SET(zwavefd,&rs);
    n = select(zwavefd+1,&rs,NULL,NULL,&to);
    if (n == 0) {
        return 0;
    }
    else if (n < 0) {
        printf("ERROR:rtt.c PyZwave_receiveByte: select() error\n");
        exit(1);
    }
    n = (int)read(zwavefd, &c,1);
    if (n != 1) {
        printf("ERROR:rtt.c PyZwave_receiveByte: error! n=%d\n", n);
        exit(1);
    }
    zwave_check_state(c);
    return 1;
}
unsigned int zwave_my_address;
int PyZwave_receive(int wait_msec) {
    int tmpBytesReceived;

    while(PyZwave_bytesReceived == 0) {
        if (!PyZwave_receiveByte(wait_msec)) {
            break; // No data received.
        }
    }
    tmpBytesReceived = PyZwave_bytesReceived;
    PyZwave_bytesReceived = 0;
    return tmpBytesReceived;
}

void PyZwave_senddata_ack_cb(void * data, int txStatus)
{
    PyZwave_senddataAckReceived = txStatus;
}


int PyZwave_send(unsigned id,unsigned char *in,int len) {
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_senddata_ack_callback(PyZwave_senddata_ack_cb, NULL);
    int res = ZW_sendData(id, in, len);
    if (res != 0)
        return res;
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
    if (PyZwave_senddataAckReceived == TRANSMIT_COMPLETE_OK)
        return 0;
    else {
        printf("ERROR:rtt.c send: transmit failed %i\n", PyZwave_senddataAckReceived);
        return -1;
    }
}

//discover - Sen 12.8.8
void PyZwave_discover_ack_cb(void * data, int txStatus) //TODO: this function is not called, why??????!
{
    //int i=0;
    PyZwave_senddataAckReceived = txStatus;
    /*for(i=0;i<10;++i){
        //      init_data_buf[i]=zdata[i];
        printf("buf%d: %d", i, init_data_buf[i]);
    }
    printf("\n");*/

}

void PyZwave_get_init_data(void){
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    ZW_GetInitData();
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK){
            break; // Ack or error received.
        }
    }
}

unsigned long PyZwave_get_addr(void){
    unsigned long network_id = 0;
    int i;
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    ZW_MemoryGetID();
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
    for (i = 3; i >= 0; i--){
        network_id <<= 8;
        network_id |= zdata[i];
    }
    zwave_my_address = zdata[4];
    return network_id;
}

void PyZwave_discover(void){
    PyZwave_get_init_data();
    PyZwave_get_addr();
    if(verbose) printf("INFO:rtt.c PyZwave_discover: my zwave address is %d\n", zwave_my_address);
}

int PyZwave_hard_reset(void){
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    int ret = ZW_SetDefault();
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
    return ret;
}

int PyZwave_basic_set(unsigned int node_id, unsigned char value){
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    txoptions |= TRANSMIT_OPTION_ACK;//ack basic get
    repeat_cmd = BASIC_SET;
    zwavecmd_basic_set(node_id, value);
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
    if(PyZwave_senddataAckReceived == TRANSMIT_COMPLETE_OK)
        return 1;
    else
        return 0;
}

int PyZwave_is_node_fail(unsigned int node_id){
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    txoptions |= TRANSMIT_OPTION_ACK;//ack basic get
    repeat_cmd = BASIC_GET;
    zwavecmd_basic_get((unsigned int)node_id);
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }

    if(verbose) printf("INFO:rtt.c PyZwave_is_node_fail zwavecmd_basic_get %d for node id %d\n",PyZwave_senddataAckReceived,node_id);
    if(PyZwave_senddataAckReceived == TRANSMIT_COMPLETE_OK)
        return 0;//node still alive, no need to check fail

    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    ZW_isFailedNodeId(node_id);
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
    if(verbose) printf("INFO:rtt.c PyZwave_is_node_fail ZW_isFailed %d for node id %d\n",zwave_check_node_isfail,node_id);
    fflush(stdout);
    return (int)zwave_check_node_isfail;
}

void PyZwave_remove_fail(unsigned int node_id){
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    ZW_removeFailedNodeId((int)node_id);
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }
}

void PyZwave_check_all_remove_fail(void){
    int i;
    PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
    register_discover_callback(PyZwave_discover_ack_cb, NULL);
    ZW_GetInitData();
    while (1) {
        if (!PyZwave_receiveByte(1000)) {
            break; // No data received.
        }
        if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
            break; // Ack or error received.
    }

    for(i=2;i<=init_data_buf[0];i++) {
        int node_id = (unsigned)init_data_buf[i];
    	if(PyZwave_is_node_fail(node_id)) {
            PyZwave_remove_fail(node_id);
            if(verbose) printf("INFO:rtt.c PyZwave_is_node_fail ZW_removeFailed for node id %d\n",init_data_buf[i]);
        }
    }
}

// Penn
void PyZwave_routing(unsigned node_id) {
  PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
  register_discover_callback(PyZwave_discover_ack_cb, NULL);
  ZW_GetRoutingInformation(node_id);
  while (1) {
    if (!PyZwave_receiveByte(1000))
      break; // No data received.
    if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
      break; // Ack or error received.
  }
}

void PyZwave_get_device_type(unsigned node_id) {
  PyZwave_senddataAckReceived = TRANSMIT_WAIT_FOR_ACK;
  register_discover_callback(PyZwave_discover_ack_cb, NULL);
  ZW_GetNodeProtocolInfo(node_id);
  while (1) {
    if (!PyZwave_receiveByte(1000))
      break; // No data received.
    if (PyZwave_senddataAckReceived != TRANSMIT_WAIT_FOR_ACK)
      break; // Ack or error received.
  }
}

int PyZwave_zwavefd(void) {
    return zwavefd;
}

char *PyZwave_status(void) {
    return current_status;
}

void PyZwave_clearstatus(void) {
    memset(current_status, '\0', sizeof(current_status));
}

int main(int argc, char *argv[])
{
    struct timeval to;
    fd_set rs;
    int i;
    char c;
    int n;
    char cmd_buf[512];
#ifdef _WIN32
    HANDLE stdin_thread;
    WSADATA wsaData;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    int local_len;
    int remote_len;
    int sfd_listen;
    int sfd_commu;
#endif //_WIN32

    i = 1;
    repeat = 0;
    zseq = time(NULL) % 255;

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("ERROR:rtt.c main: WSAStartup error\n");
        return -1;
    }
#endif //_WIN32
    /*
       if (argc <= 1) {
       usage();
       exit(0);
       }
       */

    if (argv[i] && !strcmp(argv[i], "-d")) {
        if (argv[i+1]) {
            strcpy(g_dev_name, argv[i+1]);
            i++;
        }
        i++;
        txoptions |= TRANSMIT_OPTION_ACK + TRANSMIT_OPTION_AUTO_ROUTE;
        register_persistent_class_callback(COMMAND_CLASS_PROPRIETARY, PyZwave_proprietary_class_cb);
    }
    if (argv[i] && !strcmp(argv[i],"host")) {
        g_host = argv[i+1];
        i+=2;
        register_persistent_class_callback(COMMAND_CLASS_SENSOR_MULTILEVEL,multilevel_sensor_persistent_dump);
    }

    if ((argc-i) <= 0)
        usage();

    if (zwave_init() < 0) {
        printf("ERROR:rtt.c main: can not open zwave device\n");
        exit(-1);
    }


#ifdef _WIN32
    ///////////// create command line thread
    if ((sfd_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("INFO:rtt.c main: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        exit(-1);
    }
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    local.sin_port = htons(0);
    if (bind(sfd_listen, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) < 0) {
        printf("ERROR:rtt.c main: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        exit(-1);
    }
    if (getsockname(sfd_listen, (struct sockaddr *)&local, &local_len) < 0) {
        printf("ERROR:rtt.c main: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        exit(-1);
    }
    server_port = ntohs(local.sin_port);
    printf("SERVER_PORT: %d\n", server_port);

    listen(sfd_listen, 5);

    //stdin_thread = CreateThread(NULL, 0, read_stdin_thread, NULL, 0, NULL);
    stdin_thread = (HANDLE)_beginthread(read_stdin_thread, 0, NULL);

    if ((sfd_commu=(int)accept(sfd_listen, (struct sockaddr *)&remote, &remote_len)) < 0) {
        printf("ERROR:rtt.c main: %s:%d ERRNO=%d (%s)\n", __FUNCTION__, __LINE__, ERRNO, strerror(ERRNO));
        exit(-1);
    }
    printf("INFO:rtt.c main: Accept connection from %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
#endif //_WIN32

    process_cmd(argc-i, &argv[i]);

    to.tv_sec = interval/1000;
    to.tv_usec = (interval%1000)*1000;

    while(1) {
        FD_ZERO(&rs);
        FD_SET(zwavefd, &rs);
#ifdef _WIN32
        FD_SET(sfd_commu, &rs);
#else //_WIN32
        FD_SET(STDIN_FILENO, &rs);
#endif //_WIN32


        n = select(FD_SETSIZE,&rs,NULL,NULL, &to);
        if (n < 0) {
            printf("ERROR:rtt.c main: Z-Wave device file is closed !\n");
            break;
        }
        else if (n == 0) {	// timeout
            if (nowait)
                exit(main_ret);

            to.tv_sec = interval/1000;
            to.tv_usec = (interval%1000)*1000;

            execute_idle_callback();

            if (repeat) {
                if (repeat_cmd == BASIC_SET) {
                    repeat_value = ((repeat_value==0)? 255:0);
                    zwavecmd_basic_set(repeat_nodeid, repeat_value);
                } else {
                    if (rtt_start_ms != 0) {
                        printf("X\n");
                    } else {
                        printf("\7");
                        repeat_value = ((repeat_value==0)? 255:0);
                        zwavecmd_basic_set(repeat_nodeid, repeat_value);
                        fflush(stdout);
                    }
                    rtt_start_ms = get_tick_ms();
                    zwavecmd_basic_get(repeat_nodeid);
                }
            }
            continue;
        }
#ifdef _WIN32
        if (FD_ISSET(sfd_commu, &rs)) {
            DWORD ret;
            read(sfd_commu, cmd_buf, sizeof(cmd_buf));
            if (process_cmd_line(cmd_buf)) {
                fclose(stdin);
                WaitForSingleObject(stdin_thread, INFINITE);
                break;
            }
        }
#else //_WIN32
        if (FD_ISSET(STDIN_FILENO, &rs)) {
            fgets(cmd_buf, sizeof(cmd_buf), stdin);
            if (process_cmd_line(cmd_buf)) {
                break;
            }
        }
#endif //_WIN32
        if (FD_ISSET(zwavefd,&rs)) {
            int len=read(zwavefd,&c,1);
            if (len > 0) {
                //printf("%x\n",c);
                zwave_check_state(c);
            }
        }
    }
    printf("INFO:rtt.c main: end\n");
    return 0;
}
