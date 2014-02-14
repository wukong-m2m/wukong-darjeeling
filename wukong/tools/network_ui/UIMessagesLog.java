import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public class UIMessagesLog implements NetworkServerMessagesListener {
	TextArea textArea;
	public UIMessagesLog (TextArea textArea) {
		this.textArea = textArea;
	}

	public void log(String msg) {
		this.textArea.append(msg + "\n");
	}

	public void messageDropped(int src, int dest, int[] message) {

	}

	public void messageSent(int src, int dest, int[] message) {

	}

	public void clientConnected(int client) {
		this.log("Client " + client + " connected.");
	}

	public void clientDisconnected(int client) {
		this.log("Client " + client + " disconnected.");
	}
}