#include "config.h"

#ifdef ROUTING_USE_WUKONG_META

#include "types.h"
#include "routing.h"
#include "debug.h"
#include <string.h>
#include "../../../../wkpf/include/common/wkpf_config.h"
#include "djtimer.h"
#include "routing_wukong_meta.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef RADIO_USE_ZWAVE
#include "../radios/radio_zwave.h"
#endif

#ifdef RADIO_USE_WIFI_ARDUINO
#include "../../arduino/radios/radio_wifi1.h"
#endif

struct Routing_Table
{
    wkcomm_address_t my_id;
    wkcomm_address_t gateway_id;
    uint8_t uuid[UUID_LENGTH];
} id_table;

uint8_t routing_mode = NORMAL_MODE;

dj_time_t routing_search_time = 0;
#ifdef RADIO_USE_ZWAVE
#define BROADCAST_ADDRESS 0xFF
radio_zwave_address_t scan_id = BROADCAST_ADDRESS;
#endif
#ifdef RADIO_USE_XBEE
#define BROADCAST_ADDRESS 0xFFFF
radio_xbee_address_t scan_id = BROADCAST_ADDRESS;
#endif



uint32_t ip_net_mask = 0xFFFFFF00;
#ifdef RADIO_USE_ZWAVE
uint8_t my_zwave_net_id = 1;
uint32_t my_zwave_addr = 0;
bool radio_zwave_init_succ = false;
#endif
#ifdef RADIO_USE_WIFI_ARDUINO
uint32_t my_ip = 0;
uint32_t get_net_mask_and_ip_time = 0;
extern char self_ip[16];
extern char self_net_mask[16];
bool radio_wifi_init_succ = false;
#endif
bool I_AM_GATE = false;
uint32_t broadcast_time = 0;
uint32_t retrieve_qos_measure_time = 0;
uint32_t qos_remeasure_time = 0;
uint32_t check_node_failure_time = 0;
bool radio_init_succ = false;
uint8_t qos_remeasure_period = 0;

#ifdef RADIO_USE_WIFI_ARDUINO
struct RELI_TESTER_LIST{
        char ip[16];
        uint8_t recv_num;
        bool test_done;
}reliTesterList[RELI_TESTER_NUM];
#endif

struct ANOTHER_RADIO{
        bool use_zwave;
        uint8_t zwave_net_id;
        bool use_wifi;
        uint32_t wifi_net_id;
};

struct GATE_LIST{
        bool full;
        uint32_t addr;
        uint32_t time;
        uint8_t reli;
        uint32_t time_test_start_time;
        #ifdef RADIO_USE_WIFI_ARDUINO
        uint32_t reli_test_time;
        unsigned char reli_test_time_type; //type:s, e, c
        #endif
        bool recv_broadcast;
        struct ANOTHER_RADIO another_radio;
}gateList[GATE_NUM];

struct RELI_WITH_TIME{
        uint8_t max_reli;
        uint32_t time_restrict;
};
typedef struct RELI_WITH_TIME RWT;

struct THROUGH_GATES{
        bool full;
        bool same_radio_as_dest;
        //In case of 'same radio as dest', reli & time only for the gate's qos measurements. While RWT is unused
        //In other case, reli is unused. Time is used to save shortest time from this gate to the dest. RWT is also used.
        uint8_t reli;
        uint32_t time;
        RWT* reliWithTime;
};

struct RECENT_DEST{
        bool full;
        uint32_t dest;
        uint32_t time; //time_to_dest
        struct THROUGH_GATES gates[GATE_NUM];
}recentDest[RECENT_DEST_NUM];
uint8_t last_time_insert_index_in_recent_dest = RECENT_DEST_NUM - 1;

//without gateway
struct RECENT_DEST_WOG{
        uint32_t dest;
        bool full;
        uint8_t reli;
        uint32_t time;
        uint32_t time_test_start_time;
        #ifdef RADIO_USE_WIFI_ARDUINO
        uint32_t reli_test_time;
        unsigned char reli_test_time_type;
        #endif
}recentDestWOG[RECENT_DEST_WOG_NUM];
uint8_t last_time_insert_index_in_recent_dest_wog = RECENT_DEST_WOG_NUM - 1;



uint8_t routing_send_tmp1(uint32_t dest, uint32_t time, uint8_t reli, uint8_t* payload, uint8_t length){
	uint8_t i;
	unsigned char used_nets[4] = {0};
	used_nets[0] = 'n';
	used_nets[2] = 'n';
	if(find_dest_index_in_recent_dest(dest, &i)){
		routing_send_tmp2(dest, 0, used_nets, time, reli, recentDest[i].time, payload, length);
	}else{
		routing_send_tmp2(dest, 0, used_nets, time, reli, 0, payload, length);
	}

	return 0;
}

uint8_t routing_send_tmp2(uint32_t dest, uint32_t pre_addr, unsigned char* used_nets, uint32_t time, uint8_t reli, 
uint32_t time_to_dest, uint8_t* payload, uint8_t length){
	bool send_directly = check_i_have_same_radio_as_dest(dest);
	if(send_directly){
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nSend directly!!!\n");
		if(I_AM_GATE){
			uint8_t dest_index;
			if(find_dest_index_in_recent_dest_wog(dest, &dest_index)){
				if((recentDestWOG[dest_index].time == 0 && recentDestWOG[dest_index].reli == 0) ||
				(time >= recentDestWOG[dest_index].time && recentDestWOG[dest_index].reli >= reli)){
					time -= recentDestWOG[dest_index].time;
					send(dest, dest, used_nets, time, reli, time_to_dest, payload, length);
				}
			}else{
				send(dest, dest, used_nets, time, reli, time_to_dest, payload, length);
				if(find_empty_index_in_recent_dest_wog(&dest_index) == false){
					dest_index = last_time_insert_index_in_recent_dest_wog + 1;
					if(dest_index == RECENT_DEST_WOG_NUM) dest_index = 0;
					memset(recentDestWOG + dest_index, 0, sizeof(struct RECENT_DEST_WOG));
				}
				recentDestWOG[dest_index].full = true;
				recentDestWOG[dest_index].dest = dest;
				last_time_insert_index_in_recent_dest_wog = dest_index;
				//QoS test
				uint8_t gate_index;
				if(find_gate_index_in_gate_list(dest, &gate_index)){
					recentDestWOG[dest_index].reli = gateList[gate_index].reli;
					recentDestWOG[dest_index].time = gateList[gate_index].time;
				}else{
                                        recentDestWOG[dest_index].time_test_start_time = qos_time_test(dest);
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nTime test start to Dest WOG[%u]\n", dest_index);
                                        if(type_of_radio(dest) == 'z'){
                                                #ifdef RADIO_USE_ZWAVE
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to ZWave Dest WOG[%u]\n", dest_index);
                                                recentDestWOG[dest_index].reli = qos_reli_test_zwave(dest);
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test end to ZWave Dest WOG[%u]\n", dest_index);
                                                #endif
                                        }else if(type_of_radio(dest) == 'w'){
                                                #ifdef RADIO_USE_WIFI_ARDUINO
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to WIFI Dest WOG[%u]\n", dest_index);
                                                recentDestWOG[dest_index].reli_test_time = qos_reli_test_start_wifi(dest);
                                                recentDestWOG[dest_index].reli_test_time_type = 's';
                                                #endif
                                        }
				}
			}
		}else{
			send(dest, dest, used_nets, time, reli, time_to_dest, payload, length);
		}
	}else{
		bool gate_same_radio_as_dest = check_gate_same_radio_as_dest(dest);
		bool gate_ready = check_gate_ready(gate_same_radio_as_dest, dest, used_nets);
		if(!gate_ready){
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGates not ready!!!\n");
			// wait and resend later
		}else{
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGates ready!!!\n");
			check_time_to_dest(dest, pre_addr, time_to_dest);

			uint8_t ready_gate_indexs[GATE_NUM] = {0};
			get_ready_gate_indexs_by_check_used_nets(dest, gate_same_radio_as_dest, ready_gate_indexs, used_nets);

			SG sortGate[GATE_NUM];
			memset(sortGate, 0, sizeof(SG) * GATE_NUM);
			{
				uint8_t i;
				for(i = 0; i < GATE_NUM; i++) sortGate[i].index = GATE_NUM;
			}

			calculate_and_sort_ready_gates_from_max_reli_to_min_reli(dest, ready_gate_indexs, time, gate_same_radio_as_dest, sortGate);

			bool succ = false;
			uint8_t succ_pct = 0, fail_pct = 101;
			//failure percentage, default value is bigger than 1, it means the beginning

			uint8_t last_index_in_sort_gate = 0;
			select_forwarding_gates(&succ, &succ_pct, &fail_pct, sortGate, reli, &last_index_in_sort_gate);

			if(succ == false){//handle failure, I cannot meet requirement and notice previous node
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nFAILED. req: reli is %u, time is %lu\n", reli, time);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "        max: reli is %u\n", succ_pct);
				if(pre_addr != 0){
					send_reli_with_time_restrict_msg(dest, pre_addr, time, succ_pct);
				}
			}else{
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nSUCCESS. req: reli is %u, time is %lu\n", reli, time);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "         max: reli is %u\n", succ_pct);
				if((succ_pct - reli >= 5) && succ_pct > 80){
				//the reli I can provide is much highed than requirement, notice previous node too
					if(pre_addr != 0){
						send_reli_with_time_restrict_msg(dest, pre_addr, time, succ_pct);
					}
				}

				uint8_t dest_index;
				bool dest_exist = find_dest_index_in_recent_dest(dest, &dest_index);
				if(!dest_exist){
					if(find_empty_index_in_recent_dest(&dest_index) == false){
						dest_index = last_time_insert_index_in_recent_dest + 1;
						if(dest_index == RECENT_DEST_NUM) dest_index = 0;
						uint8_t i = 0;
						while(i < GATE_NUM){
							if(recentDest[dest_index].gates[i].reliWithTime != NULL){
								free(recentDest[dest_index].gates[i].reliWithTime);
							}
							i++;
						}
					}
					memset(recentDest + dest_index, 0, sizeof(struct RECENT_DEST));
					recentDest[dest_index].full = true;
					recentDest[dest_index].dest = dest;
					last_time_insert_index_in_recent_dest = dest_index;
				}
				uint8_t i;
				for(i = 0; i <= last_index_in_sort_gate; i++){
					uint32_t new_time = time - gateList[sortGate[i].index].time;//TO DO: subtract elapsed time on me
					uint8_t new_reli = sortGate[i].single_gate_succ_pct;
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "For gate[%u], new reli %u, new time %lu\n", 
					sortGate[i].index, new_reli, new_time);
					if(recentDest[dest_index].gates[sortGate[i].index].full == false){
						recentDest[dest_index].gates[sortGate[i].index].full = true;
						if(gate_same_radio_as_dest == true){
							recentDest[dest_index].gates[sortGate[i].index].same_radio_as_dest = true;
						}
					}
					
					send(gateList[sortGate[i].index].addr, dest, used_nets, new_time, new_reli, 
					recentDest[dest_index].gates[sortGate[i].index].time, payload, length);
				}
			}
		}
	}
	return 0;
}	

