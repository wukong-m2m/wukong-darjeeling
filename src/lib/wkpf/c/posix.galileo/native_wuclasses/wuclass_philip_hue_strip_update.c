#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "HSVtoRGB.h"

void wuclass_philip_hue_strip_setup(wuobject_t *wuobject) {
	
}

void wuclass_philip_hue_strip_update(wuobject_t *wuobject) {

	static bool on=false;
	static int16_t bri=255;
	static int16_t hue=255;
    // bool previous_on_state = on;
    // int previous_hue = hue, previous_bri = bri;

	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_ON, &on);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_BRI, &bri);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_HUE, &hue);
	int16_t scale = 65535/255;
	int colorvalue = hue * scale;
    static uint32_t currenttime, lasttime, command_delay = 50, loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
	static char str[150];
    char command[3][126] = {"curl -X PUT --data '{\"on\":true, \"sat\":254, \"bri\":%d, \"hue\":%d}' http://192.168.4.124/api/newdeveloper/lights/%d/state", "curl -X PUT --data '{\"on\":false}' http://192.168.4.124/api/newdeveloper/lights/%d/state", "curl -X PUT --data '{\"sat\":254, \"bri\":%d, \"hue\":%d}' http://192.168.4.124/api/newdeveloper/lights/%d/state"};
	
    if (currenttime - lasttime > loop_rate){
        int i, start_index = 12, end_index = 12;
        bool state_change = false;
    	if(on)
    	{
            for (i = start_index; i <= end_index; ++i){
                sprintf(str, command[0], bri, colorvalue, i);
                DEBUG_LOG(DBG_WKPFUPDATE, "\n^^^^^^^^^^^^^^^^^%s\n", str);
                system(str);
                dj_timer_delay(command_delay);
                state_change = true;
            }
    	}else{
            for (i = start_index; i <= end_index; ++i){
                sprintf(str, command[1], i);
                DEBUG_LOG(DBG_WKPFUPDATE, "\n!!!!!!!!!!!!!!!!!%s\n", str);
                system(str);
                dj_timer_delay(command_delay);
            }
        }


        float r=0, g=0, b=0, h=0, s=0, v=0;
        if (state_change){
            h = (hue/255.0)*360;
            s = 1;
            v = bri/255.0;
            HSVtoRGB(&r, &g, &b, h, s, v);
            wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_RED, (int)(r*255));    
            wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_GREEN, (int)(g*255));    
            wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_PHILIP_HUE_STRIP_BLUE, (int)(b*255)); 
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting brightness to: %d\n", bri);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting hue_color to: %d\n", hue);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting hue_red to: %d\n", (int)(r*255));
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting hue_green to: %d\n", (int)(g*255));
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting hue_blue to: %d\n", (int)(b*255));   
        }
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Philip_Hue_Strip): Setting on_off to: %x\n", on);
        lasttime = currenttime;
    }
}
