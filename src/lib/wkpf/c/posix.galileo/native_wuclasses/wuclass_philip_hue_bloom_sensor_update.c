#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "philip_hue_utils.h"

void wuclass_philip_hue_bloom_sensor_setup(wuobject_t *wuobject)
{
}

void wuclass_philip_hue_bloom_sensor_update(wuobject_t *wuobject)
{
    bool on=false;
    uint32_t ip;
    int16_t index=0;
    int8_t gamma;
    float x, y;
    int bri;
    uint8_t r, g, b;
    char *debug_name = "Philip_HUE_BLOOM_Sensor";

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_IP_HIGH, &index);
    ip = (index & 0xffff) << 16;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_IP_LOW, &index);
    ip |= (index & 0xffff);

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_INDEX, &index);

    static uint32_t currenttime, lasttime;
    uint16_t loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
    
    if (currenttime - lasttime > loop_rate){
        char message[MESSAGE_SIZE] = {0};
        gamma = get_gamma(ip, message, MESSAGE_SIZE, index, &x, &y, &bri, &on);
        if (gamma < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____GET gamma error:%d\n", debug_name, gamma);
            if (gamma < -99){
                char *tmp = strstr(message, "\r\n\r\n")+4;
                DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____JSON error:%s\n", debug_name, tmp);
            } else {
                    DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s____Error!ip:%u,index:%d\n", debug_name, ip, index);
                }
            lasttime = currenttime;
            return;
        }
        XYbtoRGB(gamma, x, y, (float)(bri)/255.0, &r, &g, &b);        
        // wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_RED, r);    
        // wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_GREEN, g);    
        uint16_t rg = (r << 8) | (g & 0xFF);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_RED_GREEN, rg);        
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_BLUE, b);
        if (!on) bri = 0;
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_BRI, bri); 
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): red: %d\n", debug_name, r);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): green: %d\n", debug_name, g);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): blue: %d\n", debug_name, b);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): bri: %d\n", debug_name, bri);
        lasttime = currenttime;
    }
}
