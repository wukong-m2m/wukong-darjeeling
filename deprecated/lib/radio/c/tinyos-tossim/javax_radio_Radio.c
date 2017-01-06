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
#include <string.h>
#include "array.h"
#include "execution.h"
#include "jlib_base.h"
#include "tossim.h"
#include "debug.h"
// void javax.radio.Radio._waitForMessage()
void javax_radio_Radio_void__waitForMessage()
{
//	while (receiveThreadId != -1);
	dj_thread * currentThread = dj_exec_getCurrentThread();

	if (tossim_getNrMessages()==0)
	{
		DEBUG_LOG(DBG_DARJEELING, DARJEELING, "vm[%p]\n", dj_exec_getVM());
		DEBUG_LOG(DBG_DARJEELING, DARJEELING, "waitFor\t\tthread[%p]->status from %d to %d\n", currentThread, currentThread->status, THREADSTATUS_BLOCKED_FOR_IO);
		// block the current thread for IO
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		currentThread->scheduleTime = 0;
		_global_radio_receiveThreadId = currentThread->id;
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
	/*int16_t channel = */dj_exec_stackPopShort();
	// not implemented
}

// void javax.radio.Radio._broadcast(byte[])
void javax_radio_Radio_void__broadcast_byte__()
{
	dj_thread * currentThread = dj_exec_getCurrentThread();

	// get byte array to send
	dj_int_array * arr = REF_TO_VOIDP(dj_exec_stackPopRef());
	if (tossim_send((char*)arr->data.bytes, 0xffff, arr->array.length)==0)
	{
		// block the current thread for IO
		dj_vm* vm = dj_exec_getVM();
		DEBUG_LOG(DBG_DARJEELING, DARJEELING, "broadcast stops\t\tthread[%p]\n", currentThread);
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		_global_radio_sendThreadId = currentThread->id;
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

	if (tossim_send((char*)arr->data.bytes, id, arr->array.length)==0)
	{
		// block the current thread for IO
		DEBUG_LOG(DBG_DARJEELING, DARJEELING, "send stops thread[%p]\n", currentThread);
		currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
		_global_radio_sendThreadId = currentThread->id;
		dj_exec_breakExecution();
	}

	dj_exec_stackPushShort(tossim_wasAcked());

}

// byte[] javax.radio.Radio._readBytes()
void javax_radio_Radio_byte____readBytes()
{
	int length = tossim_peekMessageLength();

	dj_int_array * arr = dj_int_array_create(T_BYTE, length);
	if (arr==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_OutOfMemoryError);
		return;
	}
	void * data = tossim_popMessageBuffer();
	if (data==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
		return;
	}


	memcpy(arr->data.bytes, data, length);

	dj_exec_stackPushRef(VOIDP_TO_REF(arr));
	//avoid deadlocks
}

void javax_radio_Radio_byte__getNumMessages()
{
	dj_exec_stackPushShort(tossim_getNrMessages());
}

void javax_radio_Radio_short_getMaxMessageLength()
{
	dj_exec_stackPushShort(tossim_getMaxPayloadLength());
}

void notify_radio_receive()
{
	// unblock the thread that was waiting for send
	dj_thread * receiveThread = NULL;
	if (_global_radio_receiveThreadId!=-1) receiveThread = dj_vm_getThreadById(dj_exec_getVM(), _global_radio_receiveThreadId);

	if (receiveThread!=NULL)
	{
		if (receiveThread->status==THREADSTATUS_BLOCKED_FOR_IO){
			DEBUG_LOG(DBG_DARJEELING, DARJEELING, "notifyReceive runs \tthread[%p]\n", receiveThread);
			receiveThread->status = THREADSTATUS_RUNNING;
		}

		_global_radio_receiveThreadId = -1;
	}

}

void notify_radio_sendDone()
{
	dj_thread * sendThread = NULL;
	if (_global_radio_sendThreadId!=-1) sendThread = dj_vm_getThreadById(dj_exec_getVM(), _global_radio_sendThreadId);
	// unblock the thread that was waiting for send
	if (sendThread!=NULL)
	{
		if (sendThread->status==THREADSTATUS_BLOCKED_FOR_IO){
			DEBUG_LOG(DBG_DARJEELING, DARJEELING, "sendDone runs \t\tthread[%p]\n", sendThread);
			sendThread->status = THREADSTATUS_RUNNING;
		}
		_global_radio_sendThreadId = -1;
	}
}