bool check_i_have_same_radio_as_dest(uint32_t dest){
	if(type_of_radio(dest) == 'z'){
		#ifdef RADIO_USE_ZWAVE
		if(zwave_subnet(my_zwave_addr) == zwave_subnet(dest)) return true;
		#endif
	}else if(type_of_radio(dest) == 'w'){
		#ifdef RADIO_USE_WIFI_ARDUINO
		if(ip_subnet(my_ip) == ip_subnet(dest)) return true;
		#endif
	}
	return false;
}

void send(uint32_t addr, uint32_t dest, unsigned char* used_nets, uint32_t time, uint8_t reli, uint32_t time_to_dest, 
uint8_t* payload, uint8_t length){
	uint8_t used_nets_len = 4 + used_nets[1] * 4/*wifi_net_len*/ + used_nets[3] * 1/*zwave_net_len*/;

	uint8_t new_used_net_len = 0;
	if(type_of_radio(addr) == 'z'){
		new_used_net_len = 1;
	}else if(type_of_radio(addr) == 'w'){
		new_used_net_len = 4;
	}

	uint8_t payload_w_header_len = 1 + LEN_OF_DID + LEN_OF_DID + used_nets_len + new_used_net_len + 4 + 1 + 4 + length;
	/*payload_w_header = type + dest + pre_addr + used_nets + time + reli + time_to_dest + payload*/

	unsigned char* payload_w_header = (unsigned char*)malloc(payload_w_header_len);
	memset(payload_w_header, 0, payload_w_header_len);

	payload_w_header[0] = 'd';
	copy_in_big_endian(payload_w_header + 1, dest);
	if(type_of_radio(addr) == 'z'){
		#ifdef RADIO_USE_ZWAVE
		copy_in_big_endian(payload_w_header + 1 + LEN_OF_DID, my_zwave_addr);
		used_nets[2] = 'z';
		used_nets[3]++;
		memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID, used_nets, used_nets_len);
		payload_w_header[1 + LEN_OF_DID + LEN_OF_DID + used_nets_len] = zwave_subnet(my_zwave_addr);
		#endif
	}else if(type_of_radio(addr) == 'w'){
		#ifdef RADIO_USE_WIFI_ARDUINO
		copy_in_big_endian(payload_w_header + 1 + LEN_OF_DID, my_ip);
		used_nets[0] = 'w';
		used_nets[1]++;
		memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID, used_nets, 4);
		copy_in_big_endian(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + 4, ip_subnet(my_ip));
		memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + 4 + new_used_net_len, used_nets + 4, used_nets_len - 4);
		#endif
	}
	memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + used_nets_len + new_used_net_len, &time, 4);
	memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + used_nets_len + new_used_net_len + 4, &reli, 1);
	memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + used_nets_len + new_used_net_len + 4 + 1, &time_to_dest, 4);
	memcpy(payload_w_header + 1 + LEN_OF_DID + LEN_OF_DID + used_nets_len + new_used_net_len + 4 + 1 + 4, payload, length);

	send_by_diff_radios_depend_on_addr(addr, payload_w_header, payload_w_header_len);
	free(payload_w_header);
}

bool check_gate_same_radio_as_dest(uint32_t dest){
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(gateList[i].full == true){
			if((type_of_radio(dest) == 'z' && gateList[i].another_radio.use_zwave == true && 
			zwave_subnet(dest) == gateList[i].another_radio.zwave_net_id) ||
			(type_of_radio(dest) == 'w' && gateList[i].another_radio.use_wifi == true &&
			ip_subnet(dest) == gateList[i].another_radio.wifi_net_id)){
				return true;
			}
		}
	}
	return false;
}

bool check_gate_ready(bool gate_same_radio_as_dest, uint32_t dest, unsigned char* used_nets){
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(gateList[i].full && gateList[i].time != 0 && gateList[i].reli != 0){
			if(gate_same_radio_as_dest){
				if( (type_of_radio(dest) == 'z' && gateList[i].another_radio.use_zwave && 
				zwave_subnet(dest) == gateList[i].another_radio.zwave_net_id) ||
				(type_of_radio(dest) == 'w' && gateList[i].another_radio.use_wifi &&
				ip_subnet(dest) == gateList[i].another_radio.wifi_net_id) ){
					return true;
				}
			}else{
				bool used_wifi = false;
				bool used_zwave = false;
				if(used_nets[0] == 'w') used_wifi = true;
				if(used_nets[2] == 'z') used_zwave = true;

				if(used_wifi && type_of_radio(gateList[i].addr) == 'w'){
					if(same_as_used_wifi_nets(ip_subnet(gateList[i].addr), used_nets)){
						continue;
					}
				}
				if(used_wifi && gateList[i].another_radio.use_wifi){
					if(same_as_used_wifi_nets(gateList[i].another_radio.wifi_net_id, used_nets)){
						continue;
					}
				}
				if(used_zwave && type_of_radio(gateList[i].addr) == 'z'){
					if(same_as_used_zwave_nets(zwave_subnet(gateList[i].addr), used_nets)){
						continue;
					}
				}
				if(used_zwave && gateList[i].another_radio.use_zwave){
					if(same_as_used_zwave_nets(gateList[i].another_radio.zwave_net_id, used_nets)){
						continue;
					}
				}
				return true;
			}
		}
	}
	return false;
}

void check_time_to_dest(uint32_t dest, uint32_t pre_addr, uint32_t time_to_dest){
	uint8_t dest_index;
	bool dest_exist = find_dest_index_in_recent_dest(dest, &dest_index);
	if(pre_addr != 0 && dest_exist){
		if(recentDest[dest_index].time != time_to_dest){
			unsigned char* buf = (unsigned char*)malloc(1 + LEN_OF_DID + LEN_OF_DID + 4);
			memset(buf, 0, 1 + LEN_OF_DID + LEN_OF_DID + 4);

			buf[0] = 'n';
			copy_in_big_endian(buf + 1, dest);
			if(type_of_radio(pre_addr) == 'z'){
				#ifdef RADIO_USE_ZWAVE
				copy_in_big_endian(buf + 1 + LEN_OF_DID, my_zwave_addr);
				#endif
			}else if(type_of_radio(pre_addr) == 'w'){
				#ifdef RADIO_USE_WIFI_ARDUINO
				copy_in_big_endian(buf + 1 + LEN_OF_DID, my_ip);
				#endif
			}
			memcpy(buf + 1 + LEN_OF_DID + LEN_OF_DID, &(recentDest[dest_index].time), 4);

			send_by_diff_radios_depend_on_addr(pre_addr, buf, 1 + LEN_OF_DID + LEN_OF_DID + 4);
			free(buf);
		}
	}
}

void get_ready_gate_indexs_by_check_used_nets(uint32_t dest, bool gate_same_radio_as_dest, uint8_t* ready_gate_indexs, unsigned char* used_nets){
	if(gate_same_radio_as_dest){
		uint8_t i;
		for(i = 0; i < GATE_NUM; i++){
			if(gateList[i].full && gateList[i].reli != 0 && gateList[i].time != 0){
				if( (type_of_radio(dest) == 'z' && gateList[i].another_radio.use_zwave && 
				zwave_subnet(dest) == gateList[i].another_radio.zwave_net_id) ||
				(type_of_radio(dest) == 'w' && gateList[i].another_radio.use_wifi &&
				ip_subnet(dest) == gateList[i].another_radio.wifi_net_id) ){
						ready_gate_indexs[i] = 1;
				}
			}
		}
	}else{
		uint8_t i;
		bool used_wifi = false;
		bool used_zwave = false;
		if(used_nets[0] == 'w') used_wifi = true;
		if(used_nets[2] == 'z') used_zwave = true;
		for(i = 0; i < GATE_NUM; i++){
			if(gateList[i].full && gateList[i].reli != 0 && gateList[i].time != 0){
				if(used_wifi && type_of_radio(gateList[i].addr) == 'w'){
					if(same_as_used_wifi_nets(ip_subnet(gateList[i].addr), used_nets)){
						continue;
					}
				}
				if(used_wifi && gateList[i].another_radio.use_wifi){
					if(same_as_used_wifi_nets(gateList[i].another_radio.wifi_net_id, used_nets)){
						continue;
					}
				}
				if(used_zwave && type_of_radio(gateList[i].addr) == 'z'){
					if(same_as_used_zwave_nets(zwave_subnet(gateList[i].addr), used_nets)){
						continue;
					}
				}
				if(used_zwave && gateList[i].another_radio.use_zwave){
					if(same_as_used_zwave_nets(gateList[i].another_radio.zwave_net_id, used_nets)){
						continue;
					}
				}
				ready_gate_indexs[i] = 1;
			}
		}
	}
}

