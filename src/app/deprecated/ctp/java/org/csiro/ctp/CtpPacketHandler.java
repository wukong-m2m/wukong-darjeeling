/*
 * CtpPacketHandler.java
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
import org.csiro.messaging.PacketHandler;

public class CtpPacketHandler extends PacketHandler {

	private boolean isCongested;
	private boolean routingPull;

	public Packet wrap(byte data[]) {
		switch (data[0]) {
		case CtpDataFrame.MESSAGETYPE:
			return new CtpDataFrame(data);
		case CtpRoutingFrame.MESSAGETYPE:
			return new CtpRoutingFrame(data);
		case CtpPingFrame.MESSAGETYPE:
			return new CtpPingFrame(data);
		default:
			return null;
		}
	}

	public void setCongested(boolean isCongested) {
		this.isCongested = isCongested;
	}

	public void setRoutingPull(boolean routingPull) {
		this.routingPull = routingPull;
	}

	public boolean isCongested() {
		return isCongested;
	}

	public boolean isRoutingPull() {
		return routingPull;
	}

	public void send(CtpPacket packet) {
		packet.setCongested(isCongested);

		packet.setRoutingPull(routingPull);

		isCongested = false;

		routingPull = false;
		super.send(packet);
	}

}
