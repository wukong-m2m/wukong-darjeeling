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

		// open input file
		try {
			FileInputStream fileInput = new FileInputStream(archive_file);

			while (true) {
				int filelength = fileInput.read()  + fileInput.read()*256;
				if (filelength == 0)
					break;

				int filetype = fileInput.read();
		        sb.append(String.format("FILE length %d, type '%s'\n", filelength, this.filetypeToString(filetype)));
		        byte[] filedata = new byte[filelength];
		        fileInput.read(filedata);
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


	private void parseLinkTable(StringBuilder sb, byte[] filedata) {

	}
	private void parseComponentMap(StringBuilder sb, byte[] filedata) {

	}
	private void parseInitvalues(StringBuilder sb, byte[] filedata) {
	    int number_of_initvalues = filedata[0]+256*filedata[1];
		sb.append(String.format("\t%s: \t\t\t%d initvalues\n", bts(filedata, 0, 2), number_of_initvalues));
		int pos = 2;
		for (int i=0; i<number_of_initvalues; i++) {
			int component_id = filedata[pos]+filedata[pos+1]*256;
			int property_number = filedata[pos+2];
			int value_size = filedata[pos+3];
	        sb.append(String.format("\t%s: \t\t\tcomponent %d, property %d, size %d\n", bts(filedata, pos, 4),
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
	        sb.append(String.format("\t%s: \t\t\t\tvalue %s\n", bts(filedata, pos+4, value_size), valuestr));
	        pos += 4+value_size;
		}		
    // initvalues = filedata[2:]
    // pos = 0
    // for i in range(number_of_initvalues):
    //     component_id = initvalues[pos]+initvalues[pos+1]*256
    //     property_number = initvalues[pos+2]
    //     value_size = initvalues[pos+3]
    //     print "\t%s: \t\t\tcomponent %d, property %d, size %d" % (str(initvalues[pos:pos+4]),
    //                                                               component_id,
    //                                                               property_number,
    //                                                               value_size)
    //     value = initvalues[pos+4:pos+4+value_size]
    //     if value_size == 1:
    //         valuestr = str(value[0])
    //     elif value_size == 2:
    //         valuestr = str(value[0]+value[1]*256)
    //     else:
    //         valuestr = str(value)
    //     print "\t%s: \t\t\t\tvalue %s" % (str(value), valuestr)
    //     pos += 4+value_size

	}

	private String bts(byte[] bytes, int start, int length) {
		StringBuilder sb = new StringBuilder();
		while (length-- > 0) {
			sb.append(String.format("[%x] ", bytes[start++]));
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


