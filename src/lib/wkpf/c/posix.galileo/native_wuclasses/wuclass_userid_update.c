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

mraa_uart_context uart;

void wuclass_userid_setup(wuobject_t *wuobject) {
    uart = mraa_uart_init(0);
    if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return EXIT_FAILURE;
    }

}

void wuclass_userid_update(wuobject_t *wuobject) {
    char buffer_rev[30]={};
    int available = mraa_uart_data_available(uart, 1000);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): available: %d\n", available);                
    if(available){                                       
        int x = mraa_uart_read(uart, buffer_rev, 30);      
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): buffer: %s\n", buffer_rev); 
        int16_t output = (int16_t)atoi(buffer_rev);
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_USERID_USERID, output);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(userID): output %d\n", output);
    }
}
