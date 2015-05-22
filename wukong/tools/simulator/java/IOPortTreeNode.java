import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.util.*;
import java.io.*;

public abstract class IOPortTreeNode extends DefaultMutableTreeNode {
	protected String name;
	protected String fullfilename;
	protected Integer value;

	public IOPortTreeNode(String dir, String file) {
		this.name = file.substring(4);
		this.fullfilename = dir + "/" + file;
		this.update_info();
	}

	public void update_info() {
		this.value = null;
		try {
			BufferedReader br = new BufferedReader(new FileReader(new File(this.fullfilename)));
			String ioport_data = br.readLine();
			this.value = Integer.parseInt(ioport_data);
			br.close();    		
		} catch (IOException e) {
			System.out.println(e);
		}
	}

	public Integer getValue() {
		return value;
	}

	public String toString() {
		return name + " = " + value;
	}
}