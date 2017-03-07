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
 
#include <stdint.h>

#include "array.h"
#include "execution.h"
#include "jlib_base.h"

static short sendThreadId = -1, receiveThreadId = -1;

// void javax.radio.Radio._waitForMessage()
void javax_radio_Radio_void__waitForMessage()
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

// void javax.radio.Radio._init()
void javax_radio_Radio_void__init()
{
}

// void javax.radio.Radio.setChannel(short)
void javax_radio_Radio_void_setChannel_short()
{
	int16_t channel = dj_exec_stackPopShort();
	// not implemented
}

// void javax.radio.Radio._broadcast(byte[])
void javax_radio_Radio_void__broadcast_byte__()
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

// boolean javax.radio.Radio._send(short, byte[])
void javax_radio_Radio_boolean__send_short_byte__()
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

// byte[] javax.radio.Radio._readBytes()
void javax_radio_Radio_byte____readBytes()
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

void javax_radio_Radio_byte__getNumMessages()
{
	dj_exec_stackPushShort(nesc_getNrMessages());
}

void javax_radio_Radio_short_getMaxMessageLength()
{
	dj_exec_stackPushShort(nesc_getMaxPayloadLength());
}

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

void notify_radio_sendDone()
{
	dj_thread * sendThread;
	if (sendThreadId!=-1) sendThread = dj_vm_getThreadById(dj_exec_getVM(), sendThreadId);

	// unblock the thread that was waiting for send
	if (sendThread!=NULL)
	{
		if (sendThread->status==THREADSTATUS_BLOCKED_FOR_IO)
			sendThread->status = THREADSTATUS_RUNNING;
		sendThreadId = -1;
	}
}
