import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;


public class NodeDetailsPanel extends JPanel {
	private TextArea textArea;

	public NodeDetailsPanel() {
		this.setLayout(new GridLayout(1,1));
		this.textArea = new TextArea();
		this.textArea.setEditable(false);
		this.textArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
		this.add(this.textArea);
	}

	public void setDeviceTreeNode(DeviceTreeNode node) {
		if (node == null) {
			this.textArea.setText("null");
			return;
		}

		if (!(node instanceof SimulatedDeviceTreeNode)) {
			this.textArea.setText("No details for client " + new Integer(node.getClientId()).toString());
			return;
		}

		String directory = ((SimulatedDeviceTreeNode)node).getDirectory();
		String archive_file = directory + "/app_infusion.dja";
		StringBuilder sb = new StringBuilder();
		sb.append("\nContents of app_infusion.dja for node " + node.getClientId() + ":\n\n");

		// open input file
		try {
			FileInputStream fileInput = new FileInputStream(archive_file);

			while (true) {
				int filelength = fileInput.read() + fileInput.read()*256;
				if (filelength == 0)
					break;

				int filetype = fileInput.read();
		        sb.append(String.format("FILE length %d, type '%s'\n", filelength, this.filetypeToString(filetype)));
		        int[] filedata = new int[filelength];
		        for (int i=0; i<filelength; i++)
		        	filedata[i] = fileInput.read();

				if (filetype == 0)
				    sb.append("\t Java archive\n");
				else if (filetype == 1)
				    sb.append("\t Java archive\n");
				else if (filetype == 2)
				    this.parseLinkTable(sb, filedata);
				else if (filetype == 3)
				    this.parseComponentMap(sb, filedata);
				else if (filetype == 4)
				    this.parseInitvalues(sb, filedata);
				sb.append("\n");
			}

			if (fileInput.available() == 0)
				sb.append("END OF ARCHIVE\n");
			else
				sb.append("UNEXPECTED DATA AFTER END OF ARCHIVE: " + fileInput.available() + " bytes.\n");
			fileInput.close();
		} catch (FileNotFoundException fnfex) {
			sb.append(archive_file + " not found.");
		} catch (IOException fnfex) {
			sb.append("IO Exception. :-(");
		}
		this.textArea.setText(sb.toString());
	}


	private void parseLinkTable(StringBuilder sb, int[] filedata) {
	    int number_of_links = filedata[0]+256*filedata[1];
		sb.append(String.format("\t%s: \t\t\t%d links\n", bts(filedata, 0, 2), number_of_links));
		for (int i=0; i<number_of_links; i++) {
			int[] link = Arrays.copyOfRange(filedata, 2+i*6, 2+i*6+6);
	        int fromcomponent = link[0]+link[1]*256;
	        int fromport = link[2];
	        int tocomponent = link[3]+link[4]*256;
	        int toport = link[5];
        	sb.append(String.format("\t%s: \t\tlink from (%d,%d) to (%d,%d)\n",
        												  bts(link, 0, 6),
                                                          fromcomponent,
                                                          fromport,
                                                          tocomponent,
                                                          toport));
	    }
	}

	private void parseComponentMap(StringBuilder sb, int[] filedata) {
	    int number_of_components = filedata[0]+256*filedata[1];
	    sb.append(String.format("\t%s: \t\t\t%d components\n", bts(filedata, 0, 2), number_of_components));

		int[] offsettable = Arrays.copyOfRange(filedata, 2, 2+(number_of_components)*2);
	    sb.append("\t\t\t\t\toffset table:\n");
	    for (int i=0; i<number_of_components; i++) {
	        int offset = offsettable[2*i]+offsettable[2*i+1]*256;
	        sb.append(String.format("\t%s: \t\t\t\tcomponent %d at offset %d\n", bts(offsettable, 2*i, 2), i, offset));
		}

	    int[] componenttable = Arrays.copyOfRange(filedata, 2+(number_of_components)*2, filedata.length);
	    int pos = 0;
	    sb.append("\t\t\t\t\tcomponents:\n");
	    for (int i=0; i<number_of_components; i++) {
	        int number_of_endpoints = componenttable[pos];
	        int wuclass = componenttable[pos+1]+componenttable[pos+2]*256;
	        sb.append(String.format("\t%s: \t\t\tcomponent %d, wuclass %d, %d endpoint(s):\n", bts(componenttable, pos, 3), i,  wuclass, number_of_endpoints));
	        pos += 3;
	        for (int j=0; j<number_of_endpoints; j++) {
	            int node = componenttable[pos]+componenttable[pos+1]*256;
	            int port = componenttable[pos+2];
	            sb.append(String.format("\t%s: \t\t\t\tnode %d, port %d\n", bts(componenttable, pos, 3), node, port));
	            pos += 3;
	        }
	    }
	}

	private void parseInitvalues(StringBuilder sb, int[] filedata) {
	    int number_of_initvalues = filedata[0]+256*filedata[1];
		sb.append(String.format("\t%s: \t\t\t%d initvalues\n", bts(filedata, 0, 2), number_of_initvalues));
		int pos = 2;
		for (int i=0; i<number_of_initvalues; i++) {
			int component_id = filedata[pos]+filedata[pos+1]*256;
			int property_number = filedata[pos+2];
			int value_size = filedata[pos+3];
	        sb.append(String.format("\t%s: \t\tcomponent %d, property %d, size %d\n", bts(filedata, pos, 4),
					                                                                    component_id,
					                                                                    property_number,
					                                                                    value_size));
	        String valuestr = "";
	        if (value_size == 1) {
	        	valuestr = new Integer(filedata[pos+4]).toString();
	        } else if (value_size == 2) {
	        	valuestr = new Integer(filedata[pos+4]+filedata[pos+5]*256).toString();
	        } else
	        	valuestr = "?";
	        sb.append(String.format("\t%s: \t\t\t\t\tvalue %s\n", bts(filedata, pos+4, value_size), valuestr));
	        pos += 4+value_size;
		}
	}

	private String bts(int[] bytes, int start, int length) {
		StringBuilder sb = new StringBuilder();
		while (length-- > 0) {
			sb.append(String.format("[%2x]", bytes[start++]));
		}
		return sb.toString();
	}

	private String filetypeToString(int type) {
		if (type == 0) return "library infusion";
        if (type == 1) return "application infusion";
        if (type == 2) return "wkpf link table";
        if (type == 3) return "wkpf component map";
        if (type == 4) return "wkpf initvalues";
        return "?";
	}
}


