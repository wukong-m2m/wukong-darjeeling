import java.io.*;

public class SensorTreeNode extends ActuatorTreeNode {
	public SensorTreeNode (String dir, String file) {
		super(dir, file);
		this.name = file.substring(3);
	}

	public void setValue(int value) {
		System.out.println("SET " + this.fullfilename + " TO " + value);
		try {
    		this.value = value;
			PrintWriter writer = new PrintWriter(this.fullfilename);
			writer.println(this.value.toString());
			writer.close();
		} catch (FileNotFoundException e) {
			System.out.println(e);
		}
	}
}
