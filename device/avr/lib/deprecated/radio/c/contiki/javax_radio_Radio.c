/*
 * javax_radio_Radio.c
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
 
//thread status
#include "execution.h"
//arrays
#include "array.h"
//communication using rime stack
#include "net/rime.h"
//exceptions
#include "jlib_base.h"
//memory allocation
#include "types.h"
#include "heap.h"
//for memcpy
#include "string.h"
//buffer filling
#include "main.h"
//like many contiki examples we consider communication channel to be 128
#define DEFAULT_BROADCAST_COMMUNICATION_CHANNEL 128

//if net simulator from contiki is used we set id postfix to 1
#ifdef USE_NETSIM
#define MOTE_ID_POSTFIX 1
#endif

//if cooja simulator from contiki is used we include node-ids and we set id postfix to 0
#ifdef IS_COOJA
#include "node-id.h"
#define MOTE_ID_POSTFIX 0
#endif

extern char * incoming_buffer;
extern int incoming_buffer_length;

static short sendThreadId = -1, receiveThreadId = -1;
static struct broadcast_conn broadcast_connection;

#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
extern list_t neighbor_table;
static struct announcement example_announcement;
static struct rmh_conn unicast_connection;
#else
static struct unicast_conn unicast_connection;
#endif

//if multihop reliable unicast is used
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
/**
 * multi-hop reliable unicast callback function
 */
static void read_if_not_busy() {
	if (incoming_buffer == NULL) {
		incoming_buffer_length = packetbuf_datalen();
		incoming_buffer = dj_mem_alloc(incoming_buffer_length, CHUNKID_REFARRAY);
		memcpy(incoming_buffer, (char*)packetbuf_dataptr(), incoming_buffer_length);
		dj_notifyRadioReceive();
	}
}
static void
recv(struct rmh_conn *c, rimeaddr_t *sender,
		uint8_t hops)
{
	rimeaddr_copy(sender, packetbuf_addr(PACKETBUF_ADDR_SENDER));
	read_if_not_busy();
}
/*
 * This function is called to forward a packet. The function picks a
 * random neighbor from the neighbor list and returns its address. The
 * rmh layer sends the packet to this address. If no neighbor is
 * found, the function returns NULL to signal to the rmh layer
 * that the packet should be dropped.
 */
//---------------------------------------------------------------------------*/
static rimeaddr_t *
forward(struct rmh_conn *c,
		rimeaddr_t *originator, rimeaddr_t *dest,
		rimeaddr_t *prevhop, uint8_t hops)
{
	/* Find a random neighbor to send to. */
	int num, i;
	struct example_neighbor *n;

	if(list_length(neighbor_table) > 0) {
		num = random_rand() % list_length(neighbor_table);
		i = 0;
		for(n = list_head(neighbor_table); n != NULL && i != num; n = n->next) {
			++i;
		}
		if(n != NULL) {
			return &n->addr;
		}
	}

	return NULL;
}
#endif
/**
 * broadcast callbacbk function
 */
static void broadcast_recv(struct broadcast_conn *c, rimeaddr_t *sender) {
	rimeaddr_copy(sender, packetbuf_addr(PACKETBUF_ADDR_SENDER));
	if (incoming_buffer == NULL) {
		incoming_buffer_length = packetbuf_datalen();
		incoming_buffer = dj_mem_alloc(incoming_buffer_length, CHUNKID_REFARRAY);
		memcpy(incoming_buffer, (char*) packetbuf_dataptr(),
				incoming_buffer_length);
		dj_notifyRadioReceive();
	}
	//otherwise, if still one message is being processed discard the arrived message
}

/**
 * Single-hop unicast callback function
 */
#ifndef WITH_MULTIHOP_RELIABLE_UNICAST
static void recv_uc(struct unicast_conn *c, rimeaddr_t *from) {
	if (incoming_buffer == NULL) {
		incoming_buffer_length = packetbuf_datalen();
		incoming_buffer = dj_mem_alloc(incoming_buffer_length, CHUNKID_REFARRAY);
		memcpy(incoming_buffer, (char*) packetbuf_dataptr(),
				incoming_buffer_length);
		dj_notifyRadioReceive();
	}
	//otherwise, if still one message is being processed discard the arrived message
}
#endif //if single-hop is used


static const struct broadcast_callbacks	broadcast_callbacks = { broadcast_recv };
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
static const struct rmh_callbacks unicast_callbacks = {recv, forward};
#else
static const struct unicast_callbacks unicast_callbacks = { recv_uc };
#endif

/*
 * Initialize radio communications (both uni- and broad-cast)
 */
