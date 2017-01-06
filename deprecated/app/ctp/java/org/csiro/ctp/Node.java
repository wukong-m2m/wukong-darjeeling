/*
 * Node.java
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
 * The Node class keeps track of information about a single neighbor node.
 * 
 * @author Niels Brouwers
 * 
 */
public class Node {

	// history size for the link quality estimation
	private final static byte HISTORY_LENGTH = 4;

	// node id
	public short id;

	// node etx
	public short etx;

	// the number of times a message to this node was not delivered successfully
	// used for detecting that a node has 'died'
	public byte sendErrors;

	// tracks how many tries were needed to send a unicast message for the last
	// HISTORY_LENGTH messages
	public byte triesHistory[];

	// ring buffer
	public byte historyStart, historySize;

	/**
	 * Creates a ew Node object with the given ID and ETX values
	 * 
	 * @param id
	 * @param etx
	 */
	public Node(short id, short etx) {
		this.id = id;
		this.etx = etx;

		this.triesHistory = new byte[HISTORY_LENGTH];
		this.historyStart = 0;
		this.historySize = 0;
	}

	/**
	 * Creates a ew Node object with the given ID and ETX set to MAXETX
	 * 
	 * @param id
	 */
	public Node(short id) {
		this(id, (short) -1);
	}

	/**
	 * Calculates the cost of sending data to the sink through this node.
	 * 
	 * @return
	 */
	public short getCost() {
		short cost = 0;

		if (historySize > 0 && etx >= 0) {
			for (byte i = 0; i < historySize; i++)
				cost += triesHistory[(historyStart + HISTORY_LENGTH - i - 1) % HISTORY_LENGTH];
			cost = (short) ((cost * 100) / historySize);
			cost += etx;
		} else
			cost = -1;

		return (short) (cost);
	}

	/**
	 * Puts a new value in the history ring buffer.
	 * 
	 * @param tries
	 */
	public void updateHistory(byte tries) {
		triesHistory[historyStart] = tries;
		historyStart = (byte) ((historyStart + 1) % HISTORY_LENGTH);
		if (historySize < HISTORY_LENGTH)
			historySize++;
	}

	/**
	 * @param sendErrors
	 *            the number of send errors (undelivered packets) to this node
	 */
	public void setSendErrors(byte sendErrors) {
		this.sendErrors = sendErrors;
	}

	/**
	 * @return the number of send errors (undelivered packets) to this node
	 */
	public byte getSendErrors() {
		return sendErrors;
	}

}
