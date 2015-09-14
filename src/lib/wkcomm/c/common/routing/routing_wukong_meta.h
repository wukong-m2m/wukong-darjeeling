#ifdef ROUTING_USE_WUKONG_META

#define MPTN_ID_LEN                 4

#define MPTN_UUID_LEN              16


#define MPTN_MSGTYPE_LEN            1
#define MPTN_DEST_BYTE_OFFSET       0
#define MPTN_SRC_BYTE_OFFSET        MPTN_DEST_BYTE_OFFSET + MPTN_ID_LEN
#define MPTN_MSGTYPE_BYTE_OFFSET    MPTN_SRC_BYTE_OFFSET + MPTN_ID_LEN
#define MPTN_PAYLOAD_BYTE_OFFSET    MPTN_MSGTYPE_BYTE_OFFSET + MPTN_MSGTYPE_LEN

#define MPTN_MSGTYPE_GWDISCOVER     0
#define MPTN_MSGTYPE_GWOFFER        1
#define MPTN_MSGTYPE_IDREQ          2
#define MPTN_MSGTYPE_IDACK          3

#define NORMAL_MODE 0
#define GATEWAY_DISCOVERY_MODE  1
#define ID_REQ_MODE 2

#define IP_ADDRSTRLEN 16

void routing_handle_message(uint8_t *payload, uint8_t length);
//void routing_handle_message(wkcomm_address_t wkcomm_addr, uint8_t *payload, uint8_t length);
void routing_poweron_init();
void routing_id_req();
void routing_discover_gateway();

///////////////////////////////////////////////////////////////

struct SORT_GATE{
        uint8_t index;
        uint8_t single_gate_succ_pct;
        uint8_t single_path_succ_pct;
};
typedef struct SORT_GATE SG;

#define LEN_OF_DID              4
#define BROADCAST_PERIOD        20000
#define GATE_NUM                5
#define RECENT_DEST_NUM         1
#define RECENT_DEST_WOG_NUM     1
#define RWT_NUM                 3
#ifdef RADIO_USE_ZWAVE
#define LEN_OF_ZWAVE_SUBNET     1
#endif
#ifdef RADIO_USE_WIFI_ARDUINO
#define RELI_TESTER_NUM         3
#define LEN_OF_WIFI_SUBNET      4
#endif

#ifdef RADIO_USE_ZWAVE
#define GET_ID_PREFIX_ZWAVE(x)  (x & 0xFFFFFF00)
#define GET_ID_RADIO_ADDRESS_ZWAVE(x)   (x & 0x000000FF)
#endif
#define LEFTEST_BYTE(x)                 ((x >> 24) & 0xFF) 

void routing_wukong_meta_send(uint32_t dest, uint32_t time, uint8_t reli, uint8_t* payload, uint8_t length);
void routing_wukong_meta_send_2(uint32_t dest, uint32_t pre_addr, unsigned char* used_nets, uint32_t time, uint8_t reli, 
				uint32_t time_to_dest, uint8_t* payload, uint8_t length);
bool check_i_have_same_radio_as_dest(uint32_t dest);
void send(uint32_t addr, uint32_t dest, unsigned char* used_nets, uint32_t time, uint8_t reli, uint32_t time_to_dest,
          uint8_t* payload, uint8_t length);
bool check_gate_same_radio_as_dest(uint32_t dest);
bool check_gate_ready(bool gate_same_radio_as_dest, uint32_t dest, unsigned char* used_nets);
bool ckeck_i_have_same_radio_as_dest(uint32_t dest);
void check_time_to_dest(uint32_t dest, uint32_t pre_addr, uint32_t time_to_dest);
void get_ready_gate_indexs_by_check_used_nets(uint32_t dest, bool gate_same_radio_as_dest, uint8_t* ready_gate_indexs, unsigned char* used_nets);
void calculate_and_sort_ready_gates_from_max_reli_to_min_reli(uint32_t dest, uint8_t* ready_gate_indexs, uint32_t time,
								bool gate_same_radio_as_dest, SG* sortGate);
void select_forwarding_gates(bool* succ, uint8_t* succ_pct, uint8_t* fail_pct, SG* sortGate, uint8_t reli, uint8_t* last_index_in_sort_gate);
void send_reli_with_time_restrict_msg(uint32_t dest, uint32_t pre_addr, uint32_t time, uint8_t succ_pct);
uint32_t qos_time_test(uint32_t addr);
#ifdef RADIO_USE_ZWAVE
uint8_t qos_reli_test_zwave(uint32_t addr);
#endif
#ifdef RADIO_USE_WIFI_ARDUINO
uint32_t qos_reli_test_start_wifi(uint32_t addr);
uint32_t qos_reli_test_end_wifi(uint32_t addr);
void qos_reli_test_wifi(uint32_t addr, uint8_t gate_reli_tester_index);
#endif
#ifdef RADIO_USE_ZWAVE
void routing_handle_zwave_message(radio_zwave_address_t zwave_addr, uint8_t *payload, uint8_t length);
#endif
#ifdef RADIO_USE_WIFI_ARDUINO
void routing_handle_wifi_message(uint32_t wifi_addr, uint8_t *payload, uint8_t length);
#endif
void routing_handle_message_normal_node(uint8_t *payload, uint8_t length);
//void routing_handle_message(uint8_t *payload, uint8_t length);
void variable_init();
void broadcast_gate_addr();
void periodic_broadcast();
void periodic_check_for_qos_measure();
void periodic_check_recent_dest_to_retrieve_qos_measure();
void periodic_qos_remeasure();
void periodic_check_node_failure();
//The following are tool functions
#ifdef RADIO_USE_ZWAVE
radio_zwave_address_t addr_wkcomm_to_zwave(uint32_t wkcomm_addr);
uint32_t addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr);
#endif
#ifdef RADIO_USE_WIFI_ARDUINO
bool find_match_in_reli_tester_list(uint32_t ip, uint8_t* i);
bool find_avail_in_reli_tester_list(uint32_t ip, uint8_t* i);
#endif
void inet_ntop(char* ip_str, uint32_t ip);
uint8_t zwave_subnet(uint32_t wkcomm_addr);
uint32_t ip_subnet(uint32_t wkcomm_addr);
bool same_as_used_zwave_nets(uint8_t zwave_net_id, unsigned char* used_nets);
bool same_as_used_wifi_nets(uint32_t wifi_net_id, unsigned char* used_nets);
bool find_empty_index_in_recent_dest(uint8_t* index);
bool find_empty_index_in_recent_dest_wog(uint8_t* index);
bool find_empty_index_in_gate_list(uint8_t* index);
bool find_dest_index_in_recent_dest(uint32_t dest, uint8_t* index);
bool find_dest_index_in_recent_dest_wog(uint32_t dest, uint8_t* index);
bool find_gate_index_in_gate_list(uint32_t wkcomm_addr, uint8_t* index);
void find_and_set_min_time_to_dest(uint8_t dest_index);
uint32_t diff_of_two_num(uint32_t time_a, uint32_t time_b);
unsigned char type_of_radio(uint32_t addr);
void send_by_diff_radios_depend_on_addr(uint32_t addr, uint8_t* payload, uint8_t length);
void copy_in_big_endian(unsigned char* addr_index, uint32_t wkcomm_addr);
void retrieve_in_big_endian(unsigned char* addr_index, uint32_t* wkcomm_addr);
uint8_t random_period(uint8_t start, uint8_t range);
void debug_msg();//FOR DEBUG, show gate list, recent dest list and recent dest wog list on this node


#endif
