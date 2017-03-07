/*
 * javax_darjeeling_io_Serial.c
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
 
 

#include <stdio.h>

#include "array.h"
#include "execution.h"
#include "nesc.h"
#include "jlib_base.h"
#include "tosconfig.h"

static short uartReceiveThreadId = -1;
static uint8_t buffer[SERIAL_BUFFERSIZE];
static uint16_t bufferStart = 0, bufferSize = 0;

//called from tinyos
void notify_serial_receiveByte(byte)
{
	dj_thread * receiveThread = NULL;

	// Buffer the received byte.
	buffer[(bufferStart + bufferSize) % SERIAL_BUFFERSIZE] = byte;
	if (bufferSize < SERIAL_BUFFERSIZE) bufferSize ++;

	// Unblock the thread that was waiting for send, if any
	if (uartReceiveThreadId!=-1)
	{
		receiveThread = dj_vm_getThreadById(dj_exec_getVM(), uartReceiveThreadId);

		if (receiveThread!=NULL)
		{
			// Unblock the thread
			if (receiveThread->status == THREADSTATUS_BLOCKED_FOR_IO)
				receiveThread->status = THREADSTATUS_RUNNING;

			uartReceiveThreadId = -1;
		}
	}

}

// void javax.darjeeling.io.Serial.setBaudRate(byte, int)
void javax_darjeeling_io_Serial_void_setBaudRate_byte_int()
{
	/*int32_t rate = */dj_exec_stackPopInt();
	/*int16_t nr = */dj_exec_stackPopShort();

	// Do nothing.
}

// byte javax.darjeeling.io.Serial.getNrSerialPorts()
void javax_darjeeling_io_Serial_byte_getNrSerialPorts()
{
	dj_exec_stackPushShort(1);
}

// int javax.darjeeling.io.Serial.getDefaultBaudRate(byte)
void javax_darjeeling_io_Serial_int_getDefaultBaudRate_byte()
{
	/*int16_t nr = */dj_exec_stackPopShort();
	dj_exec_stackPushShort(0);
}

// byte javax.darjeeling.io.SerialInputStream._read(byte)
void javax_darjeeling_io_SerialInputStream_short__read_byte()
{
	// Pop arguments.
	// nr is ignored, because a single UART is assumed.
	/*int16_t nr = */dj_exec_stackPopShort();

	if (bufferSize>0)
	{
		// Dequeue a buffered byte.
		dj_exec_stackPushShort(buffer[bufferStart]);
		bufferSize --;
		bufferStart = (bufferStart+1) % SERIAL_BUFFERSIZE;
	} else
		dj_exec_stackPushShort(-1);

}

// short javax.darjeeling.io.SerialInputStream._waitForByte(byte)
void javax_darjeeling_io_SerialInputStream_void__waitForByte_byte()
{
	// Pop arguments.
	// nr is ignored, because a single UART is assumed.
	/*int16_t nr = */dj_exec_stackPopShort();

	if (bufferSize==0)
	{

		// Suspend the thread.
		if (uartReceiveThreadId==-1)
		{
			dj_thread * currentThread = dj_exec_getCurrentThread();

			// Set the thread status to blocked.
			currentThread->status = THREADSTATUS_BLOCKED_FOR_IO;
			currentThread->scheduleTime = 0;

			// Record which thread we suspended.
			uartReceiveThreadId = currentThread->id;

			// Break the execution of the VM, forcing another thread to be scheduled.
			dj_exec_breakExecution();

		}
	}
}


// void javax.darjeeling.io.SerialOutputStream._write(byte, int)
void javax_darjeeling_io_SerialOutputStream_void__write_byte_int()
{
	// Pop arguments.
	// nr is ignored, because a single UART is assumed.
	int32_t b = dj_exec_stackPopInt();
	/*int16_t nr = */dj_exec_stackPopShort();

	// Blocking write.
	nesc_uartWriteByte((uint8_t)b);
}
