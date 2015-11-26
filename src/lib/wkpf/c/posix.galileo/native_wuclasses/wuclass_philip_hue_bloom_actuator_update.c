#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "philip_hue_utils.h"

#define MESSAGE_SIZE 1024
int8_t hla_gamma = -1;

void wuclass_philip_hue_bloom_actuator_setup(wuobject_t *wuobject)
{
}

void wuclass_philip_hue_bloom_actuator_update(wuobject_t *wuobject)
{

    bool on=false;
    uint32_t ip;
    int16_t index=0, r=0, g=0, b=0;
    float x, y;

    char message[MESSAGE_SIZE] = {0}, str[150] = {0};
    char *path = "api/newdeveloper/lights/%d/state";

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_IP_HIGH, &index);
    ip = (index & 0xffff) << 16;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_IP_LOW, &index);
    ip |= (index & 0xffff);

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_INDEX, &index);
    sprintf(str, path, index);

    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_ON, &on);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_RED, &r);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_GREEN, &g);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_ACTUATOR_BLUE, &b);

    static uint32_t currenttime, lasttime;
    uint16_t loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
    
    if (currenttime - lasttime > loop_rate){
        if (hla_gamma < 0){
            hla_gamma = get_gamma(ip, message, MESSAGE_SIZE, str, NULL, NULL, NULL, NULL);
            if (hla_gamma < 0) {
                DEBUG_LOG(DBG_WKPFUPDATE, "\nFailded to get Gamma with error %d\n", hla_gamma);
                lasttime = currenttime;
                return;
            }
        }
        RGBtoXY(hla_gamma, r, g, b, &x, &y);
        char *tpl = "PUT /%s HTTP/1.1\r\nConnection: close\r\n\r\n";
        memset(message, 0, MESSAGE_SIZE);
        sprintf(message, tpl, str);
        int ret = 0;

        if(on)
        {
            char *command_on = "{\"on\":true, \"x\":%f, \"y\":%f}";
            sprintf(message+strlen(message), command_on, x, y);
            DEBUG_LOG(DBG_WKPFUPDATE, "\nOOOOOOOOONNNNNNN%s\n", message);
        }else{
            char *command_off = "{\"on\":false}";
            strcat(message+strlen(message), command_off);
            DEBUG_LOG(DBG_WKPFUPDATE, "\nofffffffffffffff%s\n", message);
        }
        ret = socket_send_to(ip, message, strlen(message), message, MESSAGE_SIZE);

        if (!ret){
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Bloom): Setting red to: %d\n", r);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Bloom): Setting green to: %d\n", g);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Bloom): Setting blue to: %d\n", b);   
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Bloom): Setting on_off to: %x\n", on);
        } else {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Bloom): Error! %d\n", ret);
        }
        lasttime = currenttime;
    }
}