void calculate_and_sort_ready_gates_from_max_reli_to_min_reli(uint32_t dest, uint8_t* ready_gate_indexs, uint32_t time, 
bool gate_same_radio_as_dest, SG* sortGate){
	uint8_t dest_index, i;
	bool dest_exist = find_dest_index_in_recent_dest(dest, &dest_index);

	for(i = 0; i < GATE_NUM; i++){
		if(ready_gate_indexs[i] == 1){
			uint32_t shortest_time_to_dest_by_this_gate;
			if(dest_exist){
				shortest_time_to_dest_by_this_gate = gateList[i].time + recentDest[dest_index].gates[i].time;
			}else{
				shortest_time_to_dest_by_this_gate = gateList[i].time;
			}

			if(time > shortest_time_to_dest_by_this_gate){
				uint8_t single_gate_succ_pct = 0;
				uint8_t single_path_succ_pct;
				if(gate_same_radio_as_dest){//already has a record
					if(dest_exist && recentDest[dest_index].gates[i].reli != 0){
						single_gate_succ_pct = recentDest[dest_index].gates[i].reli;
					}else{
						if(gateList[i].another_radio.use_zwave){
							single_gate_succ_pct = 100;
						}else if(gateList[i].another_radio.use_wifi){
							single_gate_succ_pct = 50;//30;
						}
					}
				}else{
					single_gate_succ_pct = 80;//50~80?
					if(dest_exist && recentDest[dest_index].gates[i].reliWithTime != NULL){
						uint32_t remaining_time, time_restrict, min_time_restrict = 0;
						remaining_time = time - gateList[i].time;
						uint8_t j;
 						for(j = 0; j < RWT_NUM; j++){
							time_restrict = recentDest[dest_index].gates[i].reliWithTime[j].time_restrict;
							if(time_restrict > 0){
								if((min_time_restrict == 0 || time_restrict < min_time_restrict) &&
								((time_restrict >= remaining_time) || (remaining_time - time_restrict < 300))){
									min_time_restrict = time_restrict;
									single_gate_succ_pct = 
									recentDest[dest_index].gates[i].reliWithTime[j].max_reli - 2;
									//for a conservative estimation, subtract 2 from max_reli
									//make sure the reli requirement will be met by this selected gate
								}
							}
						}
					}
				}
				single_path_succ_pct = (gateList[i].reli * single_gate_succ_pct) / 100;
				//sorting
				uint8_t k;
				for(k = 0; k < GATE_NUM; k++){
					if(sortGate[k].index == GATE_NUM) break;
				}
				while(k > 0){
					if(single_path_succ_pct > sortGate[k-1].single_path_succ_pct){
						sortGate[k].single_path_succ_pct = sortGate[k-1].single_path_succ_pct;
						sortGate[k].single_gate_succ_pct = sortGate[k-1].single_gate_succ_pct;
						sortGate[k].index = sortGate[k-1].index;
						k--;
					}else{
						break;
					}
				}
				sortGate[k].single_path_succ_pct = single_path_succ_pct;
				sortGate[k].single_gate_succ_pct = single_gate_succ_pct;
				sortGate[k].index = i;
			}
		}
	}
}

void select_forwarding_gates(bool* succ , uint8_t* succ_pct, uint8_t* fail_pct, SG* sortGate, uint8_t reli, uint8_t* last_index_in_sort_gate){
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(*fail_pct > 100){//means the beginning, send by single gateway
			*succ_pct = sortGate[i].single_path_succ_pct;
			if(*succ_pct >= reli){//succ_pct is higher than reli requirement
				*succ = true;
				sortGate[i].single_gate_succ_pct = (reli * 100) / gateList[sortGate[i].index].reli;
				//lower reli requirement
				*last_index_in_sort_gate = i;
				break;
			}else{
				*fail_pct = 100 - *succ_pct;
			}
		}else{
			*fail_pct = ((*fail_pct) * (100 - sortGate[i].single_path_succ_pct)) / 100;
			*succ_pct = 100 - *fail_pct;
			if(*succ_pct >= reli){
				*succ = true;
				*last_index_in_sort_gate = i;
				break;
			}
		}

	}
}

void send_reli_with_time_restrict_msg(uint32_t dest, uint32_t pre_addr, uint32_t time, uint8_t succ_pct){
	unsigned char* buf = (unsigned char*)malloc(1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);
	memset(buf, 0, 1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);

	buf[0] = 'c';
	copy_in_big_endian(buf + 1, dest);
	if(type_of_radio(pre_addr) == 'z'){
		#ifdef RADIO_USE_ZWAVE
		copy_in_big_endian(buf + 1 + LEN_OF_DID, my_zwave_addr);
		#endif
	}else if(type_of_radio(pre_addr) == 'w'){
		#ifdef RADIO_USE_WIFI_ARDUINO
		copy_in_big_endian(buf + 1 + LEN_OF_DID, my_ip);
		#endif
	}
	memcpy(buf + 1 + LEN_OF_DID + LEN_OF_DID, &time, 4);
	memcpy(buf + 1 + LEN_OF_DID + LEN_OF_DID + 4, &succ_pct, 1);

	send_by_diff_radios_depend_on_addr(pre_addr, buf, 1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);
	free(buf);			
}

uint32_t qos_time_test(uint32_t addr){
	unsigned char* buf = (unsigned char*)malloc(1 + LEN_OF_DID);
	memset(buf, 0, 1 + LEN_OF_DID);
	
	buf[0] = 't';
	if(type_of_radio(addr) == 'z'){
		#ifdef RADIO_USE_ZWAVE
		copy_in_big_endian(buf + 1, my_zwave_addr);
		#endif
	}else if(type_of_radio(addr) == 'w'){
		#ifdef RADIO_USE_WIFI_ARDUINO
		copy_in_big_endian(buf + 1, my_ip);
		#endif
	}
	send_by_diff_radios_depend_on_addr(addr, buf, 1 + LEN_OF_DID);
	free(buf);
	return dj_timer_getTimeMillis();
}

#ifdef RADIO_USE_ZWAVE
uint8_t qos_reli_test_zwave(uint32_t addr){
	uint8_t i = 100, cnt = 0;
	unsigned char buf[1] = {0};
	buf[0] = 'r';
	while(i > 0){
		if(radio_zwave_send(addr_wkcomm_to_zwave(addr), buf, sizeof(buf)) == 0){
			cnt++;
		}
		i--;
	}
	return cnt;
}
#endif	

#ifdef RADIO_USE_WIFI_ARDUINO
uint32_t qos_reli_test_start_wifi(uint32_t addr){
	unsigned char buf[2] = {0};
	buf[0] = 'r';
	buf[1] = 's';
	send_by_diff_radios_depend_on_addr(addr, buf, sizeof(buf));
	return dj_timer_getTimeMillis();
}

uint32_t qos_reli_test_end_wifi(uint32_t addr){
	unsigned char buf[2] = {0};
	buf[0] = 'r';
	buf[1] = 'e';
	send_by_diff_radios_depend_on_addr(addr, buf, sizeof(buf));
	return dj_timer_getTimeMillis();
}

void qos_reli_test_wifi(char* addr, uint8_t gate_reli_tester_index){
	uint8_t i;
	unsigned char buf[2] = {0};
	buf[0] = 'r';
	buf[1] = gate_reli_tester_index;

		radio_wifi_send(addr, buf, sizeof(buf));
	for(i = 0; i < 49/*num of wifi reli test sending, remain uncertain, 100 times takes too much time, while 10 times is imprecise*/; i++){
		dj_timer_delay(100);
		radio_wifi_send_without_ip(buf, sizeof(buf));
	}
	DEBUG_LOG(DBG_WUKONG_META_ROUTING, "Finish sending 50 msgs to %s\n", addr);
}
#endif	

#ifdef RADIO_USE_ZWAVE
void routing_handle_zwave_message(radio_zwave_address_t zwave_addr, uint8_t *payload, uint8_t length){
	if(routing_mode == NORMAL_MODE && payload[0] == 'r'){
		//ignore reli test msg
	}else{
		routing_handle_message(payload, length);
	}
	memset(payload, 0, length);
}
#endif

#ifdef RADIO_USE_WIFI_ARDUINO
void routing_handle_wifi_message(char* wifi_addr, uint8_t *payload, uint8_t length){
	if(routing_mode == NORMAL_MODE && payload[0] == 'r'){
		switch(payload[1]){
			case 's':
				{
					uint8_t i;
					if(find_match_in_reli_tester_list(wifi_addr, &i))
					{
						unsigned char buf[4] = {0};
						buf[0] = 'a';
						buf[1] = 'r';
						buf[2] = 's';
						buf[3] = i;
						radio_wifi_send(wifi_addr, buf, sizeof(buf));
					}else if(find_avail_in_reli_tester_list(wifi_addr, &i)){
						memcpy(reliTesterList[i].ip, wifi_addr, 16);
						unsigned char buf[4] = {0};
						buf[0] = 'a';
						buf[1] = 'r';
						buf[2] = 's';
						buf[3] = i;
						radio_wifi_send(wifi_addr, buf, sizeof(buf));
					}
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli start from ip %s, and send ack back\n", wifi_addr);
					break;
				}
			case 'e':
				{
					uint8_t i;
					if(find_match_in_reli_tester_list(wifi_addr, &i)){
						unsigned char buf[4] = {0};
						buf[0] = 'a';
						buf[1] = 'r';
						buf[2] = 'e';
						buf[3] = reliTesterList[i].recv_num;
						radio_wifi_send(wifi_addr, buf, sizeof(buf));
						reliTesterList[i].test_done = true;
					}
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli end from ip %s, and send ack back\n", wifi_addr);
					break;
				}
			case 'c':
				{
					uint8_t i;
					unsigned char buf[3] = {0};
					if(find_match_in_reli_tester_list(wifi_addr, &i)){
						memset(reliTesterList + i, 0, sizeof(struct RELI_TESTER_LIST));
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nClear reli tester list[%u], then send ack\n", i);
					}
					buf[0] = 'a';
					buf[1] = 'r';
					buf[2] = 'c';
					radio_wifi_send(wifi_addr, buf, sizeof(buf));
					break;
				}
			default:
				if(!reliTesterList[payload[1]].test_done){
					reliTesterList[payload[1]].recv_num++;
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "recv_num[%u] = %u, from ip %s\n", 
					payload[1], reliTesterList[payload[1]].recv_num, wifi_addr);
				}else{
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv R from ip %s, but test is done.\n", wifi_addr);
				}
				break;
		}
	}else if(routing_mode == NORMAL_MODE && payload[0] == 'a' && payload[1] == 'r'){
		switch(payload[2]){
			case 's':
				{
					uint8_t i;
					uint32_t addr = inet_pton(wifi_addr);

					if(find_gate_index_in_gate_list(addr, &i) 
					&& gateList[i].reli_test_time_type == 's'){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli start ack from WIFI gate[%u]\n", i);
						qos_reli_test_wifi(wifi_addr, payload[3]);
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test end to WIFI gate[%u]\n", i);
						gateList[i].reli_test_time = qos_reli_test_end_wifi(addr); 
						gateList[i].reli_test_time_type = 'e';
					}else if(find_dest_index_in_recent_dest_wog(addr, &i)
					&& recentDestWOG[i].reli_test_time_type == 's'){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli start ack from WIFI Dest WOG[%u]\n", i);
						qos_reli_test_wifi(wifi_addr, payload[3]);
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test end to WIFI Dest WOG[%u]\n", i);
						recentDestWOG[i].reli_test_time = qos_reli_test_end_wifi(addr);
						recentDestWOG[i].reli_test_time_type = 'e';
					}
					break;
				}
			case 'e':
				{
					uint8_t i;
					uint32_t addr = inet_pton(wifi_addr);

					if(find_gate_index_in_gate_list(addr, &i) 
					&& gateList[i].reli_test_time_type == 'e'){
						gateList[i].reli = payload[3] * 2;
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli end ack from WIFI gate[%u], reli = %u\n", 
						i, gateList[i].reli);
						unsigned char buf[2] = {0};
						buf[0] = 'r';
						buf[1] = 'c';
						radio_wifi_send(wifi_addr, buf, sizeof(buf));
						gateList[i].reli_test_time = dj_timer_getTimeMillis();
						gateList[i].reli_test_time_type = 'c';
					}else if(find_dest_index_in_recent_dest_wog(addr, &i)
					&& recentDestWOG[i].reli_test_time_type == 'e'){
						recentDestWOG[i].reli = payload[3] * 2;
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv reli end ack from WIFI Dest WOG[%u], reli = %u\n", 
						i, recentDestWOG[i].reli);
						unsigned char buf[2] = {0};
						buf[0] = 'r';
						buf[1] = 'c';
						radio_wifi_send(wifi_addr, buf, sizeof(buf));
						recentDestWOG[i].reli_test_time = dj_timer_getTimeMillis();
						recentDestWOG[i].reli_test_time_type = 'c';
					}
					break;
				}
			case 'c':
				{
					uint8_t i;
					uint32_t addr = inet_pton(wifi_addr);

					if(find_gate_index_in_gate_list(addr, &i) 
					&& gateList[i].reli_test_time_type == 'c'){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nClear reli tester list in WIFI gate[%u]\n", i);
						gateList[i].reli_test_time = 0;
						gateList[i].reli_test_time_type = 0;
					}else if(find_dest_index_in_recent_dest_wog(addr, &i)
					&& recentDestWOG[i].reli_test_time_type == 'c'){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nClear reli tester list in WIFI Dest WOG[%u]\n", i);
						recentDestWOG[i].reli_test_time = 0;
						recentDestWOG[i].reli_test_time_type = 0;
					}
					break;					
				}
			default:
				break;
		}
	}else{
		routing_handle_message(payload, length);
	}
	memset(payload, 0, length);
}
#endif // RADIO_USE_WIFI_ARDUINO

