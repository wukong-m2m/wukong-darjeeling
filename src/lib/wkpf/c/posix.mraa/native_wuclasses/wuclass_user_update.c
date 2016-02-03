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

#ifdef MRAA_LIBRARY
#include <mraa.h>

mraa_uart_context user_uart;
int user_ttyFd;

bool wuclass_user_id_dataAvailable(unsigned int millis){
  if (user_ttyFd == -1){return false;}
  struct timeval timeout;
  // no waiting
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  // int nfds; //unknown
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(user_ttyFd, &readfds);
  if (select(user_ttyFd + 1, &readfds, NULL, NULL, &timeout) > 0)
  {
    return true;                // data is ready
  }else{
    return false;
  }
}


void wuclass_user_setup(wuobject_t *wuobject) {
  user_ttyFd = -1;
  if ( !(user_uart = mraa_uart_init(0)) ){
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): uart failure\n");                
    return;
  }
  const char *devPath = mraa_uart_get_dev_path(user_uart);

  if (!devPath){
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): uart failure\n");                
    return;
  }
  if ( (user_ttyFd = open(devPath, O_RDWR)) == -1){
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): uart failure\n");                
    return;
  }
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): uart ready\n");                
  
}

void wuclass_user_update(wuobject_t *wuobject) {
    char buffer_rev[30]={};
    bool available = wuclass_user_id_dataAvailable(1000);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): available: %d\n", available);                
    if(available){                                       
        int x = read(user_ttyFd, buffer_rev, 30);
        if (x < 0) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): uart error\n");
            return; 
        }
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): size: %d buffer: %s\n",x , buffer_rev); 
        if (x == 30) {
		int i;
		for (i = 0; i < 30; i++) 
        		DEBUG_LOG(DBG_WKPFUPDATE, "%d ", buffer_rev[i]);
        	DEBUG_LOG(DBG_WKPFUPDATE, "\n"); 
	} 
        int16_t output = (int16_t)atoi(buffer_rev);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_USER_USER, output);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): output %d\n", output);
    }
}
#endif