/*
 * Thread.java
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
 
package java.lang;

/**
 * 
 * A thread is a thread of execution in a program. The Java Virtual Machine
 * allows an application to have multiple threads of execution running
 * concurrently.
 * 
 * 
 * 
 * @author Niels Brouwers
 * 
 */
public class Thread implements Runnable
{

	// thread ID is a link to an internal thread
	private short id;

	// creates a new java thread and returns the thread id
	private static native short _create();

	// start the thread, calls the 'run' method on the runnable object
	private native void _start(short id);

	// gets the status of a thread
	private native short _getStatus(short id);

	// sets the runnable object on a thread
	private native void _setRunnable(short id, Runnable r);

	// returns the id of the currently executing thread
	private static native short _getCurrentThreadId();

	// creates a new java.lang.Thread object with a given ID
	private Thread(short id)
	{
		this.id = id;
	}

	/**
	 * Constructs a new Thread object.
	 */
	public Thread()
	{
		this.id = _create();
		_setRunnable(id, this);
	}

	/**
	 * Constructs a new Thread object.
	 * 
	 * @param runnable the Runnable object whose run() function will be executing
	 */
	public Thread(Runnable runnable)
	{
		// cannot instantiate with null
		if (runnable == null)
			throw new IllegalArgumentException();

		this.id = _create();
		_setRunnable(id, runnable);
	}

	public void start()
	{
		// check thread status
		short status = _getStatus(this.id);

		// state 0 == THREADSTATE_CREATED
		if (status != 0)
			throw new IllegalThreadStateException();

		// start
		_start(this.id);
	}

	public void run()
	{
	}

	public static native void sleep(long time);

	public static native int activeCount();

	public static Thread currentThread()
	{
		return new Thread(_getCurrentThreadId());
	}

	public static int getCurrentThreadId()
	{
		return _getCurrentThreadId();
	}
	
	public boolean isAlive()
	{
		// check thread status
		short status = _getStatus(this.id);

		// TODO look up the definition of 'alive'
		return (status >= 0);
	}

	public static native void yield();

}