void routing_handle_message_normal_node(uint8_t *payload, uint8_t length){
	if(payload[0] == 'b'){
		///////////////////////////////////////////////////////////////////
		bool ignore = false;
		if(payload[1] == 0){//ZWave
			#ifdef RADIO_USE_ZWAVE
			if(zwave_subnet(my_zwave_addr) != payload[3]){
				ignore =true;
			}
			#endif
		}
		if(!radio_init_succ) ignore = true;
		///////////////////////////////////////////////////////////////////
		if(!ignore){
			bool same_radio_as_gate = false;
			unsigned char another_net = payload[1 + LEN_OF_DID];
			if(I_AM_GATE){
				if(another_net == 'z'){
					#ifdef RADIO_USE_ZWAVE
					uint8_t gate_zwave_net_id = payload[1 + LEN_OF_DID + 1];
					if(gate_zwave_net_id == zwave_subnet(my_zwave_addr)) same_radio_as_gate = true;
					#endif
				}else if(another_net == 'w'){
					#ifdef RADIO_USE_WIFI_ARDUINO
					uint32_t gate_wifi_net_id;
					retrieve_in_big_endian(payload + 1 + LEN_OF_DID + 1, &gate_wifi_net_id);
					if(gate_wifi_net_id == ip_subnet(my_ip)) same_radio_as_gate = true;
					#endif
				}
			}
			if(!same_radio_as_gate){
				uint32_t gate_addr;
				retrieve_in_big_endian(payload + 1, &gate_addr);
				uint8_t i;
				if(find_gate_index_in_gate_list(gate_addr, &i)){
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv broadcast from gate[%u]\n", i);
					gateList[i].recv_broadcast = true;
				}else if( /*!find_gate_index_in_gate_list(gate_addr, &i) &&*/ find_empty_index_in_gate_list(&i) ){
					gateList[i].full = true;
					gateList[i].recv_broadcast = true;
					gateList[i].addr = gate_addr;

					if(another_net == 'z'){
						gateList[i].another_radio.use_zwave = true;
						gateList[i].another_radio.zwave_net_id = payload[1 + LEN_OF_DID + 1];
					}else if(another_net == 'w'){
						gateList[i].another_radio.use_wifi = true;
						retrieve_in_big_endian(payload + 1 + LEN_OF_DID + 1, &(gateList[i].another_radio.wifi_net_id));
					}
					//QoS test
					gateList[i].time_test_start_time = qos_time_test(gateList[i].addr);
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nTime test start to gate[%u]\n", i);
					if(type_of_radio(gateList[i].addr) == 'z'){
						#ifdef RADIO_USE_ZWAVE
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to ZWave gate[%u]\n", i);
						gateList[i].reli = qos_reli_test_zwave(gateList[i].addr);
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test end to ZWave gate[%u]\n", i);
						#endif	
					}else if(type_of_radio(gateList[i].addr) == 'w'){
						#ifdef RADIO_USE_WIFI_ARDUINO
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to WIFI gate[%u]\n", i);
						gateList[i].reli_test_time = qos_reli_test_start_wifi(gateList[i].addr);
						gateList[i].reli_test_time_type = 's';
						#endif	
					}
				}else{
					//TO DO:
					//2.find max distance gateway and replace it
					//3.or ignore this gateway info
				}
			}
		}
	}
	else if(payload[0] == 't'){
		uint32_t pre_addr = 0;
		unsigned char* buf = (unsigned char*)malloc(1 + 1 + LEN_OF_DID);
		memset(buf, 0, 1 + 1 + LEN_OF_DID);
		retrieve_in_big_endian(payload + 1, &pre_addr);

		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv time test, then send ack\n");
		buf[0] = 'a';
		buf[1] = 't';
		if(type_of_radio(pre_addr) == 'z'){
			#ifdef RADIO_USE_ZWAVE
			copy_in_big_endian(buf + 2, my_zwave_addr);
			#endif
		}else if(type_of_radio(pre_addr) == 'w'){
			#ifdef RADIO_USE_WIFI_ARDUINO
			copy_in_big_endian(buf + 2, my_ip);
			#endif
		}
		send_by_diff_radios_depend_on_addr(pre_addr, buf, 1 + 1 + LEN_OF_DID);
		free(buf);
	}
	else if(payload[0] == 'd'){
		uint32_t dest;
		uint8_t used_nets_index, used_nets_len, data_index, data_len;
		bool i_am_dest = false;
		unsigned char data[50] = {0};

		used_nets_index = 1 + LEN_OF_DID + LEN_OF_DID;
		used_nets_len = 4/*'w/n' + net num + 'z/n' + net num*/ + payload[used_nets_index + 1] * 4 + payload[used_nets_index + 3] * 1;
		data_index = used_nets_index + used_nets_len + 5/*time and reli*/ + 4/*time_to_dest*/;
		data_len = length - data_index;
		retrieve_in_big_endian(payload + 1, &dest);
		memcpy(data, payload + data_index, data_len);

		if(type_of_radio(dest) == 'z'){
			#ifdef RADIO_USE_ZWAVE
			if(my_zwave_addr == dest) i_am_dest = true;
			#endif
		}else if(type_of_radio(dest) == 'w'){
			#ifdef RADIO_USE_WIFI_ARDUINO
			if(my_ip == dest) i_am_dest = true;
			#endif
		}

		if(i_am_dest){
			//pass to upper layer
			//wkcomm_handle_message();
			//wkcomm_handle_message(addr_zwave_to_wkcomm(zwave_addr), payload, length);
			DEBUG_LOG(/*DBG_WUKONG_META_ROUTING*/true, "\nI am Dest. Payload = %s\n", data);
		}else{
			unsigned char used_nets[20] = {0};
			uint32_t pre_addr, time, time_to_dest;
			uint8_t reli;	

			retrieve_in_big_endian(payload + 1 + LEN_OF_DID, &pre_addr);
			memcpy(used_nets, payload + used_nets_index, used_nets_len);
			memcpy(&time, payload + used_nets_index + used_nets_len, 4);
			memcpy(&reli, payload + used_nets_index + used_nets_len + 4, 1);
			memcpy(&time_to_dest, payload + used_nets_index + used_nets_len + 4 + 1, 4);
			DEBUG_LOG(/*DBG_WUKONG_META_ROUTING*/true, "\nForwarding, data = %s, time = %lu\n", data, time);
			routing_send_tmp2(dest, pre_addr, used_nets, time, reli, time_to_dest, data, data_len);
		}
	}else if(payload[0] == 'c'){
		uint8_t i, j;
		uint32_t dest, pre_addr;
		retrieve_in_big_endian(payload + 1, &dest);
		retrieve_in_big_endian(payload + 1 + LEN_OF_DID, &pre_addr);
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv CMR msg\n");
		if(find_dest_index_in_recent_dest(dest, &i) && find_gate_index_in_gate_list(pre_addr, &j)){
			uint32_t time_restrict;
			uint8_t max_reli;
			memcpy(&time_restrict, payload + 1 + LEN_OF_DID + LEN_OF_DID, 4);
			memcpy(&max_reli, payload + 1 + LEN_OF_DID + LEN_OF_DID + 4, 1);
			if(recentDest[i].gates[j].reliWithTime == NULL){
				recentDest[i].gates[j].reliWithTime = (RWT*)malloc(sizeof(RWT) * RWT_NUM);
				memset(recentDest[i].gates[j].reliWithTime, 0, sizeof(RWT) * RWT_NUM);
			}

			//1.先找差距小於等於100取代之
			//2.否則找空位
			//3.否則找時間差最小取代之
			uint8_t k;
			for(k = 0; k < RWT_NUM; k++){
				if(recentDest[i].gates[j].reliWithTime[k].time_restrict != 0 &&
				diff_of_two_num(recentDest[i].gates[j].reliWithTime[k].time_restrict, time_restrict) <= 100) 
					break;
			}
			if(k == RWT_NUM){
				uint8_t min_gap = 0, min_gap_index = 0;
				for(k = 0; k < RWT_NUM; k++){
					if(recentDest[i].gates[j].reliWithTime[k].time_restrict == 0){ 
						break;
					}else if(min_gap == 0 || 
					diff_of_two_num(recentDest[i].gates[j].reliWithTime[k].time_restrict, time_restrict) < min_gap){ 
						min_gap = diff_of_two_num(recentDest[i].gates[j].reliWithTime[k].time_restrict, time_restrict);
						min_gap_index = k;
					}
				}
				if(k == RWT_NUM) k = min_gap_index;
			}
			recentDest[i].gates[j].reliWithTime[k].time_restrict = time_restrict;
			recentDest[i].gates[j].reliWithTime[k].max_reli = max_reli;
		}
	}else if(payload[0] == 'n'){
		uint8_t i, j;
		uint32_t dest, pre_addr;
		retrieve_in_big_endian(payload + 1, &dest);
		retrieve_in_big_endian(payload + 1 + LEN_OF_DID, &pre_addr);
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv NCT msg\n");
		if(find_dest_index_in_recent_dest(dest, &i) && find_gate_index_in_gate_list(pre_addr, &j)){
			memcpy(&(recentDest[i].gates[j].time), payload + 1 + LEN_OF_DID + LEN_OF_DID, 4);
			find_and_set_min_time_to_dest(i);
		}
	}else if(payload[0] == 's'){
		uint8_t i;
		uint32_t dest, pre_addr;
		retrieve_in_big_endian(payload + 1, &dest);
		retrieve_in_big_endian(payload + 1 + LEN_OF_DID, &pre_addr);
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv retrieve qos msg, then send ack back\n");
		if(find_dest_index_in_recent_dest_wog(dest, &i)){
			if(recentDestWOG[i].reli != 0 && recentDestWOG[i].time != 0){
				unsigned char* buf = (unsigned char*)malloc(1 + 1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);
				memset(payload, 0, 1 + 1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);

				buf[0] = 'a';
				buf[1] = 's';
				copy_in_big_endian(buf + 2, dest);
				if(type_of_radio(pre_addr) == 'z'){
					#ifdef RADIO_USE_ZWAVE
					copy_in_big_endian(buf + 2 + LEN_OF_DID, my_zwave_addr);
					#endif
				}else if(type_of_radio(pre_addr) == 'w'){
					#ifdef RADIO_USE_WIFI_ARDUINO
					copy_in_big_endian(buf + 2 + LEN_OF_DID, my_ip);
					#endif
				}
				memcpy(buf + 2 + LEN_OF_DID + LEN_OF_DID, &(recentDestWOG[i].time), 4);
				memcpy(buf + 2 + LEN_OF_DID + LEN_OF_DID + 4, &(recentDestWOG[i].reli), 1);

				send_by_diff_radios_depend_on_addr(pre_addr, buf, 1 + 1 + LEN_OF_DID + LEN_OF_DID + 4 + 1);
				free(buf);
			}
		}
	}else if(payload[0] == 'a'){
		switch (payload[1]){
			case 't':
				{
					uint32_t arrival_time = dj_timer_getTimeMillis();
					uint32_t addr = 0;
					uint8_t i;
					retrieve_in_big_endian(payload + 2, &addr);
					if(find_gate_index_in_gate_list(addr, &i) 
					&& gateList[i].time_test_start_time != 0 
					&& arrival_time > gateList[i].time_test_start_time){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv time test ack from gate[%u]\n", i);
						gateList[i].time = (arrival_time - gateList[i].time_test_start_time) / 2;
						gateList[i].time_test_start_time = 0;

						uint8_t j;
						for(j = 0; j < RECENT_DEST_NUM; j++){
							if(recentDest[j].full && recentDest[j].gates[i].time != 0){
								find_and_set_min_time_to_dest(j);
							}
						}
					}else if(find_dest_index_in_recent_dest_wog(addr, &i)
					&& recentDestWOG[i].time_test_start_time != 0 
					&& arrival_time > recentDestWOG[i].time_test_start_time){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv time test ack from Dest WOG[%u]\n", i);
						recentDestWOG[i].time = (arrival_time - recentDestWOG[i].time_test_start_time) / 2;
						recentDestWOG[i].time_test_start_time = 0;
					}
					break;
				}
			case 's':
				{
					uint8_t i, j;
					uint32_t dest, pre_addr;
					retrieve_in_big_endian(payload + 2, &dest);
					retrieve_in_big_endian(payload + 2 + LEN_OF_DID, &pre_addr);
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nRecv retrieve QoS msg ack\n", i);
					if(find_dest_index_in_recent_dest(dest, &i) && find_gate_index_in_gate_list(pre_addr, &j)){
						memcpy( &(recentDest[i].gates[j].time), payload + 2 + LEN_OF_DID + LEN_OF_DID, 4);
						memcpy( &(recentDest[i].gates[j].reli), payload + 2 + LEN_OF_DID + LEN_OF_DID + 4, 1);
						find_and_set_min_time_to_dest(i);
					}
					break;
				}
			default:
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\n[ROUTING] unknow type of ACK\n");
				break;
		}
	}

}

