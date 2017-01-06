/*
 * BroadcastMessage.java
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

/**
 * A message that is broadcasted to all nodes. The destination address defaults 0xFFFF
 * 
 * @author Niels Brouwers
 *
 */
public class BroadcastMessage extends Message
{

	/**
	 * Constructs a new Broadcast message with no payload data.
	 * @param type message type
	 * @param group message group
	 */
	public BroadcastMessage(byte type, byte group)
	{
		super((short)0xffff, type, group);
	}
	
	/**
	 * Constructs a new Broadcast message with no payload data.
	 * @param type message type
	 * @param group message group
	 * @param data the payload data
	 */
	public BroadcastMessage(byte type, byte group, byte[] data)
	{
		super((short)0xffff, type, group, data);
	}
	
}
