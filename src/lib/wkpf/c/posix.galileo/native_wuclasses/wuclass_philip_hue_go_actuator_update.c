#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "philip_hue_utils.h"

int8_t gamma_hue_go = -1;

void wuclass_philip_hue_go_actuator_setup(wuobject_t *wuobject)
{
}

void wuclass_philip_hue_go_actuator_update(wuobject_t *wuobject)
{
	bool on=false;
    uint32_t ip;
    int16_t index=0, r=0, g=0, b=0;
    char *debug_name = "Philip_HUE_GO_Actuator";
    int8_t *gamma = &gamma_hue_go;

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_IP_HIGH, &index);
    ip = (index & 0xffff) << 16;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_IP_LOW, &index);
    ip |= (index & 0xffff);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_INDEX, &index);
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_ON, &on);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_RED, &r);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_GREEN, &g);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_ACTUATOR_BLUE, &b);


    static uint32_t currenttime, lasttime;
    uint16_t loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
	
    if (currenttime - lasttime > loop_rate){
        float x, y, bri;
        char message[MESSAGE_SIZE] = {0}, command[BUF_SIZE] = {0};
        if (*gamma < 0){
            *gamma = get_gamma(ip, message, MESSAGE_SIZE, index, NULL, NULL, NULL, NULL);
            if (*gamma < 0) {
                DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____GET gamma error:%d\n", debug_name, *gamma);
                if (*gamma < -99){
                    char *tmp = strstr(message, "\r\n\r\n")+4;
                    DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____JSON error:%s\n", debug_name, tmp);
                }
                lasttime = currenttime;
                return;
            }
        }

        RGBtoXY(*gamma, r, g, b, &x, &y, &bri);
    	if(on)
    	{
            sprintf(command, "{\"on\":true,\"xy\":[%.4f,%.4f],\"bri\":%d}", x, y, (int)(bri*255.0));
    	}else{
            sprintf(command, "{\"on\":false}");
        }
        dj_timer_delay(50);
        DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____PUT command:%s\n", debug_name, command);
        int ret = put_command(ip, message, MESSAGE_SIZE, index, command, strlen(command));

        if (!ret){
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): Setting red to: %d\n", debug_name, r);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): Setting green to: %d\n", debug_name, g);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): Setting blue to: %d\n", debug_name, b);   
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): Setting on_off to: %x\n", debug_name, on);
        } else {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): Error! %d\n", debug_name, ret);
        }
        lasttime = currenttime;
    }
}