void routing_handle_message(uint8_t *payload, uint8_t length)
{
	if(routing_mode == NORMAL_MODE){
		routing_handle_message_normal_node(payload, length);
	}else if (routing_mode == GATEWAY_DISCOVERY_MODE || routing_mode == ID_REQ_MODE){
		wkcomm_address_t dest=0, src=0;
		uint8_t msg_type, i;
		char ipstr[IP_ADDRSTRLEN];

		DEBUG_LOG(DBG_WKROUTING, "r_handle packet:[ ");
		for (i = 0; i < length; ++i){
			DEBUG_LOG(DBG_WKROUTING, "%d", payload[i]);
			DEBUG_LOG(DBG_WKROUTING, " ");
		}
		DEBUG_LOG(DBG_WKROUTING, "]\n");


		if (length < MPTN_PAYLOAD_BYTE_OFFSET)
		{
			DEBUG_LOG(DBG_WKROUTING, "r_handle: drops garbage\n");
			return;
		}
		for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
		{
			dest <<= 8;
			dest |= payload[i];
		}
		// DEBUG_LOG(DBG_WKROUTING, "r_handle: dest_id %d\n", dest);
		for (i = MPTN_SRC_BYTE_OFFSET; i < MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN; ++i)
		{
			src <<= 8;
			src |= payload[i];
		}
		// DEBUG_LOG(DBG_WKROUTING, "r_handle: src_id %d\n", src);

		msg_type = payload[MPTN_MSGTYPE_BYTE_OFFSET];
		// DEBUG_LOG(DBG_WKROUTING, "r_handle: msg_type %d\n", msg_type);
		// DEBUG_LOG(DBG_WKROUTING, "r_handle: my_id %d\n", id_table.my_id);

		if (routing_mode == GATEWAY_DISCOVERY_MODE)
		{
			if (msg_type == MPTN_MSGTYPE_GWOFFER)
			{
				id_table.gateway_id = src;
				wkpf_config_set_gwid(src);
				inet_ntop(ipstr, src);
				DEBUG_LOG(DBG_WKROUTING, "r_handle: get gateway id %s\n", ipstr);
				DEBUG_LOG(DBG_WKROUTING, "r_handle: get uuid=");
				for (i = 0; i < MPTN_UUID_LEN; ++i)
				{
					id_table.uuid[i] = payload[i+MPTN_PAYLOAD_BYTE_OFFSET];
					DEBUG_LOG(DBG_WKROUTING, " %d", id_table.uuid[i]);
				}
				DEBUG_LOG(DBG_WKROUTING, "\n");
				wkpf_config_set_uuid(id_table.uuid);
				routing_id_req();
			}
		}
		else if (routing_mode == ID_REQ_MODE)
		{
			if (msg_type == MPTN_MSGTYPE_IDACK && src == 0)
			{
				DEBUG_LOG(DBG_WKROUTING, "r_handle: recv IDACK packet\n");
				inet_ntop(ipstr, dest);
				if (dest != id_table.my_id){
					wkpf_config_set_myid(dest);
					id_table.my_id = dest;
					DEBUG_LOG(DBG_WKROUTING, "r_handle: set id=%s\n",ipstr);
					// id_table.gateway_id = GET_ID_PREFIX(dest) | GET_ID_RADIO_ADDRESS(id_table.gateway_id);
					// wkpf_config_set_gwid(id_table.gateway_id);
				} else {
					DEBUG_LOG(DBG_WKROUTING, "r_handle: the same ID=%s\n",ipstr);
				}
				routing_mode = NORMAL_MODE;
			}
    		}
	}
}

// INIT
void routing_init() {
	#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_WIFI_ARDUINO)
		I_AM_GATE = true;
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nI'm gate !\n");
	#endif

	#ifdef RADIO_USE_ZWAVE
		radio_zwave_init();
	#endif
	#ifdef RADIO_USE_WIFI_ARDUINO
		radio_wifi_init();
		get_net_mask_and_ip_time = dj_timer_getTimeMillis();//
		if(self_ip[0] != '0' && self_ip[0] != 0){
			my_ip = inet_pton(self_ip);
			ip_net_mask = inet_pton(self_net_mask);
			if(I_AM_GATE){
				radio_wifi_init_succ = true;
			}else{
				radio_init_succ = true;	
			}
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nWIFI init succeed !!! self_ip = %s, self_net_mask = %s\n", self_ip, self_net_mask);
		}else{
			get_net_mask_and_ip_time = dj_timer_getTimeMillis();
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nWIFI init failed !!! self_ip = %s, self_net_mask = %s\n", self_ip, self_net_mask);
		}
	#endif

	routing_poweron_init();
	variable_init();

	#ifdef RADIO_USE_ZWAVE
		my_zwave_addr = id_table.my_id;
 		//2.my_zwave_addr = ((my_zwave_net_id<<8) | id_table.my_id);
		//3.my_zwave_addr |= my_zwave_net_id;
		//  my_zwave_addr <<= 8;
		//  my_zwave_addr |= radio_zwave_get_node_id();
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nZWave init succeed !!! ");
		DEBUG_LOG(DBG_WUKONG_META_ROUTING, "my ZWave net id = %u, my ZWave addr = %u\n", 
		zwave_subnet(my_zwave_addr), addr_wkcomm_to_zwave(my_zwave_addr)); 
		if(I_AM_GATE){
			radio_zwave_init_succ = true;
		}else{
			radio_init_succ = true;	
		}
	#endif

	if(I_AM_GATE && !radio_init_succ){
		#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_WIFI_ARDUINO)
		if(radio_wifi_init_succ && radio_zwave_init_succ) radio_init_succ = true;
		#endif
	}

	if(routing_mode == NORMAL_MODE && I_AM_GATE){
		if(radio_init_succ){
			broadcast_time = dj_timer_getTimeMillis();
			broadcast_gate_addr();
		}
	}
}

