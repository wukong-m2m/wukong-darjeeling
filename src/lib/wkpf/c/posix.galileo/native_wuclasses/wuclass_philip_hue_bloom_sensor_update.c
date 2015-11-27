#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "philip_hue_utils.h"

#define MESSAGE_SIZE 1024

void wuclass_philip_hue_bloom_sensor_setup(wuobject_t *wuobject)
{
}

void wuclass_philip_hue_bloom_sensor_update(wuobject_t *wuobject)
{

    bool on=false;
    uint32_t ip;
    int16_t index=0;
    int8_t gamma;
    float x, y, bri;
    uint8_t r, g, b;

    char message[MESSAGE_SIZE] = {0}, str[150] = {0};
    char *path = "api/newdeveloper/lights/%d/state";

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_IP_HIGH, &index);
    ip = (index & 0xffff) << 16;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_IP_LOW, &index);
    ip |= (index & 0xffff);

    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_INDEX, &index);
    sprintf(str, path, index);

    static uint32_t currenttime, lasttime;
    uint16_t loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
    
    if (currenttime - lasttime > loop_rate){
        gamma = get_gamma(ip, message, MESSAGE_SIZE, str, &x, &y, &bri, &on);
        if (gamma < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "\nFailded to get Gamma with error %d\n", gamma);
            lasttime = currenttime;
            return;
        }
        XYbtoRGB(gamma, x, y, bri, &r, &g, &b);        
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_RED, r);    
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_GREEN, g);    
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_BLOOM_SENSOR_BLUE, b);   
        if (!on) bri = 0;
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_GO_SENSOR_ON, bri); 
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Go_Sensor): red: %d\n", r);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Go_Sensor): green: %d\n", g);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Go_Sensor): blue: %d\n", b);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Go_Sensor): bri: %d\n", bri);
        lasttime = currenttime;
    }
}
