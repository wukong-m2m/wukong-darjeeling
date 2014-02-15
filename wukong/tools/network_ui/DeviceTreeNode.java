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
	private Map<String, IOPortTreeNode> ioPorts;
	private DefaultMutableTreeNode sensorsGroupNode;
	private DefaultMutableTreeNode actuatorsGroupNode;
	private DefaultTreeModel treemodel;

	public DeviceTreeNode(String network_directory, String node_directory, DirectoryWatcher directorywatcher, DefaultTreeModel treemodel) throws java.io.IOException {
		this.directory = network_directory + '/' + node_directory;
		this.name = node_directory;
		this.ioPorts = new HashMap<String, IOPortTreeNode>();
		this.treemodel = treemodel;
		directorywatcher.watchDirectory(this.directory, this);
		this.init();
	}

	public String toString() {
		return name + " (" + this.location + ")";
	}

	public int getClientId() {
		// Assuming nodes are named "node_X"
		return Integer.parseInt(name.substring(5));
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
            if(e.kind() == StandardWatchEventKind.ENTRY_CREATE){
                Path context = (Path)e.context();
                String filename = context.toString();
				if (filename.startsWith("IN_") || filename.startsWith("OUT_"))
					this.addIoPortTreeNode(filename);
            } else if(e.kind() == StandardWatchEventKind.ENTRY_MODIFY){
                Path context = (Path)e.context();
                String filename = context.toString();
                if (filename.equals("config.txt")) {
                	this.update_info();
                } else if (this.ioPorts.containsKey(filename)) {
                	this.ioPorts.get(filename).update_info();
                	this.treemodel.nodeChanged(this.ioPorts.get(filename));
                }
            } else if(e.kind() == StandardWatchEventKind.ENTRY_DELETE){
                Path context = (Path)e.context();
                String filename = context.toString();
				if (filename.startsWith("IN_") || filename.startsWith("OUT_"))
					this.removeIoPortTreeNode(filename);
            } else if(e.kind() == StandardWatchEventKind.OVERFLOW){
                System.err.println("OVERFLOW: more changes happened than we could retreive");
			}
        }
    }

    private void update_info() {
    	File configfile = new File(this.directory, "config.txt");
    	if (configfile.isFile()) {
			try {
				BufferedReader br = new BufferedReader(new FileReader(configfile));

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
    	this.treemodel.nodeChanged(this);
    }

    private void addIoPortTreeNode(String filename) {
		IOPortTreeNode ioPortTreeNode;
		if (filename.startsWith("IN_"))
			ioPortTreeNode = new SensorTreeNode(directory, filename);
		else
			ioPortTreeNode = new ActuatorTreeNode(directory, filename);
		this.add(ioPortTreeNode);
		this.treemodel.nodeStructureChanged(this);
		this.ioPorts.put(filename, ioPortTreeNode);
    }

    private void removeIoPortTreeNode(String filename) {
    	if (this.ioPorts.containsKey(filename)) {
    		IOPortTreeNode port = this.ioPorts.get(filename);
    		this.remove(port);
			this.treemodel.nodeStructureChanged(this);
    		this.ioPorts.remove(filename);
    	}
    }

	private void init() throws java.io.IOException {
		File folder = new File(this.directory);
		File[] listOfFiles = folder.listFiles(); 

		for (int i = 0; i < listOfFiles.length; i++) 
		{
			if (listOfFiles[i].isFile()) 
			{
				String filename = listOfFiles[i].getName();
				if (filename.startsWith("IN_") || filename.startsWith("OUT_")) {
					this.addIoPortTreeNode(filename);
				}
			}
		}
		this.update_info();
	}
}