void variable_init(){
	//init gate list
	memset(gateList, 0, sizeof(struct GATE_LIST) * GATE_NUM);

	//init recnet used dest list
	#ifdef RADIO_USE_WIFI_ARDUINO
	memset(reliTesterList, 0, sizeof(struct RELI_TESTER_LIST) * RELI_TESTER_NUM);
	#endif

	//init recnet used dest list
	memset(recentDest, 0, sizeof(struct RECENT_DEST) * RECENT_DEST_NUM);

	//init recent used dest without gate list
	memset(recentDestWOG, 0, sizeof(struct RECENT_DEST_WOG) * RECENT_DEST_WOG_NUM);

	//set seed for rand()
	srand(dj_timer_getTimeMillis());
	#ifdef RADIO_USE_ZWAVE
	srand(dj_timer_getTimeMillis() * radio_zwave_get_node_id());
	#endif
	//set qos remeasurement period. 
	//to avoid network congestion, each node has random period
	qos_remeasure_period = random_period(5, 6);//5~10 min
}

void routing_poweron_init()
{
    DEBUG_LOG(DBG_WKROUTING, "r_poweron_init\n");
    id_table.my_id = wkpf_config_get_myid();
    id_table.gateway_id = wkpf_config_get_gwid();
    wkpf_config_get_uuid(id_table.uuid);
    // routing_get_mac_address();
    routing_id_req();
    // dj_timer_delay(10);
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void broadcast_gate_addr(){
	if(I_AM_GATE){
		#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_WIFI_ARDUINO)
			//broadcast by zwave
			uint8_t payload1_len = 1 + LEN_OF_DID + 1 + LEN_OF_WIFI_SUBNET;
			unsigned char* payload1 = (unsigned char*)malloc(payload1_len);
			memset(payload1, 0, payload1_len);

			payload1[0] = 'b';
			copy_in_big_endian(payload1 + 1, my_zwave_addr);
			payload1[1 + LEN_OF_DID] = 'w';
			copy_in_big_endian(payload1 + 1 + LEN_OF_DID + 1, ip_subnet(my_ip));

			radio_zwave_send(0xFF, payload1, payload1_len);
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nbroadcast: %c, %u:%u:%u:%u, %c, %u.%u.%u.%u\n", 
			payload1[0], payload1[1], payload1[2],payload1[3], payload1[4], 
			payload1[1 + LEN_OF_DID], payload1[1+LEN_OF_DID+1], payload1[1+LEN_OF_DID+2], 
			payload1[1+LEN_OF_DID+3], payload1[1+LEN_OF_DID+4]);
			free(payload1);

			//broadcast by wifi
			uint8_t payload2_len = 1 + LEN_OF_DID + 1 + LEN_OF_ZWAVE_SUBNET;
			unsigned char* payload2 = (unsigned char*)malloc(payload2_len);
			memset(payload2, 0, payload2_len);

			payload2[0] = 'b';
			copy_in_big_endian(payload2 + 1, my_ip);
			payload2[1 + LEN_OF_DID] = 'z';
			payload2[1 + LEN_OF_DID + 1] = zwave_subnet(my_zwave_addr);

			uint32_t broadcast_ip = ip_subnet(my_ip) | 0xFF;
			char broadcast_ip_str[16] = {0};
			inet_ntop(broadcast_ip_str, broadcast_ip);
			
			radio_wifi_send(broadcast_ip_str, payload2, payload2_len);
			DEBUG_LOG(DBG_WUKONG_META_ROUTING, "broadcast: %c, %u.%u.%u.%u, %c, %u\n", 
			payload2[0], payload2[1], payload2[2], payload2[3], payload2[4], payload2[1 + LEN_OF_DID], payload2[1 + LEN_OF_DID + 1]);
			free(payload2);
		#endif
	}
}

void routing_poll() {
	#ifdef RADIO_USE_ZWAVE
	radio_zwave_poll();
	#endif
	#ifdef RADIO_USE_WIFI_ARDUINO
		radio_wifi_poll();
		if(self_ip[0] == '0' || self_ip[0] == 0){
			if(dj_timer_getTimeMillis() - get_net_mask_and_ip_time > 5000){
				get_net_mask_and_ip_time = dj_timer_getTimeMillis();
				radio_wifi_get_net_mask_and_ip(self_ip, self_net_mask);
				if(self_ip[0] != '0' && self_ip[0] != 0){
					my_ip = inet_pton(self_ip);
					ip_net_mask = inet_pton(self_net_mask);
					if(I_AM_GATE){
						radio_wifi_init_succ = true;
					}else{
						radio_init_succ = true;	
					}
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nWIFI init succeed !!! self_ip = %s, self_net_mask = %s\n", 
					self_ip, self_net_mask);
				}
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nWIFI init failed !!! self_ip = %s, self_net_mask = %s\n", 
				self_ip, self_net_mask);
			}
		}
	#endif
	if (dj_timer_getTimeMillis() - routing_search_time > 3000)
	{
		if (routing_mode == GATEWAY_DISCOVERY_MODE)
		{
			routing_discover_gateway();
		}
		else if(routing_mode == ID_REQ_MODE)
		{
			routing_id_req();
		}
	}

	if(I_AM_GATE && !radio_init_succ){
		#if defined(RADIO_USE_ZWAVE) && defined(RADIO_USE_WIFI_ARDUINO)
		if(radio_wifi_init_succ && radio_zwave_init_succ) radio_init_succ = true;
		#endif
	}


	if(routing_mode == NORMAL_MODE){
		if(I_AM_GATE){
			periodic_broadcast();
		}
		periodic_check_for_qos_measure();
		periodic_check_recent_dest_to_retrieve_qos_measure();
		//periodic_qos_remeasure();
		//periodic_check_node_failure();
	}
}

void periodic_broadcast(){
	if(radio_init_succ && (dj_timer_getTimeMillis() - broadcast_time > BROADCAST_PERIOD)){
		broadcast_time = dj_timer_getTimeMillis();
		broadcast_gate_addr();
	}
}

void periodic_check_for_qos_measure(){
	//check gate list for qos test
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(gateList[i].full){
			//time
			if(gateList[i].time_test_start_time != 0){
				if((type_of_radio(gateList[i].addr) == 'z' && dj_timer_getTimeMillis() - gateList[i].time_test_start_time > 2000) ||
				(type_of_radio(gateList[i].addr) == 'w' && 
				dj_timer_getTimeMillis() - gateList[i].time_test_start_time > random_period(5, 11) * 1000)){
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u] time test failed, restart\n", i);
					gateList[i].time_test_start_time = qos_time_test(gateList[i].addr);
				}
			}

			//reli
			#ifdef RADIO_USE_WIFI_ARDUINO
			if(gateList[i].reli_test_time_type != 0){
				if(dj_timer_getTimeMillis() - gateList[i].reli_test_time > random_period(5, 11) * 1000){
					switch(gateList[i].reli_test_time_type){
						case 's':
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u] reli test start failed, restart\n", i);
							gateList[i].reli_test_time = qos_reli_test_start_wifi(gateList[i].addr);
							break;
						case 'e':
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u] reli test end failed, restart\n", i);
							gateList[i].reli_test_time = qos_reli_test_end_wifi(gateList[i].addr);
							break;
						case 'c':
							{
								DEBUG_LOG(DBG_WUKONG_META_ROUTING, 
								"\nGate[%u], clear wifi reli tester list failed, restart\n", i);
								unsigned char buf[2] = {0};
								buf[0] = 'r';
								buf[1] = 'c';
								send_by_diff_radios_depend_on_addr(gateList[i].addr, buf, sizeof(buf));
								gateList[i].reli_test_time = dj_timer_getTimeMillis();
								break;
							}
					}
				}
			}
			#endif
		}
	}
	//check recent used dest wo gate list for qos test
	for(i = 0; i < RECENT_DEST_WOG_NUM; i++){
		if(recentDestWOG[i].full){
			//time
			if(recentDestWOG[i].time_test_start_time != 0){
				if((type_of_radio(recentDestWOG[i].dest) == 'z' && 
				dj_timer_getTimeMillis() - recentDestWOG[i].time_test_start_time > 2000) ||
				(type_of_radio(recentDestWOG[i].dest) == 'w' && 
				dj_timer_getTimeMillis() - recentDestWOG[i].time_test_start_time > random_period(5, 11) * 1000)){
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest WOG[%u], time test failed, restart\n", i);
					recentDestWOG[i].time_test_start_time = qos_time_test(recentDestWOG[i].dest);
				}
			}

			//reli
			#ifdef RADIO_USE_WIFI_ARDUINO
			if(recentDestWOG[i].reli_test_time_type != 0){ 
				if(dj_timer_getTimeMillis() - recentDestWOG[i].reli_test_time > random_period(5, 11) * 1000){
					switch(recentDestWOG[i].reli_test_time_type){
						case 's':
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest WOG[%u], reli test start failed, restart\n", i);
							recentDestWOG[i].reli_test_time = qos_reli_test_start_wifi(recentDestWOG[i].dest);
							break;
						case 'e':
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest WOG[%u], reli test end failed, restart\n", i);
							recentDestWOG[i].reli_test_time = qos_reli_test_end_wifi(recentDestWOG[i].dest);
							break;
						case 'c':
							{
								DEBUG_LOG(DBG_WUKONG_META_ROUTING, 
								"\nDest WOG[%u], clear wifi reli tester list failed, restart\n", i);
								unsigned char buf[2] = {0};
								buf[0] = 'r';
								buf[1] = 'c';
								send_by_diff_radios_depend_on_addr(recentDestWOG[i].dest, buf, sizeof(buf));
								recentDestWOG[i].reli_test_time = dj_timer_getTimeMillis();
								break;
							}
					}
				}
			}
			#endif
		}
	}
}

