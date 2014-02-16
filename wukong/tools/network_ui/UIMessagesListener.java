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

	private final int WKPF_PROPERTY_TYPE_SHORT         = 0;
	private final int WKPF_PROPERTY_TYPE_BOOLEAN       = 1;
	private final int WKPF_PROPERTY_TYPE_REFRESH_RATE  = 2;
	private final int WKREPROG_OK                      = 0;
	private final int WKREPROG_REQUEST_RETRANSMIT      = 1;
	private final int WKREPROG_TOOLARGE                = 2; // Not used yet
	private final int WKREPROG_FAILED                  = 3; // Not used yet


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

	private String propertyTypeToString(int type) {
		switch(type) {
			case WKPF_PROPERTY_TYPE_SHORT       : return "short";
			case WKPF_PROPERTY_TYPE_BOOLEAN     : return "boolean";
			case WKPF_PROPERTY_TYPE_REFRESH_RATE: return "refresh_rate";
		}
		return "unknown";
	}

	private String parseMessage(int[] message) {
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
		if (message.length < 8) // ?
			return "";
		int command = message[5];
		int[] payload = Arrays.copyOfRange(message, 8, message.length);
		String command_name = "UNKNOWN";
		StringBuilder sb = new StringBuilder();
		switch(command) {
			case 0x10:
				command_name = "WKREPROG_OPEN";
				sb.append("bytes to write:" + (payload[1]*256+payload[0]));
				break;
			case 0x11:
				command_name = "WKREPROG_OPEN_R";
				if (payload[0] == WKREPROG_OK) {
					sb.append("OK pagesize:" + (payload[2]*256+payload[1]));
				} else if (payload[0] == WKREPROG_TOOLARGE)
					sb.append("TOO LARGE");
				break;
			case 0x12:
				command_name = "WKREPROG_WRITE";
				sb.append("offset:" + (payload[1]*256+payload[0]));
				sb.append(" data:");
				for (int i=2; i<payload.length; i++) {
					sb.append(" [" + String.format("%02X ", payload[i]) + "]");
				}
				break;
			case 0x13:
				command_name = "WKREPROG_WRITE_R";
				if (payload[0] == WKREPROG_OK)
					sb.append("OK");
				else if (payload[0] == WKREPROG_REQUEST_RETRANSMIT)
					sb.append("REQUEST RETRANSMIT FROM OFFSET " + (payload[2]*256+payload[1])); // Now this is LE again... Need to clean up this mess.
				break;
			case 0x14:
				command_name = "WKREPROG_COMMIT";
					sb.append(" ===" + payload[0] + "=== ");
					sb.append(" ===" + payload[1] + "=== ");
					sb.append("commit reprogramming of " + (payload[1]*256+payload[0]) + " bytes");
				break;
			case 0x15:
				command_name = "WKREPROG_COMMIT_R";
				if (payload[0] == WKREPROG_OK)
					sb.append("OK");
				else if (payload[0] == WKREPROG_REQUEST_RETRANSMIT)
					sb.append("REQUEST RETRANSMIT FROM OFFSET " + (payload[2]*256+payload[1])); // Now this is LE again... Need to clean up this mess.
				else if (payload[0] == WKREPROG_FAILED)
					sb.append("FAILED");
				break;
			case 0x16:
				command_name = "WKREPROG_REBOOT";
				break;
			case 0x90:
				command_name = "WKPF_GET_WUCLASS_LIST";
				sb.append("requested message part: " + payload[0]);
				break;
			case 0x91:
				command_name = "WKPF_GET_WUCLASS_LIST_R";
				sb.append("message part:" + (payload[0]+1)); // Add 1 because payload[0] is the message number, starting from 0, while payload[1] is the total number of messages.
				sb.append(" of " + payload[1]);
				sb.append(" total wuclasses:" + payload[2]);
				sb.append(" wuclasses:");
				for (int i = 3; i < payload.length; i+=3) {
					sb.append("{id:");
					sb.append((payload[i]*256 + payload[i+1])); // TODONR: change to little endian
					sb.append(" canCreate:");
					sb.append((payload[i+2] & 0x2) == 0 ? "False" : "True");
					sb.append(" virtual:");
					sb.append((payload[i+2] & 0x1) == 0 ? "False" : "True");
					sb.append("} ");
				}
				break;
			case 0x92:
				command_name = "WKPF_GET_WUOBJECT_LIST";
				sb.append("requested message part: " + payload[0]);
				break;
			case 0x93:
				command_name = "WKPF_GET_WUOBJECT_LIST_R";
				sb.append("message part:" + payload[0]+1);
				sb.append(" of " + payload[1]);
				sb.append(" total wuobjects:" + payload[2]);
				sb.append(" wuobjects:");
				for (int i = 3; i < payload.length; i+=4) {
					sb.append("{port:");
					sb.append(payload[i]);
					sb.append(" wuclass:");
					sb.append(payload[i+1]*256+payload[i+2]); // TODONR: change to little endian
					sb.append(" virtual:");
					sb.append((payload[i+3]) == 0 ? "False" : "True");
					sb.append("} ");
				}
				break;
			case 0x94:
				command_name = "WKPF_READ_PROPERTY";
				sb.append("port:" + payload[0]);
				sb.append(" property:" + payload[3]);
				sb.append(" wuclass:" + (payload[1]*256+payload[2]));
				break;
			case 0x95:
				command_name = "WKPF_READ_PROPERTY_R";
				sb.append("port:" + payload[0]);
				sb.append(" property:" + payload[3]);
				sb.append(" wuclass:" + (payload[1]*256+payload[2]));
				sb.append(" type:" + propertyTypeToString(payload[4]));
				sb.append(" status:" + String.format("%02X ", payload[5]));
				if (payload[4]==WKPF_PROPERTY_TYPE_SHORT || payload[4]==WKPF_PROPERTY_TYPE_REFRESH_RATE) {
					sb.append(" value:" + payload[6]*256 + payload[7]);
				} else {
					sb.append(" value:" + payload[6]);
				}
				break;
			case 0x96:
				command_name = "WKPF_WRITE_PROPERTY";
				sb.append("port:" + payload[0]);
				sb.append(" property:" + payload[3]);
				sb.append(" wuclass:" + (payload[1]*256+payload[2]));
				sb.append(" type:" + propertyTypeToString(payload[4]));
				if (payload[4]==WKPF_PROPERTY_TYPE_SHORT || payload[4]==WKPF_PROPERTY_TYPE_REFRESH_RATE) {
					sb.append(" value:" + payload[5]*256 + payload[6]);
				} else {
					sb.append(" value:" + payload[5]);
				}
				break;
			case 0x97:
				command_name = "WKPF_WRITE_PROPERTY_R";
				sb.append("OK");
				break;
			case 0x98:
				command_name = "WKPF_REQUEST_PROPERTY_INIT";
				sb.append("port:" + payload[0]);
				sb.append(" property:" + payload[1]);
				break;
			case 0x99:
				command_name = "WKPF_REQUEST_PROPERTY_INIT_R";
				sb.append("OK");
				break;
			case 0x9A:
				command_name = "WKPF_GET_LOCATION";
				sb.append("offset: " + payload[0]);
				break;
			case 0x9B:
				command_name = "WKPF_GET_LOCATION_R";
				sb.append("location: ");
				for (int i : payload)
					sb.append((char) i);
				break;
			case 0x9C:
				command_name = "WKPF_SET_LOCATION";
				sb.append("offset: " + payload[0] + " bytes: " + payload[1] + ", location: ");
				for (int i = 2; i < payload.length; i++)
					sb.append((char) payload[i]);
				break;
			case 0x9D:
				command_name = "WKPF_SET_LOCATION_R";
				sb.append("OK");
				break;
			case 0xAF:
				command_name = "WKPF_ERROR_R";
				sb.append("ERROR!!!!! code: " + payload[0]);
				break;
		}
		if (sb.length() == 0) {
			for (int i=8; i<message[0]; i++) {
				sb.append("[" + String.format("%02X ", message[i]) + "] ");
			}
		}

		return "(" + command_name + ") " + sb.toString();
	}

	public void messageDropped(int src, int dest, int[] message){
		this.print("DROPPED MESSAGE from " + src + " to " + dest + ", length " + message.length);
	}
	public void messageSent(int src, int dest, int[] message){
		if (false) { // Useful for debugging
			this.print("            ");
			for (int i=0; i<message[0]; i++) {
				this.print("[" + String.format("%02X ", message[i]) + "] ");
			}
			this.println("");
		}
		String parsedCommand = parseMessage(message);
		this.println("Msg from " + src + " to " + dest + ": " + parsedCommand);
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