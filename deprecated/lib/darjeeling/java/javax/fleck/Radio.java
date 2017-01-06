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
 
package javax.fleck ;

public class Radio
{

	public native static void setChannel(short channel);
	public static native short getMessageLength();
	
	private static native void _sendMessage(short addr, byte type, byte group, byte[] message);
	private static native boolean bufferHasMessages();
	private static native short bufferGetTopAddress();
	private static native byte bufferGetTopGroup();
	private static native byte bufferGetTopType();
	private static native byte[] bufferGetTopBytes();
	private static native void bufferPop();

	public static void sendMessage(short addr, byte type, byte group, byte[] message)
	{
		// TODO check for null, size
		_sendMessage(addr, type, group, message);
	}
	
	public static Message poll()
	{
		Message ret = null;
		if (bufferHasMessages())
		{
            ret = new Message(
					bufferGetTopAddress(),
					bufferGetTopType(),
					bufferGetTopGroup(),
					bufferGetTopBytes()
					);
			bufferPop();
		}
		return ret;
	}

}