void periodic_check_recent_dest_to_retrieve_qos_measure(){
	if(dj_timer_getTimeMillis() - retrieve_qos_measure_time > 10000){
		retrieve_qos_measure_time = dj_timer_getTimeMillis();
		uint8_t i;
		for(i = 0; i < RECENT_DEST_NUM; i++){
			if(recentDest[i].full){
				uint8_t j;
				for(j = 0; j < GATE_NUM; j++){
					if(recentDest[i].gates[j].full && recentDest[i].gates[j].same_radio_as_dest &&
					recentDest[i].gates[j].time == 0 && recentDest[i].gates[j].reli == 0){
						DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nSend retrieve QoS msg!!!\n");
						unsigned char* buf = (unsigned char*)malloc(1 + LEN_OF_DID + LEN_OF_DID);
						memset(buf, 0, 1 + LEN_OF_DID + LEN_OF_DID);

						buf[0] = 's';
						copy_in_big_endian(buf + 1, recentDest[i].dest);
						if(type_of_radio(gateList[j].addr) == 'z'){
							#ifdef RADIO_USE_ZWAVE
							copy_in_big_endian(buf + 1 + LEN_OF_DID, my_zwave_addr);
							#endif
						}else if(type_of_radio(gateList[j].addr) == 'w'){
							#ifdef RADIO_USE_WIFI_ARDUINO
							copy_in_big_endian(buf + 1 + LEN_OF_DID, my_ip);
							#endif
						}
						send_by_diff_radios_depend_on_addr(gateList[j].addr, buf, 1 + LEN_OF_DID + LEN_OF_DID);
						free(buf);
					}
				}
			}
		}
		//debug_msg();	
	}
}

void periodic_qos_remeasure(){
	if(dj_timer_getTimeMillis() - qos_remeasure_time > qos_remeasure_period * 60000){//remeasure QoS every 5 ~ 10 minutes 
		qos_remeasure_period = random_period(5, 6);
		qos_remeasure_time = dj_timer_getTimeMillis();
		uint8_t i;
		for(i = 0; i < GATE_NUM; i++){
			if(gateList[i].full){
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nTime test start to gate[%u]\n", i);
				gateList[i].time_test_start_time = qos_time_test(gateList[i].addr);
				if(type_of_radio(gateList[i].addr) == 'z'){
					#ifdef RADIO_USE_ZWAVE
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to ZWave gate[%u]\n", i);
					gateList[i].reli = qos_reli_test_zwave(gateList[i].addr);
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test end to ZWave gate[%u]\n", i);
					#endif	
				}else if(type_of_radio(gateList[i].addr) == 'w'){
					#ifdef RADIO_USE_WIFI_ARDUINO
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nReli test start to WIFI gate[%u]\n", i);
					gateList[i].reli_test_time = qos_reli_test_start_wifi(gateList[i].addr);
					gateList[i].reli_test_time_type = 's';
					#endif	
				}
			}
		}
	}
}

void periodic_check_node_failure(){
	if(dj_timer_getTimeMillis() - check_node_failure_time > 120000){//check whether the gateways fails every 2 minutes
		check_node_failure_time = dj_timer_getTimeMillis();
		uint8_t i;
		for(i = 0; i < GATE_NUM; i++){
			if(gateList[i].full){
				if(gateList[i].recv_broadcast){
					gateList[i].recv_broadcast = false;
				}else{
					//delete all info about this gate
					DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u] failed.\n", i);
					memset(gateList + i, 0, sizeof(struct GATE_LIST));
					uint8_t j;
					for(j = 0; j < RECENT_DEST_NUM; j++){
						if(recentDest[j].full && recentDest[j].gates[i].full){
							if(recentDest[j].gates[i].reliWithTime != NULL) free(recentDest[j].gates[i].reliWithTime);
							memset(recentDest[j].gates + i, 0, sizeof(struct THROUGH_GATES));
						}
					}
				}
			}
		}
	}
}

//Followings are tool functions 
#ifdef RADIO_USE_ZWAVE
radio_zwave_address_t addr_wkcomm_to_zwave(wkcomm_address_t  wkcomm_addr) {
    return (radio_zwave_address_t)(GET_ID_RADIO_ADDRESS_ZWAVE(wkcomm_addr));
}
wkcomm_address_t  addr_zwave_to_wkcomm(radio_zwave_address_t zwave_addr) {
    wkcomm_address_t  wkcomm_addr = GET_ID_PREFIX_ZWAVE(my_zwave_addr) | zwave_addr;
    return wkcomm_addr;
}
#endif

#ifdef RADIO_USE_WIFI_ARDUINO
bool find_match_in_reli_tester_list(char* ip, uint8_t* i){
	for(*i = 0; *i < RELI_TESTER_NUM; (*i)++){
		if(strcmp(ip, reliTesterList[*i].ip) == 0){
			return true;
		}
	}
	return false;
}

bool find_avail_in_reli_tester_list(char* ip, uint8_t* i){
	for(*i = 0; *i < RELI_TESTER_NUM; (*i)++){
		if(strcmp("", reliTesterList[*i].ip) == 0){
			memcpy(reliTesterList[*i].ip, ip, strlen(ip));
			return true;
		}
	}
	return false;
}
#endif

uint32_t inet_pton(char* ip_str){
	uint8_t i, num_of_bits = 0;
	unsigned char byte_c[3] = {0};
	uint32_t ip_num = 0;

	for(i = 0; i <= strlen(ip_str); i++){
		if(ip_str[i] == '.' || ip_str[i] == 0){
			ip_num *= 256;
			switch(num_of_bits){
				case 1:
					ip_num += (byte_c[0] - 48);
					break;
				case 2:
					ip_num += (byte_c[0] - 48) * 10 + (byte_c[1] - 48);
					break;
				case 3:
					ip_num += (byte_c[0] - 48) * 100 + (byte_c[1] - 48) * 10 + (byte_c[2] - 48);
					break;
			}
			memset(byte_c, 0, sizeof(byte_c));
			num_of_bits = 0;
		}else{
			byte_c[num_of_bits++] = (unsigned char)ip_str[i];
		}

	}
	return ip_num;
}

void inet_ntop(char* ip_str, uint32_t ip)
{
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	sprintf(ip_str, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]); 
}

uint8_t zwave_subnet(uint32_t wkcomm_addr){
	return (uint8_t)(wkcomm_addr >> 8 & 0xFF);
}

uint32_t ip_subnet(uint32_t wkcomm_addr){
	return (uint32_t)(wkcomm_addr & ip_net_mask);
}

bool same_as_used_zwave_nets(uint8_t zwave_net_id, unsigned char* used_nets){
	uint8_t wifi_net_num = used_nets[1];
	uint8_t zwave_net_num = used_nets[3];
	uint8_t start_index = 4 + wifi_net_num * 4;
	uint8_t end_index = start_index + zwave_net_num * sizeof(zwave_net_id);
	uint8_t i;

	if(start_index < end_index){
		for(i = start_index; i < end_index; i++){
			if(zwave_net_id == used_nets[i]){
				return true;
			}
		}
	}
	return false;
}

bool same_as_used_wifi_nets(uint32_t wifi_net_id, unsigned char* used_nets){
	uint8_t wifi_net_num = used_nets[1];
	uint8_t start_index = 4;
	uint8_t end_index = start_index + wifi_net_num * sizeof(wifi_net_id);
	uint8_t i;

	if(start_index < end_index){
		for(i = start_index; i < end_index; i+=4){
			uint32_t used_net_id;
			retrieve_in_big_endian(used_nets + i, &used_net_id);
			if(wifi_net_id == used_net_id){
				return true;
			}
		}
	}
	return false;
}

bool find_empty_index_in_recent_dest(uint8_t* index){
	uint8_t i;
	for(i = 0; i < RECENT_DEST_NUM; i++){
		if(recentDest[i].full == false){
			*index = i;
			return true;
		}
	}
	return false;
}

bool find_empty_index_in_recent_dest_wog(uint8_t* index){
	uint8_t i;
	for(i = 0; i < RECENT_DEST_WOG_NUM; i++){
		if(recentDestWOG[i].full == false){
			*index = i;
			return true;
		}
	}
	return false;
}

bool find_empty_index_in_gate_list(uint8_t* index){
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(gateList[i].full == false){
			*index = i;
			return true;
		}
	}
	return false;
}

bool find_dest_index_in_recent_dest(uint32_t dest, uint8_t* index){
	uint8_t i;
	for(i = 0; i < RECENT_DEST_NUM; i++){
		if(recentDest[i].full && dest == recentDest[i].dest){
			*index = i;
			return true;
		}
	}
	return false;
}

bool find_dest_index_in_recent_dest_wog(uint32_t dest, uint8_t* index){
	uint8_t i;
	for(i = 0; i < RECENT_DEST_WOG_NUM; i++){
		if(recentDestWOG[i].full && dest == recentDestWOG[i].dest){
			*index = i;
			return true;
		}
	}
	return false;
}

bool find_gate_index_in_gate_list(uint32_t wkcomm_addr, uint8_t* index){
	uint8_t i;
	for(i = 0; i < GATE_NUM; i++){
		if(gateList[i].full && wkcomm_addr == gateList[i].addr){
			*index = i;
			return true;
		}
	}
	return false;
}

void find_and_set_min_time_to_dest(uint8_t dest_index){			
	uint8_t k;
	uint32_t min_time_to_dest = 0;
	for(k = 0; k < GATE_NUM; k++){
		if(recentDest[dest_index].gates[k].time != 0){
			if(min_time_to_dest == 0 || 
			(min_time_to_dest > gateList[k].time + recentDest[dest_index].gates[k].time)){
				min_time_to_dest = gateList[k].time + recentDest[dest_index].gates[k].time;
			}
		}
	}
	recentDest[dest_index].time = min_time_to_dest;
}

uint32_t diff_of_two_num(uint32_t time_a, uint32_t time_b){
	if(time_a >= time_b){
		return time_a - time_b;
	}else{
		return time_b - time_a;
	}
}

