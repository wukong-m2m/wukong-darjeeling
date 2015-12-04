#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "djtimer.h"
#include "smartthings.h"
#include "stconfig.h"

void wuclass_st_presence_setup(wuobject_t *wuobject)
{
}

void wuclass_st_presence_update(wuobject_t *wuobject)
{
    bool presence=false;
    char *debug_name = "ST_PRESENCE";
    char *retrived_status_name = "presence";

    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_ST_PRESENCE_PRESENCE, &presence);

    static uint32_t currenttime, lasttime;
    uint16_t loop_rate = 500;
    currenttime = dj_timer_getTimeMillis();
    
    if (currenttime - lasttime > loop_rate){
        char message[MESSAGE_SIZE] = {0}, status[BUF_SIZE] = {0};
        strcpy(status, retrived_status_name);
        int ret = getStatus(message, MESSAGE_SIZE, APP_ID, ACCESS_TOKEN, PRESENCE_DEV_ID, status, BUF_SIZE);
        if (ret < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____GET status error:%d\n", debug_name, ret);
            if (ret < -99){
                char *tmp = strstr(message, "\r\n\r\n")+4;
                DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____JSON error:%s\n", debug_name, tmp);
            }
            lasttime = currenttime;
            return;
        }
        if (strcmp(status, "present") && strcmp(status, "not present")){
            DEBUG_LOG(DBG_WKPFUPDATE, "\n_____%s_____wrong status:%s\n", debug_name, status);
            lasttime = currenttime;
            return;
        }
        presence = (!strcmp(status, "present"))?true:false;
        wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_ST_SWITCH_ON_OFF_STATE, presence);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(%s): presence_state:%s\n", debug_name, status);
        lasttime = currenttime;
    }
}
