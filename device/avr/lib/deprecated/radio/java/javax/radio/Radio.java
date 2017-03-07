/*
 * Radio.java
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
 
package javax.radio;


public class Radio
{
	
	private static native void _waitForMessage();
	private static native byte[] _readBytes();
	private static native void _init();
	private static native byte _getNumMessages();
	public static native void setChannel(short channel);
	public static native short getMaxMessageLength();

	private static Object sendLock, receiveLock;
	
	private Radio()
	{
		// don't let people instantiate this class
	}
	
	public static void init()
	{
		_init();
		sendLock = new Object();
		receiveLock = new Object();
	}
	
	public static Object getSendLock()
	{
		return sendLock;
	}
	
	public static byte getNumMessages()
	{
		return _getNumMessages();
	}
	
	public static byte[] receive()
	{
		if (receiveLock==null)
			throw new RadioNotInitialisedException();
		
		// allow only one thread to wait for radio messages
		synchronized(receiveLock)
		{
			_waitForMessage();
			return _readBytes();
		}		
	}
	
	private static native void _broadcast(byte[] data);

	/**
	 * The same as broadcase(byte[]) but it gets data as 
	 * a String argument
	 * @param data a String message
	 */
	public static void broadcast(String data)
	{
		broadcast(data.toByteArray());
	}
	/**
	 * Broadcasts {@code data} 
	 * @param data a byte array message
	 */
	public static void broadcast(byte[] data)
	{
		if (sendLock==null)
			throw new RadioNotInitialisedException();
		
		if (data==null)
			throw new NullPointerException();

		// allow only one thread to send at the same time
		synchronized (sendLock)
		{
			// broadcast the message
			_broadcast(data);
		}
	}
		
	private static native boolean _send(short receiverId, byte[] data);
	
	/**
	 * The same as send(short, byte[]) but it gets the data as 
	 * a String argument
	 * @param receiverId 
	 * @param data a String message
	 * @return true if sending is successful
	 */
	public static boolean send(short receiverId, String data){
		return send(receiverId, data.toByteArray());
	}
	
	/**
	 * Sends {@code data} to a receiver with id {@code receiverId}
	 * @param receiverId id of the receiver
	 * @param data a byte array message
	 * @return true if sending is successful
	 */
	public static boolean send(short receiverId, byte[] data)
	{
		if (sendLock==null){
			throw new RadioNotInitialisedException();
		}

		if (data==null){
			throw new NullPointerException();
		}
		
		// allow only one thread to send at the same time
		synchronized (sendLock)
		{
			// send the message
			return _send(receiverId, data);
		}
	}
		
	
}
