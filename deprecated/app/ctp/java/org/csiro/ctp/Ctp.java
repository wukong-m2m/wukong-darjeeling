/*
 * Ctp.java
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

/**
 * Collection Tree Protocol (CTP)
 * 
 * A simplified version of the CTP implementation found in TinyOS.
 * 
 * @author Niels Brouwers
 * 
 */
public class Ctp {

	public final static int DEBUG_INTERVAL = 60000;

	// packet handler that can handle CTP packets
	private CtpPacketHandler handler;

	// routing engine responsible for finding a route to the sink and link
	// quality estimation
	private RoutingEngine routingEngine;

	// data engine is repsonsible for generating and forwarding data packets to
	// sink
	private DataEngine dataEngine;

	private Ctp(boolean isSink) {
		handler = new CtpPacketHandler();

		routingEngine = new RoutingEngine(isSink, handler);
		handler.addPacketListener(routingEngine);

		dataEngine = new DataEngine(routingEngine);
		handler.addPacketListener(dataEngine);
	}

	/**
	 * Constructs a new CTP routing object that acts as a sink.
	 * 
	 * @param consumer
	 *            a CtpDataConsumer object that handles incoming data
	 */
	public Ctp(CtpDataConsumer consumer) {
		this(true);
		dataEngine.setConsumer(consumer);
	}

	/**
	 * Constructs a new CTP routing object that acts as a leaf.
	 * 
	 * @param consumer
	 *            a CtpDataProvider object that creates new data to send to the
	 *            sink.
	 */
	public Ctp(CtpDataProvider provider) {
		this(false);
		dataEngine.setProvider(provider);
	}

	public RoutingEngine getRoutingEngine() {
		return routingEngine;
	}

	public void start() {
		routingEngine.start();

		/*
		 * // debug info new Thread() { public void run() {
		 * Thread.sleep(Darjeeling.random()%DEBUG_INTERVAL);
		 * 
		 * while(true) { Thread.sleep(DEBUG_INTERVAL);
		 * 
		 * System.gc();
		 * 
		 * Debug.print(String.concat( "DEBUG id: ",
		 * Integer.toString(Darjeeling.getNodeId()), " mem: ",
		 * Integer.toString(Darjeeling.getMemFree()), " threads: ",
		 * Integer.toString(Darjeeling.getNrThreads()), "\n" ));
		 * Debug.print(String.concat( "DEBUG id: ",
		 * Integer.toString(Darjeeling.getNodeId()), " parent: ",
		 * Integer.toString(routingEngine.getParent()), " etx: ",
		 * Integer.toString(routingEngine.getEtx()), "\n" ));
		 * 
		 * NodeList nodeList = routingEngine.getNodeList();
		 * synchronized(nodeList) { for (short i=0; i<nodeList.size(); i++) {
		 * Node node = nodeList.get(i); Debug.print(String.concat(
		 * "DEBUG\t id: ", Integer.toString(node.id), " etx: ",
		 * Integer.toString(node.etx), " cost: ",
		 * Integer.toString(node.getCost()), "\n" )); } }
		 * 
		 * }
		 * 
		 * } }.start();
		 */
	}

}
