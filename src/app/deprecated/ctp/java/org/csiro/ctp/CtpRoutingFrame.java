/*
 * CtpRoutingFrame.java
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

import javax.darjeeling.Darjeeling;

import org.csiro.messaging.Packet;

public class CtpRoutingFrame extends CtpPacket {

	public final static byte MESSAGETYPE = 64;

	public CtpRoutingFrame(short etx) {
		super((short) (Packet.SIZE + 5), MESSAGETYPE, (short) Darjeeling.getNodeId());
		this.setEtx(etx);
	}

	public CtpRoutingFrame(byte[] bytes) {
		super(bytes);
	}

	public void setParent(short parent) {
		setShort((short) (Packet.SIZE + 1), parent);
	}

	public void setEtx(short etx) {
		setShort((short) (Packet.SIZE + 3), etx);
	}

	public short getParent() {
		return getShort((short) (Packet.SIZE + 1));
	}

	public short getEtx() {
		return getShort((short) (Packet.SIZE + 3));
	}

}
