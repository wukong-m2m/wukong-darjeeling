/*
 * javax_darjeeling_radio_Radio.c
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
 
 
#include <stdint.h>
#include <string.h>

#include "array.h"
#include "execution.h"
#include "jlib_base.h"


//declarations of nesc functions
void nesc_setLed(int nr, int on);
int32_t nesc_getTime();
int32_t nesc_getNodeId();
void nesc_setChannel(uint8_t channel);
uint16_t nesc_getMaxPayloadLength();
int nesc_send(const char * message, int16_t receiverId, uint16_t length);
uint16_t nesc_peekMessageLength();
void * nesc_popMessageBuffer();
int nesc_getNrMessages();
int nesc_wasAcked();


static short sendThreadId = -1, receiveThreadId = -1;

// void javax.darjeeling.radio.Radio._waitForMessage()
void javax_darjeeling_radio_Radio_void__waitForMessage()
{

	dj_thread * currentThread = dj_exec_getCurrentThread();

	if (nesc_getNrMessages()==0)
	{

		// block the current thread for IO
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		currentThread->scheduleTime = 0;
		receiveThreadId = currentThread->id;
		dj_exec_breakExecution();
	}
}

//called from tinyos
void notify_radio_receive()
{
	// unblock the thread that was waiting for send
	dj_thread * receiveThread = NULL;
	if (receiveThreadId!=-1) receiveThread = dj_vm_getThreadById(dj_exec_getVM(), receiveThreadId);

	if (receiveThread!=NULL)
	{
		if (receiveThread->status==THREADSTATUS_BLOCKED_FOR_IO)
			receiveThread->status = THREADSTATUS_RUNNING;
		receiveThreadId = -1;
	}

}

//called from tinyos
void notify_radio_sendDone()
{
	dj_thread * sendThread = NULL;
	if (sendThreadId!=-1) sendThread = dj_vm_getThreadById(dj_exec_getVM(), sendThreadId);

	// unblock the thread that was waiting for send
	if (sendThread!=NULL)
	{
		if (sendThread->status==THREADSTATUS_BLOCKED_FOR_IO)
			sendThread->status = THREADSTATUS_RUNNING;
		sendThreadId = -1;
	}
}

// byte[] javax.darjeeling.radio.Radio._readBytes()
void javax_darjeeling_radio_Radio_byte____readBytes()
{
	int length = nesc_peekMessageLength();

	dj_int_array * arr = dj_int_array_create(T_BYTE, length);

	if (arr==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_OutOfMemoryError);
		return;
	}

	void * data = nesc_popMessageBuffer();

	if (data==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
		return;
	}

	memcpy(arr->data.bytes, data, length);

	dj_exec_stackPushRef(VOIDP_TO_REF(arr));
}

// void javax.darjeeling.radio.Radio._init(int)
void javax_darjeeling_radio_Radio_void__init_int()
{
	/*int flags = */dj_exec_stackPopInt();

	//TODO: use flags to initialise the radio
}

// byte javax.darjeeling.radio.Radio._getNumMessages()
void javax_darjeeling_radio_Radio_byte__getNumMessages()
{
	dj_exec_stackPushShort(nesc_getNrMessages());
}

// boolean javax.darjeeling.radio.Radio.hasRadio()
void javax_darjeeling_radio_Radio_boolean_hasRadio()
{
	dj_exec_stackPushShort(1);
}

// void javax.darjeeling.radio.Radio.setChannel(short)
void javax_darjeeling_radio_Radio_void_setChannel_short()
{
	int16_t channel = dj_exec_stackPopShort();

	nesc_setChannel(channel);
}

// short javax.darjeeling.radio.Radio.getFirstChannel()
void javax_darjeeling_radio_Radio_short_getFirstChannel()
{
	//TODO: implement
	dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
}

// short javax.darjeeling.radio.Radio.getLastChannel()
void javax_darjeeling_radio_Radio_short_getLastChannel()
{
	//TODO: implement
	dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
}

// void javax.darjeeling.radio.Radio.setOutputPower(short)
void javax_darjeeling_radio_Radio_void_setOutputPower_short()
{
	/*int power = */dj_exec_stackPopShort();
	//TODO: implement
	dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
}

// short javax.darjeeling.radio.Radio.getMaxMessageLength()
void javax_darjeeling_radio_Radio_short_getMaxMessageLength()
{
	dj_exec_stackPushShort(nesc_getMaxPayloadLength());
}

// void javax.darjeeling.radio.Radio._broadcast(byte[])
void javax_darjeeling_radio_Radio_void__broadcast_byte__()
{
	dj_thread * currentThread = dj_exec_getCurrentThread();

	// get byte array to send
	dj_int_array * arr = REF_TO_VOIDP(dj_exec_stackPopRef());

	if (nesc_send((char*)arr->data.bytes, 0xffff, arr->array.length)==0)
	{
		// block the current thread for IO
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		sendThreadId = currentThread->id;
		dj_exec_breakExecution();
	}

}

// boolean javax.darjeeling.radio.Radio._send(short, byte[])
void javax_darjeeling_radio_Radio_boolean__send_short_byte__()
{
	dj_thread * currentThread = dj_exec_getCurrentThread();

	// get byte array to send
	dj_int_array * arr = REF_TO_VOIDP(dj_exec_stackPopRef());
	int16_t id = dj_exec_stackPopShort();

	if (nesc_send((char*)arr->data.bytes, id, arr->array.length)==0)
	{
		// block the current thread for IO
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		sendThreadId = currentThread->id;
		dj_exec_breakExecution();
	}

	dj_exec_stackPushShort(nesc_wasAcked());
}
