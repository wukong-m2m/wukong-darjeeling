/*
 * NodeList.java
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
 * List of nodes used by the routing engine.
 * 
 * @author Niels Brouwers
 * 
 */
public class NodeList {

	// the maximum number of times a unicast message to a node may fail before
	// it is declared dead and pruned from the list
	private static int MAX_SEND_ERRORS = 5;

	// number of nodes in the list
	private byte size;

	// node list
	private Node nodes[];

	/**
	 * Creates a new NodeList object with a given maximum size.
	 * 
	 * @param maxSize
	 */
	public NodeList(short maxSize) {
		nodes = new Node[maxSize];
		size = 0;
	}

	/**
	 * @return the number of elements currently in the node list
	 */
	public byte size() {
		return size;
	}

	/**
	 * Creates a new Node object with the given id and etx and inserts it into
	 * the list. If the a Node object with the given id is already in the list
	 * that object is returned. If the list is full and the given etx is lower
	 * than the maximum etx in the list, the Node object with the highest etx is
	 * removed. If the etx is higher than the max etx in the list and the list
	 * is full, the method returns null.
	 * 
	 * @param id
	 *            id of the Node object to create
	 * @param etx
	 *            etx of the Node object to create
	 * @return a Node object with the given id and etx or null if the list is
	 *         full and the etx is higher than the highest etx in the list.
	 */
	public Node insert(short id, short etx) {
		Node ret = null;

		// don't add nodes with an etx of -1 to the list
		if (etx == -1)
			return null;

		synchronized (this) {
			// check if the node is already in the list
			for (byte i = 0; i < size; i++) {
				if (nodes[i].id == id)
					return nodes[i];
			}

			// check if the node can be added to the list
			if (size < nodes.length) {
				ret = nodes[size] = new Node(id, etx);
				size++;
			} else {
				// release the node with the highest ETX
				short highestEtx = -1;
				byte highestEtxIndex = 0;
				for (byte i = 0; i < nodes.length; i++) {
					if (nodes[i].etx > highestEtx) {
						highestEtx = nodes[i].etx;
						highestEtxIndex = i;
					}
				}

				// If the node with the highest etx in the nodelist has a higher
				// etx than the
				// offered etx, replace that node with a new one
				if (highestEtx > etx)
					ret = nodes[highestEtxIndex] = new Node(id, etx);
			}
		}
		return ret;
	}

	/**
	 * Searches for a node with the given node id and returns it. If the element
	 * is not found null is returned.
	 * 
	 * @param nodeId
	 *            node id to search for
	 * @return a Node object with the given id or null if not found
	 */
	public Node getNodeById(short nodeId) {
		Node ret = null;
		synchronized (this) {
			// look for an existing node
			for (short i = 0; i < size; i++) {
				if (nodes[i].id == nodeId)
					return nodes[i];
			}
		}
		return ret;
	}

	/**
	 * Elects a Node in the list with the lowest cost.
	 * 
	 * @return the Node in the list with the lowest cost.
	 */
	public Node getParent() {
		Node ret = null;
		synchronized (this) {
			short etx, lowestEtx = -1;

			// elect the node with the lowest total etx to sink
			for (short i = 0; i < size; i++) {
				etx = nodes[i].getCost();
				if (etx >= 0 && (etx < lowestEtx || i == 0)) {
					ret = nodes[i];
					lowestEtx = etx;
				}
			}

		}
		return ret;
	}

	/**
	 * Signals to the node list that a unicast packet to a given node was not
	 * delivered successfully. If the number of send errors for that node equals
	 * MAX_SEND_ERRORS the node is declared dead and removed from the list.
	 * 
	 * @param id
	 *            node id
	 * @param tries
	 *            number of tries before the handler gave up
	 */
	public void sendError(short id, byte tries) {
		synchronized (this) {
			for (short i = 0; i < size; i++)
				if (nodes[i].id == id) {
					nodes[i].updateHistory(tries);
					nodes[i].sendErrors++;
					if (nodes[i].sendErrors >= MAX_SEND_ERRORS) {
						// remove node
						nodes[i] = nodes[size - 1];
						nodes[size - 1] = null;
						size--;
					}
					return;
				}
		}
	}

	/**
	 * Signals to the node list that a unicast packet to a given node was
	 * delivered successfully.
	 * 
	 * @param id
	 *            node id
	 * @param tries
	 *            number of tries before an ack was received
	 */
	public void sendOk(short id, byte tries) {
		synchronized (this) {
			Node node = getNodeById(id);
			if (node != null) {
				node.updateHistory(tries);
				node.setSendErrors((byte) 0);
			}
		}
	}

	/**
	 * Gets a node by index
	 * 
	 * @param i
	 *            list index
	 * @return the ith element in the list
	 */
	public Node get(short i) {
		return nodes[i];
	}

	/**
	 * Elects a node in the list at index nr modulo size. This method can be
	 * used with a random number generator or a round-robin scheme.
	 * 
	 * @param nr
	 *            index
	 * @return the node in the list at index nr modulo size
	 */
	public Node elect(short nr) {
		if (size == 0)
			return null;

		synchronized (this) {
			return nodes[nr % size];
		}
	}

}
