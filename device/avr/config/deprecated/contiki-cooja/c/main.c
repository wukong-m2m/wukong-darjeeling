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
 
#include "main.h"
#include "darjeeling.h"

static struct broadcast_conn *my_broadcast_connection;
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
static struct rmh_conn *my_unicast_connection;
#else
static struct unicast_conn *my_unicast_connection;
#endif

static char *broadcast_buffer_data;
static char *unicast_buffer_data;
int broadcast_buffer_length = 0;
int unicast_buffer_length = 0;

static rimeaddr_t unicast_to;


char *incoming_buffer = NULL;
int incoming_buffer_length =0;

/**
 * reads initialization info from radio init (javax_radio_Radio.c)
 */
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
void init_connections(struct broadcast_conn * broadcast_connection, struct rmh_conn *unicast_connection){
#else
void init_connections(struct broadcast_conn * broadcast_connection, struct unicast_conn *unicast_connection){
#endif
	my_broadcast_connection = broadcast_connection;
	my_unicast_connection = unicast_connection;
}
/**
 * fills the broadcast radio buffer with the data it receives, then at a timer interval
 * radio_sender_process uses this thread to send data to its neighbors
 */
void fill_broadcast_buffer(char* data, int length){
	broadcast_buffer_data = data;
	broadcast_buffer_length = length;
}
/**
 * fills the unicast radio buffer with the data it receives, then at a timer interval
 * radio_sender_process uses this thread to send data to its neighbors
 */
void fill_unicast_buffer(char* data, int length, rimeaddr_t to){
	unicast_buffer_data = data;
	unicast_buffer_length = length;
	unicast_to = to;
}
/*---------------------------------------------------------------------------*/
//two prothothreads, one for sending messages periodically, and the other
//for our main application
PROCESS(radio_sender_process, "Radio Sender Process");
PROCESS(main_process, "Main Contiki/Darjeeling Process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&main_process, &radio_sender_process);
/*---------------------------------------------------------------------------*/

void close_connections(){
	broadcast_close(my_broadcast_connection);
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
	rmh_close(my_unicast_connection);
#else
	unicast_close(my_unicast_connection);
#endif
}
/**
 * This process checks periodically if there is something to be sent to the neighbors
 * if so it uses abc ( Anonymous best-effort local area Broad Cast) to send the data,
 */
PROCESS_THREAD(radio_sender_process, ev, data){
 	static struct etimer et;
	PROCESS_BEGIN();
	PROCESS_EXITHANDLER(close_connections();)

#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
	/* Initialize the memory for the neighbor table entries. */
	memb_init(&neighbor_mem);

	/* Initialize the list used for the neighbor table. */
	list_init(neighbor_table);
#endif
	while (1){

	    etimer_set(&et, CLOCK_SECOND * 2);
	//	PROCESS_YIELD_UNTIL(etimer_expired(&et));
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	    if (broadcast_buffer_data != NULL){
	    	packetbuf_copyfrom(broadcast_buffer_data,broadcast_buffer_length);
	    	//	packetbuf_copyfrom("Hello", 6);
	    	broadcast_send(my_broadcast_connection);
	    	dj_notifyRadioSendDone();
	    	broadcast_buffer_data = NULL;
	    }

	    if (unicast_buffer_data != NULL){
	    	//send reliably through multi-hop routing
	    	//in Contiki 2.3 MAX_HOPS is simple ignored,
	    	packetbuf_copyfrom(unicast_buffer_data, unicast_buffer_length);
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
	        rmh_send(my_unicast_connection, &unicast_to, NUM_OF_MAX_TRIES, MAX_HOPS);
#else
	        unicast_send(my_unicast_connection, &unicast_to/*, NUM_OF_MAX_TRIES, MAX_HOPS*/);
#endif
	    	dj_notifyRadioSendDone();
	    	unicast_buffer_data = NULL;
	    }


	}
	PROCESS_END();
}
/**
 * Main process runs darjeeling on top of Contiki
 */
PROCESS_THREAD(main_process, ev, data)
{
	PROCESS_BEGIN();
	static struct etimer timer;
	PROCESS_EXITHANDLER()
	dj_init();

	while (1)
	{
		dj_run();

		// we set the timer from here every time
		etimer_set(&timer, CLOCK_CONF_SECOND / 4);

		// and wait until the vent we receive is the one we're waiting for
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
	}
	PROCESS_END();
}