// void javax.radio.Radio._init()
void javax_radio_Radio_void__init() {
	//initialize broadcast connction
	broadcast_open(&broadcast_connection,
			DEFAULT_BROADCAST_COMMUNICATION_CHANNEL, &broadcast_callbacks);

	//initialize unicast connection

	/* Open a rmh connection on Rime channel CHANNEL. */
	//for now consider the channel used for unicast communication to be
	//1 more than broadcast communication channel

	//if multihop reliable unicast is going to be used
#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
	rmh_open(&unicast_connection, DEFAULT_BROADCAST_COMMUNICATION_CHANNEL + 1, &unicast_callbacks);

	Register an announcement with the same announcement ID as the
	Rime channel we use to open the rmh connection above.
	announcement_register(&example_announcement,
			DEFAULT_BROADCAST_COMMUNICATION_CHANNEL + 1,
			0,
			received_announcement);
	//otherwise use a simple unicast connection
#else
	unicast_open(&unicast_connection, DEFAULT_BROADCAST_COMMUNICATION_CHANNEL
			+ 1, &unicast_callbacks);
#endif
	init_connections(&broadcast_connection, &unicast_connection);
}

/*
 * Reading a message from radio should be blocking. Therefore,
 * wait until a message is arrived. Until that time, do not run
 * the next instructions
 */
void javax_radio_Radio_void__waitForMessage() {
	// wait for radio
	dj_thread * currentThread = dj_exec_getCurrentThread();
	receiveThreadId = currentThread->id;
	if (incoming_buffer == NULL) {
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		dj_exec_breakExecution();
	}
}

// byte[] javax.radio.Radio._readBytes()
void javax_radio_Radio_byte____readBytes() {
	//wait until a real message is received in the callback function

	dj_int_array * arr = dj_int_array_create(T_BYTE, incoming_buffer_length);

	if (arr == NULL) {
		dj_exec_createAndThrow(BASE_CDEF_java_lang_OutOfMemoryError);
		return;
	}

	memcpy(arr->data.bytes, incoming_buffer, incoming_buffer_length);
	if (arr->data.bytes == NULL) {
		dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
		return;
	}
	dj_exec_stackPushRef(VOIDP_TO_REF(arr));
	dj_mem_free(incoming_buffer);
	incoming_buffer = NULL;
	incoming_buffer_length = 0;
}

// byte javax.radio.Radio._getNumMessages()
void javax_radio_Radio_byte__getNumMessages() {
	if (incoming_buffer != NULL)
		//TODO: if we increase input_buffer size, this number
		//should change accordingly
		dj_exec_stackPushShort(1);
	else
		dj_exec_stackPushShort(0);

}

// void javax.radio.Radio.setChannel(short)
void javax_radio_Radio_void_setChannel_short() {
	/*int16_t channel = */dj_exec_stackPopShort();
	// not implemented
}

// short javax.radio.Radio.getMaxMessageLength()
void javax_radio_Radio_short_getMaxMessageLength() {
	dj_exec_stackPushShort(strlen(incoming_buffer));
}

// void javax.radio.Radio._broadcast(byte[])
void javax_radio_Radio_void__broadcast_byte__() {
	//	rimeaddr_t addr;
	dj_thread * currentThread = dj_exec_getCurrentThread();
	dj_int_array * byteArray = REF_TO_VOIDP(dj_exec_stackPopRef());

	// check null
	if (byteArray == nullref)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);

	// copy bytes to the rime buffer

	fill_broadcast_buffer((char *) byteArray->data.bytes,
			byteArray->array.length);
	// block the current thread for IO
	//and wait until main sends the buffer
	currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
	sendThreadId = currentThread->id;
	dj_exec_breakExecution();
}

// boolean javax.radio.Radio._send(short, byte[])
void javax_radio_Radio_boolean__send_short_byte__() {
	rimeaddr_t to;
	dj_thread * currentThread = dj_exec_getCurrentThread();
	dj_int_array * byteArray = REF_TO_VOIDP(dj_exec_stackPopRef());

	int16_t id = dj_exec_stackPopShort();
	// check null
	if (byteArray == nullref)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);

	to.u8[0] = id;
	//keep the second part 0 for now, it might change,
	to.u8[1] = MOTE_ID_POSTFIX;
	fill_unicast_buffer((char *) byteArray->data.bytes,
			byteArray->array.length, to);

	// block the current thread for IO
	//and wait until main sends the buffer
	currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
	sendThreadId = currentThread->id;
	dj_exec_breakExecution();
	//when it is finished, return true
	dj_exec_stackPushShort(1);

}
/**
 * Notifies the broadcast message receiver to start reading the message buffer
 */
void notify_radio_receive() {
	// unblock the thread that was waiting for send
	dj_thread * receiveThread = NULL;
	if (receiveThreadId != -1)
		receiveThread = dj_vm_getThreadById(dj_exec_getVM(), receiveThreadId);

	if (receiveThread != NULL) {
		if (receiveThread->status == THREADSTATUS_BLOCKED_FOR_IO)
			receiveThread->status = THREADSTATUS_RUNNING;
		receiveThreadId = -1;
	}
}

/**
 * Notifies the broadcast message sender to continue its operation after send instruction
 */
void notify_radio_sendDone() {
	dj_thread * sendThread = NULL;
	if (sendThreadId != -1)
		sendThread = dj_vm_getThreadById(dj_exec_getVM(), sendThreadId);

	// unblock the thread that was waiting for send
	if (sendThread != NULL) {
		if (sendThread->status == THREADSTATUS_BLOCKED_FOR_IO)
			sendThread->status = THREADSTATUS_RUNNING;
		sendThreadId = -1;
	}
}
