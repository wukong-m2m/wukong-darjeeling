/*
 * CtpPacket.java
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
 
package org.csiro.ctp;

import org.csiro.messaging.Packet;

public class CtpPacket extends Packet {

	protected CtpPacket(short size, byte type, short id) {
		super(size, type, id);
	}

	protected CtpPacket(short size, byte type, short id, short receiverId) {
		super(size, type, id, receiverId);
	}

	protected CtpPacket(byte[] bytes) {
		super(bytes);
	}

	public boolean getRoutingPull() {
		return (bytes[Packet.SIZE] & 1) > 0;
	}

	public boolean getCongested() {
		return (bytes[Packet.SIZE] & 2) > 0;
	}

	public void setRoutingPull(boolean pull) {
		if (pull)
			bytes[Packet.SIZE] &= 1;
		else
			bytes[Packet.SIZE] &= ~1;
	}

	public void setCongested(boolean congested) {
		if (congested)
			bytes[Packet.SIZE] &= 2;
		else
			bytes[Packet.SIZE] &= ~2;
	}

}
