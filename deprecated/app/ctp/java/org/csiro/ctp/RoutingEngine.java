/*
 * RoutingEngine.java
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
import org.csiro.messaging.Packet;
import org.csiro.messaging.PacketBufferFullException;
import org.csiro.messaging.PacketListener;

/**
 * 
 * The Routing Engine is responsible for a) finding a parent and b) estimating
 * link quality to neighbors. It does this by periodically broadcasting its ETX
 * value and gathering ETX values from neighbors. Neighbors are also pinged
 * periodically to sample the link quality to other nodes. Link quality is
 * measured as the average of the number of retransmissions needed for the last
 * n pings.
 * 
 * Only the best n neighbors are kept track of. In this implementation 'best'
 * means 'lowest etx'. Neighbors have failed to respond to ping messages for
 * some maximum number of times are automatically removed from the neighbor
 * list.
 * 
 * @author Niels Brouwers
 * 
 */
public class RoutingEngine implements PacketListener {

	// Broadcasting and pinging starts at MIN_INTERVAl and increases
	// expontentially to MAX_INTERVAL
	// This process is reset in case of a routing pull which is in turn
	// triggered by incoming packets
	// with the routing pull bit set to one, or a lost parent.
	public final static int MIN_INTERVAL = 5000;
	public final static int MAX_INTERVAL = 1200000;

	// The packet handler allows us to send/broadcast packets and deals with
	// retransmissions
	private final CtpPacketHandler handler;

	// List of known neighbors
	private final NodeList nodeList;

	// True if the RoutingEngine should act as a sink, which means it should
	// broadcast ETX=0
	private final boolean isSink;

	// Expected transmissions to parent
	private short etx;
	private short parent;

	// intervals for beaconing and pinging
	private int beaconInterval;
	private int pingInterval;

	/**
	 * Constructs a new RoutingEngine
	 * 
	 * @param isSink
	 *            indicates whether the routing engine is a sink and should
	 *            broadcast ETX=0
	 * @param handler
	 *            the packet handler for sending/receiving packets
	 */
	public RoutingEngine(boolean isSink, CtpPacketHandler handler) {
		this.isSink = isSink;
		this.handler = handler;
		nodeList = new NodeList((short) 4);
		routingPull();
		calculateEtx();
	}

	/**
	 * Returns the current expected transmissions to sink (ETX)
	 * 
	 * @return
	 */
	public short getEtx() {
		return etx;
	}

	/**
	 * @return the current parent node ID
	 */
	public short getParent() {
		return parent;
	}

	/**
	 * @return true if the routing engine is acting as a sink
	 */
	public boolean isSink() {
		return isSink;
	}

	/**
	 * @return a NodeList object containing information about the node neighbors
	 */
	public NodeList getNodeList() {
		return nodeList;
	}

	/**
	 * Sends a CtpPacket to the parent. If there is no parent, the packet will
	 * be dropped and a routing pull will be forced.
	 * 
	 * @param packet
	 *            packet to send to parent
	 */
	public void sendToParent(CtpPacket packet) {
		if (parent == -1) {
			// no route, force routing pull
			routingPull();

			Debug.print("No route to parent, data frame dropped\n");
		} else {
			// send the frame
			packet.setReceiverId(parent);
			packet.setSenderId((short) Darjeeling.getNodeId());
			try {
				handler.send(packet);
			} catch (PacketBufferFullException ex) {
				Debug.print("Data dropped: buffer full");
			}
		}

	}

	/**
	 * Called by the CtpPacketHandler when a packet has arrived at the node
	 */
	public void packetReceived(Packet packet) {

		// if the packet is a CtpPacket with routing pull set to true, reset the
		// intervals
		if (packet instanceof CtpPacket && (((CtpPacket) packet).getRoutingPull()))
			resetInterval();

		// process routing frame
		if (packet instanceof CtpRoutingFrame) {
			/*
			 * String neighbors =
			 * "(RoutingEngine -> PacketReceived) Neighbors are : ["; for (short
			 * i = 0 ; i < nodeList.size(); i ++){ neighbors =
			 * String.concat(neighbors, Integer.toString(nodeList.get(i).id),
			 * ", "); } neighbors = String.concat(neighbors, "]");
			 * Debug.print(neighbors);
			 */
			Node node = nodeList.getNodeById(packet.getSenderId());

			CtpRoutingFrame routingPacket = (CtpRoutingFrame) packet;
			short nodeEtx = routingPacket.getEtx();

			if (node != null)
				node.etx = nodeEtx;
			else
				nodeList.insert(routingPacket.getSenderId(), nodeEtx);
		}

		// dataframes with lower ETX values than ours should also trigger a
		// routing pull
		if (packet instanceof CtpDataFrame) {
			CtpDataFrame dataFrame = (CtpDataFrame) packet;
			if (dataFrame.getEtx() < etx)
				routingPull();
		}

	}

