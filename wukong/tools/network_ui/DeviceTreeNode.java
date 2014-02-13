import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.util.*;
import java.io.*;
import name.pachler.nio.file.*;
public class DeviceTreeNode extends DefaultMutableTreeNode implements DirectoryWatcherListener {
	private String directory;
	private String name;
	private String location = "UNKNOWN";
	private Map<String, SensorTreeNode> sensors;
	private Map<String, ActuatorTreeNode> actuators;
	private DefaultMutableTreeNode sensorsGroupNode;
	private DefaultMutableTreeNode actuatorsGroupNode;
	private DefaultTreeModel treemodel;

	public DeviceTreeNode(String network_directory, String node_directory, DirectoryWatcher directorywatcher, DefaultTreeModel treemodel) throws java.io.IOException {
		this.directory = network_directory + '/' + node_directory;
		this.name = node_directory;
		this.sensors = new HashMap<String, SensorTreeNode>();
		this.actuators = new HashMap<String, ActuatorTreeNode>();
		this.treemodel = treemodel;
		directorywatcher.watchDirectory(this.directory, this);
		this.update_info();
	}

	public String toString() {
		return name;
	}

    public void directoryChanged(WatchKey signalledKey) {
        // get list of events from key
        java.util.List<WatchEvent<?>> list = signalledKey.pollEvents();

        // VERY IMPORTANT! call reset() AFTER pollEvents() to allow the
        // key to be reported again by the watch service
        signalledKey.reset();

        // we'll simply print what has happened; real applications
        // will do something more sensible here
        for(WatchEvent e : list){
            String message = "";
            if(e.kind() == StandardWatchEventKind.ENTRY_CREATE){
                Path context = (Path)e.context();
                message = context.toString() + " created";
            } else if(e.kind() == StandardWatchEventKind.ENTRY_MODIFY){
                Path context = (Path)e.context();
                String filename = context.toString();
                message = filename + " modified";
                if (this.sensors.containsKey(filename)) {
                	this.sensors.get(filename).update_info();
                	this.treemodel.nodeChanged(this.sensors.get(filename));
                }
                if (this.actuators.containsKey(filename)) {
                	this.actuators.get(filename).update_info();
                	this.treemodel.nodeChanged(this.actuators.get(filename));
                }
            } else if(e.kind() == StandardWatchEventKind.OVERFLOW){
                message = "OVERFLOW: more changes happened than we could retreive";
            } else
                message = e.toString();
            System.out.println(message);
        }
    }

	private void update_info() throws java.io.IOException {
		File folder = new File(this.directory);
		File[] listOfFiles = folder.listFiles(); 

		for (int i = 0; i < listOfFiles.length; i++) 
		{
			if (listOfFiles[i].isFile()) 
			{
				String filename = listOfFiles[i].getName();
				if (filename.startsWith("IN_") || filename.startsWith("OUT_")) {
					if (filename.startsWith("IN_")) {
						if (this.sensorsGroupNode == null) {
							this.sensorsGroupNode = new DefaultMutableTreeNode("sensors");
							this.add(this.sensorsGroupNode);
						}
						SensorTreeNode sensor = new SensorTreeNode(directory, filename);
						this.sensorsGroupNode.add(sensor);
						this.sensors.put(filename, sensor);
					} else {
						if (this.actuatorsGroupNode == null) {
							this.actuatorsGroupNode = new DefaultMutableTreeNode("sensors");
							this.add(this.actuatorsGroupNode);
						}
						ActuatorTreeNode actuator = new ActuatorTreeNode(directory, filename);
						this.actuatorsGroupNode.add(actuator);
						this.actuators.put(filename, actuator);
					}
				}
				if (filename.equals("config.txt")) {
					try {
						BufferedReader br = new BufferedReader(new FileReader(new File(directory, filename)));

						String config_line;
						while ((config_line = br.readLine()) != null) {
							if (config_line.equals("Location (in raw bytes on the next line):")) {
								int location_length = br.read();
								config_line = br.readLine();
								this.location = config_line.substring(0, location_length);
								break;
							}
						}
						br.close();
					} catch (IOException e) {
						this.location = "UNKNOWN";
						System.out.println(e);
					}
				}
			}
		}
	}
}