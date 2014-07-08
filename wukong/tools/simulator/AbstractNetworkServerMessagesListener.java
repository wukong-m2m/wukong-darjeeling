import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public abstract class AbstractNetworkServerMessagesListener implements INetworkServerMessagesListener {
	public abstract void print(String msg);

	public void println(String msg) {
		this.print(msg + System.getProperty("line.separator"));
	}

	public void messageDropped(int src, int dest, int[] message) {
		String parsedCommand = NetworkMessageParser.parseMessage(message);
		this.println("DROPPED MESSAGE from " + src + " to " + dest + ": " + parsedCommand);
	}

	public void messageSent(int src, int dest, int[] message) {
		if (false) { // Useful for debugging
			this.print("            ");
			for (int i=0; i<message[0]; i++) {
				this.print("[" + String.format("%02X ", message[i]) + "] ");
			}
			this.println("");
		}
		String parsedCommand = NetworkMessageParser.parseMessage(message);
		this.println("Msg from " + src + " to " + dest + ": " + parsedCommand);
	}

	public void clientConnected(int client) {
		this.println("Node " + client + " connected.");
	}

	public void clientDisconnected(int client) {
		this.println("Node " + client + " disconnected.");
	}

	public void discovery(Integer[] ids) {
		this.print("Discovery: " + ids.length + " clients (");
		for (int i=0; i<ids.length; i++) {
			this.print(ids[i].toString());
			if (i != ids.length-1)
				this.print(", ");
		}
		this.println(")");
	}
}