	/**
	 * Called by the CtpPacketHandler when a unicast packet has been delivered
	 * successfully
	 * 
	 * @param packet
	 *            the packet that was delivered
	 * @param tries
	 *            the number of tries
	 */
	public void packetDelivered(Packet packet, byte tries) {
		nodeList.sendOk(packet.getReceiverId(), tries);
		calculateEtx();
	}

	/**
	 * Called by the CtpPacketHandler when a unicast packet could not be
	 * delivered
	 * 
	 * @param packet
	 *            the packet that was dropped
	 * @param tries
	 *            the number of tries
	 */
	public void packetNotDelivered(Packet packet, byte tries) {
		nodeList.sendError(packet.getReceiverId(), tries);
		calculateEtx();

		// route is lost, perform routing pull
		if (parent == -1)
			routingPull();
	}

	// calculates the ETX for this node
	private void calculateEtx() {
		if (isSink) {
			etx = 0;
			parent = -1;
		} else {
			Node parentNode = nodeList.getParent();
			if (parentNode == null) {
				etx = -1;
				parent = -1;
			} else {
				etx = parentNode.getCost();
				parent = parentNode.id;
			}
		}

	}

	/**
	 * Starts the routing engine.
	 */
	public void start() {
		// start the ping thread
		// if (!isSink)
		new Thread() {
			public void run() {
				Debug.print("PingLoop thread is : " + Thread.getCurrentThreadId());
				try {
					pingLoop();
				} catch (Throwable t) {
					Debug.print("Ping thread terminated unexpectedly: " + t + "\n");
				}
			}
		}.start();

		// start the beacon thread
		new Thread() {
			public void run() {
				Debug.print("BeaconLoop thread is : " + Thread.getCurrentThreadId());
				try {
					beaconLoop();
				} catch (Throwable t) {
					Debug.print("Beacon thread terminated unexpectedly: " + t + "\n");
				}
			}
		}.start();

	}

	// resets the beacon and ping intervals
	private void resetInterval() {
		beaconInterval = MIN_INTERVAL * 1024;
		pingInterval = MIN_INTERVAL * 1024;
	}

	/**
	 * Performs a routing pull by resetting the broadcast/ping intervals and
	 * causes the next outgoing packet to have its routing pull bit set.
	 */
	public void routingPull() {
		beaconInterval = MIN_INTERVAL * 1024;
		pingInterval = MIN_INTERVAL * 1024;
		handler.setRoutingPull(true);
	}

	// periodically sends a 'ping' message to one of the neighbors
	private void pingLoop() {
		short nr = 0;
		while (true) {
			int sleep = Darjeeling.random() % pingInterval;
			Thread.sleep(sleep);

			Node node = null;

			// round-robin elect a node from the neighbor list
			node = nodeList.elect(nr++);

			if (node != null) {

				try {
					CtpPingFrame ping = new CtpPingFrame((short) Darjeeling.getNodeId(), node.id);
					handler.send(ping);
				} catch (PacketBufferFullException ex) {
					Debug.print("PingLoop : ERROR Packet buffer full!\n");
				}

				// decay interval
				if (parent != -1) {
					pingInterval *= 2;
					if (pingInterval > 1024 * MAX_INTERVAL)
						pingInterval = 1024 * MAX_INTERVAL;
				} else
					routingPull();
			}

			Thread.sleep(pingInterval - sleep);

		}
	}

	// periodically broadcasts a routing message
	private void beaconLoop() {
		while (true) {
			int sleep = Darjeeling.random() % beaconInterval;
			Thread.sleep(sleep);

			try {
				CtpRoutingFrame routingPacket = new CtpRoutingFrame(etx);
				handler.send(routingPacket);
			} catch (PacketBufferFullException ex) {
				Debug.print("BeaconLoop : ERROR Packet buffer full!\n");
			}

			// decay interval
			Thread.sleep(beaconInterval - sleep);

			if (isSink || (!isSink && parent != -1)) {
				beaconInterval *= 2;
				if (beaconInterval > 1024 * MAX_INTERVAL)
					beaconInterval = 1024 * MAX_INTERVAL;
			}

			if (!isSink && parent == -1)
				routingPull();

		}
	}

}
