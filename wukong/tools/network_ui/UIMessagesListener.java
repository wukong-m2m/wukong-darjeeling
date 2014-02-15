import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public class UIMessagesListener implements NetworkServerMessagesListener {
	TextArea textArea;
	JTree tree;
	DefaultTreeModel treemodel;

	public UIMessagesListener (TextArea textArea, JTree tree, DefaultTreeModel treemodel) {
		this.textArea = textArea;
		this.tree = tree;
		this.treemodel = treemodel;
	}

	public void print(String msg) {
		this.textArea.append(msg);
	}
	public void println(String msg) {
		this.textArea.append(msg + "\n");
	}

	public String parseMessage(int[] message) {
		/*
		 NetworkServer header
		 byte 0   : length
		 byte 1,2 : src (litte endian)
		 byte 3,4 : dest (litte endian)
		 WKComm header
		 byte 5   : command
		 byte 6,7 : seqnr
		 bytes 8+ : payload
		*/
		if (message.length < 6) // ?
			return "";
		int command = message[5];
		String command_name = "UNKNOWN";
		switch(command) {
			case 0x10:
				command_name = "WKREPROG_COMM_CMD_REPROG_OPEN";
				break;
			case 0x11:
				command_name = "WKREPROG_COMM_CMD_REPROG_OPEN_R";
				break;
			case 0x12:
				command_name = "WKREPROG_COMM_CMD_REPROG_WRITE";
				break;
			case 0x13:
				command_name = "WKREPROG_COMM_CMD_REPROG_WRITE_R";
				break;
			case 0x14:
				command_name = "WKREPROG_COMM_CMD_REPROG_COMMIT";
				break;
			case 0x15:
				command_name = "WKREPROG_COMM_CMD_REPROG_COMMIT_R";
				break;
			case 0x16:
				command_name = "WKREPROG_COMM_CMD_REPROG_REBOOT";
				break;
			case 0x90:
				command_name = "WKPF_COMM_CMD_GET_WUCLASS_LIST";
				break;
			case 0x91:
				command_name = "WKPF_COMM_CMD_GET_WUCLASS_LIST_R";
				break;
			case 0x92:
				command_name = "WKPF_COMM_CMD_GET_WUOBJECT_LIST";
				break;
			case 0x93:
				command_name = "WKPF_COMM_CMD_GET_WUOBJECT_LIST_R";
				break;
			case 0x94:
				command_name = "WKPF_COMM_CMD_READ_PROPERTY";
				break;
			case 0x95:
				command_name = "WKPF_COMM_CMD_READ_PROPERTY_R";
				break;
			case 0x96:
				command_name = "WKPF_COMM_CMD_WRITE_PROPERTY";
				break;
			case 0x97:
				command_name = "WKPF_COMM_CMD_WRITE_PROPERTY_R";
				break;
			case 0x98:
				command_name = "WKPF_COMM_CMD_REQUEST_PROPERTY_INIT";
				break;
			case 0x99:
				command_name = "WKPF_COMM_CMD_REQUEST_PROPERTY_INIT_R";
				break;
			case 0x9A:
				command_name = "WKPF_COMM_CMD_GET_LOCATION";
				break;
			case 0x9B:
				command_name = "WKPF_COMM_CMD_GET_LOCATION_R";
				break;
			case 0x9C:
				command_name = "WKPF_COMM_CMD_SET_LOCATION";
				break;
			case 0x9D:
				command_name = "WKPF_COMM_CMD_SET_LOCATION_R";
				break;
			case 0x9E:
				command_name = "WKPF_COMM_CMD_GET_FEATURES";
				break;
			case 0x9F:
				command_name = "WKPF_COMM_CMD_GET_FEATURES_R";
				break;
			case 0xA0:
				command_name = "WKPF_COMM_CMD_SET_FEATURE";
				break;
			case 0xA1:
				command_name = "WKPF_COMM_CMD_SET_FEATURE_R";
				break;
			case 0xAF:
				command_name = "WKPF_COMM_CMD_ERROR_R";
				break;
		}
		return command_name;
	}


	public void messageDropped(int src, int dest, int[] message){
		this.print("Dropped message from " + src + " to " + dest + ", length " + message.length);
	}
	public void messageSent(int src, int dest, int[] message){
		String tmp = parseMessage(message);
		this.print("Forwarding message from " + src + " to " + dest + " (" + tmp + ") ");
		for (int i=0; i<message[0]; i++) {
			this.print(" [" + String.format("%02X ", message[i]) + "]");
		}
		this.println("");
	}

	public void updateClientInTree(int client) {
		DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
		for (int i=0; i<root.getChildCount(); i++) {
			if (root.getChildAt(i) instanceof DeviceTreeNode) {
				DeviceTreeNode device = (DeviceTreeNode)root.getChildAt(i);
				if (device.getClientId() == client)
                	this.treemodel.nodeChanged(device);
			}
		}
	}

	public void clientConnected(int client){
		this.println("Node " + client + " connected.");
		updateClientInTree(client);
	}
	public void clientDisconnected(int client){
		this.println("Node " + client + " disconnected.");
		updateClientInTree(client);
	}
}