/*
 * DataEngine.java
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

import org.csiro.debug.Debug;
import org.csiro.messaging.AbstractPacketListener;
import org.csiro.messaging.ArrayQueue;
import org.csiro.messaging.Packet;
import org.csiro.messaging.PacketBufferFullException;

public class DataEngine extends AbstractPacketListener {

	public final static int DATA_SEND_INTERVAL = 500000;
	private final static byte INSTANCEBUFFER_SIZE = 4;

	private CtpDataConsumer consumer;
	private CtpDataProvider provider;

	private static class PacketInstance {
		public short id;
		public byte thl;
		public byte seq;

		public PacketInstance(short id, byte thl, byte seq) {
			this.id = id;
			this.thl = thl;
			this.seq = seq;
		}
	}

	private static class InstanceQueue extends ArrayQueue<PacketInstance> {

		public InstanceQueue() {
			super(INSTANCEBUFFER_SIZE);
		}

		public boolean isDuplicate(short id, short thl, short seq) {
			synchronized (this) {

				// check if this element is already in the queue
				for (short i = 0; i < size; i++) {
					PacketInstance instance = queue[(head + i) % queue.length];
					if (instance.id == id && (byte) instance.thl == thl
							&& (byte) instance.seq == seq) {
						// duplicate found, refresh the item by moving it to the
						// head of the queue
						for (short j = i; j >= 1; j--)
							queue[(head + j) % queue.length] = queue[(head + j - 1)
									% queue.length];

						queue[head] = instance;
						return true;
					}
				}

				// not a duplicate, push a new instance into the queue
				queue[(head + size) % queue.length] = new PacketInstance(id, (byte) thl, (byte) seq);
				if (size < queue.length)
					size++;

				return false;
			}

		}

	}

	private InstanceQueue instanceQueue;
	private RoutingEngine routingEngine;

	public DataEngine(RoutingEngine routingEngine) {
		this.routingEngine = routingEngine;

		instanceQueue = new InstanceQueue();

		// start the data send loop in a new thread
		new Thread() {
			public void run() {
				Debug.print("Data engine thread is : " + Thread.getCurrentThreadId());
				while (true) {
					try {
						sendDataLoop();
					} catch (Throwable t) {
						Debug.print("!!! SendDataLoop died with throwable: " + t + "!!!\nRestarting");
					}
				}
			}
		}.start();

	}

	private void sendDataLoop() {
		Thread.sleep(Darjeeling.random() % DATA_SEND_INTERVAL);
		short seq = 0;
		while (true) {
			Thread.sleep(DATA_SEND_INTERVAL);

			// send data packet if there is a route to the sink, and if the
			// routing engine is not a sink
			if (routingEngine.getParent() != -1 && !routingEngine.isSink() && provider != null) {
				short data[] = provider.getData();

				seq++;

				try {
					CtpDataFrame dataPacket = new CtpDataFrame((short) Darjeeling.getNodeId(), routingEngine.getEtx(), seq, data);

					routingEngine.sendToParent(dataPacket);

				} catch (PacketBufferFullException ex) {
					Debug.print("Data send error, packet buffer full!\n");
				}

			}
		}
	}

	public void setConsumer(CtpDataConsumer consumer) {
		this.consumer = consumer;
	}

	public void setProvider(CtpDataProvider provider) {
		this.provider = provider;
	}

	public CtpDataConsumer getConsumer() {
		return consumer;
	}

	public CtpDataProvider getProvider() {
		return provider;
	}

	public void packetReceived(Packet packet) {
		if (packet instanceof CtpDataFrame) {
			CtpDataFrame dataFrame = (CtpDataFrame) packet;

			// discard duplicates
			if (instanceQueue.isDuplicate(dataFrame.getOrigin(), dataFrame.getTimeHasLived(), dataFrame.getSequenceNumber()))
				return;

			// deliver or forward the message based on whether this node is a
			// sink or not
			if (routingEngine.isSink()) {
				if (consumer != null) {
					short data[] = new short[dataFrame.getDataLength()];
					for (byte i = 0; i < data.length; i++)
						data[i] = dataFrame.getDataItem(i);
					consumer.dataReceived(data, dataFrame.getOrigin(), dataFrame.getTimeHasLived());
				}
			} else {
				Debug.print("(DataEngine -> PacketReceived) Forwarding packet with new ETX " + routingEngine.getEtx() + " : ");
				Debug.print(dataFrame.getBytes());
				// forward
				dataFrame.setTimeHasLived((byte) (dataFrame.getTimeHasLived() + 1));
				dataFrame.setEtx(routingEngine.getEtx());
				routingEngine.sendToParent(dataFrame);
			}

		}
	}

}
