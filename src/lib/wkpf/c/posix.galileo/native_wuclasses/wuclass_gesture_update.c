#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <math.h>
#include "config.h"
#include "mraa.h"

mraa_uart_context gesture_uart;
int gesture_ttyFd;

bool wuclass_gesture_id_dataAvailable(unsigned int millis){
    if (gesture_ttyFd == -1){return false;}
    struct timeval timeout;
    // no waiting
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    // int nfds; //unknown
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(gesture_ttyFd, &readfds);
    if (select(gesture_ttyFd + 1, &readfds, NULL, NULL, &timeout) > 0)
    {
        return true;                // data is ready
    }else{
        return false;
    }
}


void wuclass_gesture_setup(wuobject_t *wuobject) {
    gesture_ttyFd = -1;
    if ( !(gesture_uart = mraa_uart_init(0)) ){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): uart failure\n");                
        return;
    }
    const char *devPath = mraa_uart_get_dev_path(gesture_uart);

    if (!devPath){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): uart failure\n");                
        return;
    }
    if ( (gesture_ttyFd = open(devPath, O_RDWR)) == -1){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): uart failure\n");                
        return;
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): uart ready\n");                

}

void wuclass_gesture_update(wuobject_t *wuobject) {
    char buffer_rev[30]={};
    bool available = wuclass_gesture_id_dataAvailable(1000);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): available: %d\n", available);                
    if(available){                                       
        int x = read(gesture_ttyFd, buffer_rev, 30);
        if (x < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): uart error\n");
            return; 
        }
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): size: %d buffer: %s\n",x , buffer_rev); 
        if (x == 30) {
            int i;
            for (i = 0; i < 30; i++) 
                DEBUG_LOG(DBG_WKPFUPDATE, "%d ", buffer_rev[i]);
            DEBUG_LOG(DBG_WKPFUPDATE, "\n"); 
        } 
        int16_t output = (int16_t)atoi(buffer_rev);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_USER_USER, output);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture): output %d\n", output);
    }
}
