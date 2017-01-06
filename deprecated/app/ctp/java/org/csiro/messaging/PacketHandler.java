/*
 * PacketHandler.java
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
 
package org.csiro.messaging;

import javax.fleck.Leds;
import javax.radio.Radio;
import javax.util.ArrayBag;

import org.csiro.debug.Debug;

public abstract class PacketHandler {

	private final static byte MAXTRIES = 4;
	private final static byte MAXPENDING = 6;
	private final static int SENDBACKOFFTIME = 100;

	private final ArrayBag<PacketListener> listeners;
	private final ArrayQueue<Packet> pendingPackets;

	private byte tries;

	public PacketHandler() {
		listeners = new ArrayBag<PacketListener>((short) 4);
		pendingPackets = new ArrayQueue<Packet>(MAXPENDING);
		tries = 0;

		// start a new receive thread
		new Thread() {
			public void run() {
				Debug.print("Receive thread is : " + Thread.getCurrentThreadId());
				while (true) {
					try {
						receive();
					} catch (Throwable t) {
						Debug.print("receive: " + t + "\n");
					}
				}
			}
		}.start();

		// start a new send thread
		new Thread() {
			public void run() {
				Debug.print("Send thread is : " + Thread.getCurrentThreadId());
				while (true) {
					// send a packet very SENDBACKOFFTIME milliseconds
					Thread.sleep(SENDBACKOFFTIME);
					try {
						send();
					} catch (Throwable t) {
						Debug.print("throwable during send: " + t + "\n");
					}
				}
			}
		}.start();

	}

	public void addPacketListener(PacketListener listener) {
		listeners.add(listener);
	}

	public void removePacketListener(PacketListener listener) {
		listeners.remove(listener);
	}

	protected abstract Packet wrap(byte data[]);

	public void send(Packet packet) {
		synchronized (pendingPackets) {
			if (!pendingPackets.offer(packet))
				throw new PacketBufferFullException();
		}

	}

	private void firePacketDeliveredEvent(Packet packet, byte tries) {
		for (short i = 0; i < listeners.size(); i++) {
			try {
				listeners.get(i).packetDelivered(packet, tries);
			} catch (Throwable t) {
				Debug.print("Exception during packet delivery! Listener=");
				Debug.print(listeners.get(i).toString());
				Debug.print(" throwable=");
				Debug.print(t.toString());
				Debug.print("\n");
			}
		}
	}

	private void firePacketNotDeliveredEvent(Packet packet, byte tries) {
		for (short i = 0; i < listeners.size(); i++) {
			try {
				listeners.get(i).packetNotDelivered(packet, tries);
			} catch (Throwable t) {
				Debug.print("Exception during packet not delivered notification! Listener=");
				Debug.print(listeners.get(i).toString());
				Debug.print(" throwable=");
				Debug.print(t.toString());
				Debug.print("\n");
			}
		}
	}

	private void send() {
		Packet packet = null;

		// try getting a packet from the queue
		synchronized (pendingPackets) {
			packet = pendingPackets.peek();
		}

		if (packet != null) {
			if (packet.isBroadcast()) {
				Leds.setLed(0, true);
				Radio.broadcast(packet.getBytes());
				Leds.setLed(0, false);

				synchronized (pendingPackets) {
					pendingPackets.remove();
				}
			} else {
				Leds.setLed(0, true);
				boolean ack = Radio.send(packet.getReceiverId(), packet.getBytes());
				Leds.setLed(0, false);
				tries++;

				// if the packet was delivered, or if the packet could not be
				// delivered (too many retries)
				// notify the listeners and remove the packet from the queue
				if (ack || tries > MAXTRIES) {
					synchronized (pendingPackets) {
						pendingPackets.remove();
					}

					if (ack)
						firePacketDeliveredEvent(packet, tries);
					else
						firePacketNotDeliveredEvent(packet, tries);

					tries = 0;
				}
			}

		}
	}

	private void receive() {

		// wait for a packet
		byte data[] = Radio.receive();

		// deserialise into a Message object
		Packet packet = null;
		try {
			packet = wrap(data);
		} catch (Exception ex) {
			Debug.print("The problem is " + ex.toString());
			// error decoding packet
			return;
		}

		if (packet == null) {
			// unknown packet type
			return;
		}

		// deliver the packet
		for (short i = 0; i < listeners.size(); i++)
			listeners.get(i).packetReceived(packet);

	}

}
