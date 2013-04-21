/*
 * main.c
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <stdlib.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "debug.h"
#include "vm.h"
#include "heap.h"
#include "infusion.h"
#include "types.h"
#include "vmthread.h"
#include "djtimer.h"
#include "execution.h"
#include "hooks.h"

#include "jlib_base.h"
#include "jlib_darjeeling3.h"
#include "jlib_uart.h"
//#include "jlib_wkcomm.h"
#include "jlib_wkpf.h"
#include "jlib_wkreprog.h"

#include "pointerwidth.h"

extern unsigned char di_lib_infusions_archive_data[];
extern unsigned char di_app_infusion_archive_data[];

unsigned char mem[HEAPSIZE];

#include "avr.h"
#include "wifi.h"


WIFI_SETTING wifi_setting = { "home321",       /* SSID */
                        "0123456789" ,        /* WPA/WPA2 passphrase */
                        "192.168.2.5" ,   /* IP address */
                        "255.255.255.0" ,   /* subnet mask */
                        "192.168.2.222"   ,   /* Gateway IP */
                      };

int main()
{
core_init();
uart_init(0, 9600);
uart_write_byte(0, 's');
  SOCKET s1;

  if(!wifi_init(&wifi_setting))
  {uart_write_byte(0, 'i');}

  s1=wifi_client_connect("192.168.2.8", "80");
  if(s1==INVALID_SOCKET)
  {uart_write_byte(0, 'c');}
  
  //wifi_sendData(s1,"GET / HTTP/1.0\r\n\r\n",18);


	return 0;
}