unsigned char type_of_radio(uint32_t addr){
	unsigned char leftest_byte = LEFTEST_BYTE(addr);
	unsigned char radio = 0;
	if(leftest_byte == 0){
		radio = 'z';
	}else{
		radio = 'w';
	}
	return radio;
}

void send_by_diff_radios_depend_on_addr(uint32_t addr, uint8_t* payload, uint8_t length){
	if(type_of_radio(addr) == 'z'){
		#ifdef RADIO_USE_ZWAVE
		radio_zwave_send(addr_wkcomm_to_zwave(addr), payload, length);
		#endif
	}else if(type_of_radio(addr) == 'w'){
		#ifdef RADIO_USE_WIFI_ARDUINO
		char ip_str[16] = {0};
		inet_ntop(ip_str, addr);
		radio_wifi_send(ip_str, payload, length);
		#endif
	}
}

void copy_in_big_endian(unsigned char* addr_index, uint32_t wkcomm_addr){
	addr_index[0] = wkcomm_addr >> 24 & 0xFF;
	addr_index[1] = wkcomm_addr >> 16 & 0xFF;
	addr_index[2] = wkcomm_addr >> 8 & 0xFF;
	addr_index[3] = wkcomm_addr & 0xFF;
}

void retrieve_in_big_endian(unsigned char* addr_index, uint32_t* wkcomm_addr){
	*wkcomm_addr = addr_index[0];
	*wkcomm_addr <<= 8;
	*wkcomm_addr |= addr_index[1];
	*wkcomm_addr <<= 8;
	*wkcomm_addr |= addr_index[2];
	*wkcomm_addr <<= 8;
	*wkcomm_addr |= addr_index[3];
}

uint8_t random_period(uint8_t start, uint8_t range){
	return (start + (rand() % range));
}

void debug_msg(){
	uint8_t k;
	//print gate list
	for(k = 0; k < GATE_NUM; k++){
		if(gateList[k].full){
			if(type_of_radio(gateList[k].addr) == 'z'){
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u]: ZWave Addr = %u:%u, time = %lu, reli = %u, ", 
				k, zwave_subnet(gateList[k].addr), (uint8_t)(gateList[k].addr & 0x000000FF) ,gateList[k].time, gateList[k].reli);
			}else if(type_of_radio(gateList[k].addr) == 'w'){
				char ip_debug[16] = {0};
				inet_ntop(ip_debug, gateList[k].addr);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nGate[%u]: WIFI Addr = %s, time = %lu, reli = %u, ", 
				k, ip_debug, gateList[k].time, gateList[k].reli);
			}
			if(gateList[k].another_radio.use_zwave){
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "another radio ZWave = %u\n", gateList[k].another_radio.zwave_net_id);
			}else{
				char ip_debug[16] = {0};
				inet_ntop(ip_debug, gateList[k].another_radio.wifi_net_id);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "aother radio WIFI = %s\n", ip_debug);
			}
		}
	}
	//print recent dest
	for(k = 0; k < RECENT_DEST_NUM; k++){
		if(recentDest[k].full){
			if(type_of_radio(recentDest[k].dest) == 'z'){
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest[%u]: ZWave Addr = %u:%u, time_to_dest = %lu\n",
				k, zwave_subnet(recentDest[k].dest), (uint8_t)(recentDest[k].dest & 0x000000FF) ,recentDest[k].time);
			}else if(type_of_radio(recentDest[k].dest) == 'w'){
				char ip_debug[16] = {0};
				inet_ntop(ip_debug, recentDest[k].dest);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest[%u]: WIFI Addr = %s, time_to_dest = %lu\n", 
				k, ip_debug, recentDest[k].time);
			}
			uint8_t l;
			for(l = 0; l < GATE_NUM; l++){
				if(recentDest[k].gates[l].full){
					if(recentDest[k].gates[l].same_radio_as_dest){
						if(recentDest[k].gates[l].time != 0){//reli might be 0
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "*** Gate[%u]: time = %lu, reli = %u\n", 
							l, gateList[l].time + recentDest[k].gates[l].time, 
							(gateList[l].reli * recentDest[k].gates[l].reli) / 100);
						}
					}else{	
						if(recentDest[k].gates[l].time != 0)
							DEBUG_LOG(DBG_WUKONG_META_ROUTING, "### Gate[%u]: time = %lu\n", 
							l, gateList[l].time + recentDest[k].gates[l].time);
						if(recentDest[k].gates[l].reliWithTime != NULL){
							uint8_t m;
							for(m = 0; m < RWT_NUM; m++){
								if(recentDest[k].gates[l].reliWithTime[m].time_restrict != 0){
									DEBUG_LOG(DBG_WUKONG_META_ROUTING, "             time = %lu, max_reli = %u\n",
									recentDest[k].gates[l].reliWithTime[m].time_restrict,
									recentDest[k].gates[l].reliWithTime[m].max_reli);
								}
							}
						}
					}
				}
			}
		}
	}	
	//print recent dest wog
	for(k = 0; k < RECENT_DEST_WOG_NUM; k++){
		if(recentDestWOG[k].full){
			if(type_of_radio(recentDestWOG[k].dest) == 'z'){
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest WOG[%u]: ZWave Addr = %u:%u, time = %lu, reli = %u\n", 
				k, zwave_subnet(recentDestWOG[k].dest), (uint8_t)(recentDestWOG[k].dest & 0x000000FF) ,
				recentDestWOG[k].time, recentDestWOG[k].reli);
			}else if(type_of_radio(recentDestWOG[k].dest) == 'w'){
				char ip_debug[16] = {0};
				inet_ntop(ip_debug, recentDestWOG[k].dest);
				DEBUG_LOG(DBG_WUKONG_META_ROUTING, "\nDest WOG[%u]: WIFI Addr = %s, time = %lu, reli = %u\n", 
				k, ip_debug, recentDestWOG[k].time, recentDestWOG[k].reli);
			}
		}
	}
}

// MY NODE ID
// Get my own node id
wkcomm_address_t routing_get_node_id() {
    // return wkpf_config_get_myid();
    #ifdef RADIO_USE_ZWAVE
        return addr_zwave_to_wkcomm(radio_zwave_get_node_id());
    #endif
}

// SENDING
uint8_t routing_send(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
    return 0;
}

uint8_t routing_send_raw(wkcomm_address_t dest, uint8_t *payload, uint8_t length) {
    #ifdef RADIO_USE_ZWAVE
        return radio_zwave_send_raw(addr_wkcomm_to_zwave(dest), payload, length);
    #endif
    return 0;
}

void routing_discover_gateway()
{
    // DEBUG_LOG(DBG_WKROUTING, "routing discover gateway_id before=%d\n",id_table.gateway_id);
    uint8_t rt_payload[MPTN_PAYLOAD_BYTE_OFFSET]; //Autonet MAC address
    uint8_t i;
    char ipstr[IP_ADDRSTRLEN];
    if (routing_mode != GATEWAY_DISCOVERY_MODE)
    {
        routing_mode = GATEWAY_DISCOVERY_MODE;
        id_table.my_id = routing_get_node_id();
        wkpf_config_set_myid(id_table.my_id);
        id_table.gateway_id = 0;
        wkpf_config_set_gwid(id_table.gateway_id);
        for (i = 0; i < MPTN_UUID_LEN; ++i)
        {
            id_table.uuid[i] = 0;
        }
        wkpf_config_set_uuid(id_table.uuid);
        inet_ntop(ipstr, id_table.gateway_id);
        DEBUG_LOG(DBG_WKROUTING, "r_discover: gw_id=%s ", ipstr);
        inet_ntop(ipstr, id_table.my_id);
        DEBUG_LOG(DBG_WKROUTING, "my_id=%s\n", ipstr);
    }
    for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0xFF;
    }
    wkcomm_address_t temp_id;
    for (temp_id = id_table.my_id, i = 0; i < MPTN_ID_LEN; ++i)
    {
        rt_payload[MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN-1-i] = temp_id & 0xFF;
        temp_id >>= 8;
    }
    rt_payload[MPTN_MSGTYPE_BYTE_OFFSET] = MPTN_MSGTYPE_GWDISCOVER;
    routing_search_time = dj_timer_getTimeMillis();
#ifdef RADIO_USE_ZWAVE
    radio_zwave_send(addr_wkcomm_to_zwave(scan_id++), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
    if ((scan_id & 1) ==1) radio_zwave_send(addr_wkcomm_to_zwave(BROADCAST_ADDRESS), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET);
#endif
}

void routing_get_mac_address()
{
    // could get MAC address as lower 8 bytes
    // uint8_t uart_data[MPTN_UUID_LEN];
    // for (uint8_t i = 0; i < MPTN_MAC_LEN; ++i)
    // {
    //     id_table.uuid[i] = uart_data[i];
    // }
}

void routing_id_req()    //send ID request
{
    char ipstr[IP_ADDRSTRLEN];
    if (routing_mode != ID_REQ_MODE)
    {
        inet_ntop(ipstr, id_table.gateway_id);
        DEBUG_LOG(DBG_WKROUTING, "r_idreq: gw_id=%s ", ipstr);
        inet_ntop(ipstr, id_table.my_id);
        DEBUG_LOG(DBG_WKROUTING, "my_id=%s\n", ipstr);
        routing_mode = ID_REQ_MODE;
    }
    uint8_t rt_payload[MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN];
    uint8_t i;
    for (i = MPTN_DEST_BYTE_OFFSET; i < MPTN_DEST_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0;
    }
    for (i = MPTN_SRC_BYTE_OFFSET; i < MPTN_SRC_BYTE_OFFSET+MPTN_ID_LEN; ++i)
    {
        rt_payload[i] = 0xFF;
    }
    rt_payload[MPTN_MSGTYPE_BYTE_OFFSET] = MPTN_MSGTYPE_IDREQ;
    for (i = 0; i < MPTN_UUID_LEN; ++i)
    {
        rt_payload[i+MPTN_PAYLOAD_BYTE_OFFSET] = id_table.uuid[i];
    }
    routing_search_time = dj_timer_getTimeMillis();
#ifdef RADIO_USE_ZWAVE
    radio_zwave_send(addr_wkcomm_to_zwave(id_table.gateway_id), rt_payload, MPTN_PAYLOAD_BYTE_OFFSET + MPTN_UUID_LEN);
#endif
}

#endif // ROUTING_USE_GATEWAY


