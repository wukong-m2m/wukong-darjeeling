#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <mraa.h>

mraa_uart_context ics_uart;
int ics_ttyFd;
int ics_counter;
char ics_buffer[30];

bool wuclass_ics_user_dataAvailable(unsigned int millis){
    if (ics_ttyFd == -1){return false;}
    struct timeval timeout;
    // no waiting
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    // int nfds; //unknown
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(ics_ttyFd, &readfds);
    if (select(ics_ttyFd + 1, &readfds, NULL, NULL, &timeout) > 0)
    {
        return true;                // data is ready
    }else{
        return false;
    }
}


void wuclass_ics_user_setup(wuobject_t *wuobject) {
    ics_ttyFd = -1;
    ics_counter = 0;
    ics_buffer[0] = 0;
    if ( !(ics_uart = mraa_uart_init(0)) ){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ics): uart failure\n");                
        return;
    }
    const char *devPath = mraa_uart_get_dev_path(ics_uart);

    if (!devPath){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ics): uart failure\n");                
        return;
    }
    if ( (ics_ttyFd = open(devPath, O_RDWR)) == -1){
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ics): uart failure\n");                
        return;
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ics): uart ready\n");                

}

void wuclass_ics_user_update(wuobject_t *wuobject) {
    char buffer_rev[30]={};
    bool available = wuclass_ics_user_dataAvailable(1000);
    int slipper[3];
    int holder;
    int len;
    if(available){                                       
        int x = read(ics_ttyFd, buffer_rev, 30);
        if (x < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ICS): uart error\n");
            return; 
        }
        len = strlen(ics_buffer);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ICS): size: %d buffer: %s\n",x , buffer_rev); 
        int i;
        for (i = 0; i < x; i++) {
            if (buffer_rev[i] == '#') ics_counter++;
            else if (buffer_rev[i] == '@') ics_counter--;
            else if (ics_counter == 2) {
                ics_buffer[len++] = buffer_rev[i];
                ics_buffer[len] = 0;
            }
            if (ics_counter == 0 && strlen(ics_buffer)) {
                DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ICS): buffer %s\n", ics_buffer);
                sscanf(ics_buffer, "%d,%d,%d,%d", &holder, &slipper[0], &slipper[1], &slipper[2]);
                wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ICS_USER_HOLDER, holder);
                wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ICS_USER_SLIPPER0, slipper[0]);
                wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ICS_USER_SLIPPER1, slipper[1]);
                wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ICS_USER_SLIPPER2, slipper[2]);
                DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(ICS): output %d, %d, %d, %d\n", holder, slipper[0], slipper[1], slipper[2]);
                ics_buffer[0] = 0;
                len = 0;
            }
        }
    }
}
