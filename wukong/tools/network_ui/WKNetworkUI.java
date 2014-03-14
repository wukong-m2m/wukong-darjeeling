import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;
import java.lang.Runtime;
import name.pachler.nio.file.*;

public class WKNetworkUI extends JPanel implements TreeSelectionListener, ActionListener, DirectoryWatcherListener, NetworkServerMessagesListener {
    private static NetworkServer networkServer;
    private static StandardLibraryParser standardLibrary;
    private static ChildProcessManager childProcessManager;

    private DirectoryWatcher directorywatcher;

    // UI
    private JPanel sensorValuePanel;
    private JTextField sensorValueTextField;
    private JTree tree;
    private DefaultTreeModel treemodel;
    private NodeDetailsPanel nodeDetailsPanel;

    //Optionally play with line styles.  Possible values are
    //"Angled" (the default), "Horizontal", and "None".
    private static boolean playWithLineStyle = false;
    private static String lineStyle = "Horizontal";
    
    //Optionally set the look and feel.
    private static boolean useSystemLookAndFeel = true;

	private String networkdir;

    private SensorTreeNode selectedSensorTreeNode = null;

    public WKNetworkUI(String networkdir) {
        super(new GridLayout(1,0));

        File f = new File(networkdir);
        if (!f.isDirectory()) {
            System.err.println(networkdir + " not found or not a directory.");
            System.exit(1);
        }
        this.networkdir = networkdir;

        // Watch node directories for changes
        directorywatcher = new DirectoryWatcher(networkdir);

        //Create a tree that allows one selection at a time.
        DefaultMutableTreeNode top = new DefaultMutableTreeNode(networkdir);
        treemodel = new DefaultTreeModel(top);
        tree = new JTree(treemodel);
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);

        //Create the nodes.
        createNodes();

        //Set icons for sensors and actuators
        tree.setCellRenderer(new DefaultTreeCellRenderer() {
            private Icon sensorIcon = new ImageIcon("sensorIcon.png");
            private Icon actuatorIcon = new ImageIcon("actuatorIcon.png");
            private Icon unknownNodeDirectoryIcon = new ImageIcon("unknownNodeDirectoryIcon.png");
            private Icon runningChildProcessIcon = new ImageIcon("runningChildProcessIcon.png");
            private Icon stoppedChildProcessIcon = new ImageIcon("stoppedChildProcessIcon.png");
            private Icon externalClientIcon = new ImageIcon("externalClientIcon.png");
            @Override
            public Component getTreeCellRendererComponent(JTree tree, Object value, boolean selected, boolean expanded, boolean isLeaf, int row, boolean focused) {
                Component c = super.getTreeCellRendererComponent(tree, value, selected, expanded, isLeaf, row, focused);
                if (value instanceof SensorTreeNode) {
                    setIcon(sensorIcon);
                } else if (value instanceof ActuatorTreeNode) {
                    setIcon(actuatorIcon);
                } else if (value instanceof DeviceTreeNode) {
                    int clientId = ((DeviceTreeNode)value).getClientId();
                    if (WKNetworkUI.childProcessManager.hasChildProcess(clientId)) {
                        // This is a node for a child process managed by the UI
                        // (meaning we can start/stop it by double clicking on the tree node)
                        if (networkServer != null && networkServer.getConnectedClients().contains(clientId))
                            setIcon(runningChildProcessIcon);
                        else
                            setIcon(stoppedChildProcessIcon);
                    } else {
                        // This is either
                        //   - a node_X directory in the network directory, without a corresponding client connected to the network
                        //   - or, a connected client, not managed by the UI. This could be a Galileo joining the simulated network, or a simulated VM started externally
                        if (networkServer != null && networkServer.getConnectedClients().contains(clientId))
                            setIcon(externalClientIcon);
                        else
                            setIcon(unknownNodeDirectoryIcon);
                    }
                }
                return c;
            }
        });

        //Listen for when the selection changes.
        tree.addTreeSelectionListener(this);
        if (playWithLineStyle) {
            System.out.println("line style = " + lineStyle);
            tree.putClientProperty("JTree.lineStyle", lineStyle);
        }

        final WKNetworkUI final_this = this;
        MouseListener mouseListener = new MouseAdapter() {
            public void mousePressed(MouseEvent e) {
                int selRow = tree.getRowForLocation(e.getX(), e.getY());
                TreePath selPath = tree.getPathForLocation(e.getX(), e.getY());
                if(selRow != -1) {
                    if(e.getClickCount() == 2) {
                        Object lastComponent = selPath.getLastPathComponent();
                        if (lastComponent instanceof DeviceTreeNode)
                            final_this.deviceTreeNodeDoubleClicked((DeviceTreeNode)lastComponent);
                    }
                }
            }
        };
        tree.addMouseListener(mouseListener);


        //Create the scroll pane and add the tree to it. 
        JScrollPane treeView = new JScrollPane(tree);

        sensorValuePanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		sensorValueTextField = new JTextField("", 5);
		sensorValueTextField.addActionListener(this);
        sensorValuePanel.add(new JLabel("Sensor value: "));
        sensorValuePanel.add(sensorValueTextField);

        nodeDetailsPanel = new NodeDetailsPanel();

        JPanel detailsPanel = new JPanel(new BorderLayout());
        detailsPanel.add(sensorValuePanel, BorderLayout.NORTH);
        detailsPanel.add(nodeDetailsPanel, BorderLayout.CENTER);

        JScrollPane scrollPane = new JScrollPane(detailsPanel);


        //Add the scroll panes to a split pane.
        JSplitPane devicesPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        devicesPane.setTopComponent(treeView);
        devicesPane.setBottomComponent(scrollPane);

        Dimension minimumSize = new Dimension(100, 50);
        scrollPane.setMinimumSize(minimumSize);
        treeView.setMinimumSize(minimumSize);

        JTabbedPane logTabs = new JTabbedPane();
        WKNetworkUI.childProcessManager.setLogTabbedPane(logTabs);
        // Tab for network trace
        TextArea networkTraceTextArea = new TextArea();
        networkTraceTextArea.setEditable(false);
        logTabs.addTab("Network", networkTraceTextArea);
        // Start the network server
        WKNetworkUI.networkServer.addMessagesListener(new UIMessagesListener(networkTraceTextArea, WKNetworkUI.standardLibrary));
        WKNetworkUI.networkServer.addMessagesListener(this);

        //Add the split pane to this panel.
        JSplitPane mainPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        mainPane.setTopComponent(devicesPane);
        mainPane.setBottomComponent(logTabs);
        add(mainPane);
    }

    public void deviceTreeNodeDoubleClicked(DeviceTreeNode node) {
        int clientId = node.getClientId();
        if (this.childProcessManager.hasChildProcess(clientId)) {
            if (this.childProcessManager.isChildProcessRunning(clientId))
                this.childProcessManager.stopChildProcess(clientId);
            else
                this.childProcessManager.startChildProcess(clientId);
        }
    }

    /** Required by TreeSelectionListener interface. */
    public void valueChanged(TreeSelectionEvent e) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)tree.getLastSelectedPathComponent();
        if (node instanceof SensorTreeNode) {
        	this.setSelectedSensorTreeNode((SensorTreeNode)node);
        } else {
        	this.setSelectedSensorTreeNode(null);
        }

        TreePath path = tree.getSelectionPath();
        for (Object pathcomponent : path.getPath()) {
            if (pathcomponent instanceof DeviceTreeNode) {
                nodeDetailsPanel.setDeviceTreeNode((DeviceTreeNode)pathcomponent);
                return;
            }            
        }
        nodeDetailsPanel.setDeviceTreeNode(null);
    }
    private void setSelectedSensorTreeNode(SensorTreeNode s) {
        if (s == null) {
            sensorValueTextField.setText("");
            sensorValueTextField.setEnabled(false);
            sensorValueTextField.setVisible(false);
        } else {
            sensorValueTextField.setText(s.getValue().toString());
            sensorValueTextField.setVisible(true);
            sensorValueTextField.setEnabled(true);
        }
        this.selectedSensorTreeNode = s;
    }

    /** Required by ActionListener interface */
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == sensorValueTextField) {
            String text = sensorValueTextField.getText();
            try {
                int newval = Integer.parseInt(sensorValueTextField.getText());
                selectedSensorTreeNode.setValue(newval);
            } catch (NumberFormatException ex) {
            }
        }
    }

    /** Required by NetworkServerMessagesListener interface */
    public void messageDropped(int src, int dest, int[] message) {}
    public void messageSent(int src, int dest, int[] message) {}

    public void clientConnected(int client) {
        // Search for existing client
        DeviceTreeNode device = findDeviceTreeNode(client);
        if (device != null) {
            // Found: it must be a simulated node that wasn't connected to the simulator before.
            // Just update it to change the icon.
            this.treemodel.nodeChanged(device);
        } else {
            // Not found, could be an external (not simulated) node connecting.
            // Or it could be a node without a directory because it hasn't been created yet.
            // We'll add it as an external device for now, and replace the node if a directory appears later.
            addExternalDeviceTreeNode(client);
        }
    }

    public void clientDisconnected(int client) {
        DeviceTreeNode device = findDeviceTreeNode(client);
        if (device != null) {
            if (device instanceof SimulatedDeviceTreeNode
                || WKNetworkUI.childProcessManager.hasChildProcess(device.getClientId())) {
                // It's either a node filesystem in the network directory
                // or an external child process controlled (started/stopped) by the UI.
                // We keep both in the tree, and update the icon.
                this.treemodel.nodeChanged(device);
            } else {
                // It's an external device, not controlled by the UI. Remove it from the tree.
                removeDeviceTreeNode(client);
            }
        }
    }

    public void updateClientInTree(int client) {
    }

    private void rootNodeStructureChanged() {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        this.treemodel.nodeStructureChanged(root);
        // Expand everything (default is collapsed, how annoying.)
        for (int i = 0; i < tree.getRowCount(); i++) {
            tree.expandRow(i);
        }
    }

    private void addExternalDeviceTreeNode(int clientId) {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        DeviceTreeNode node = new ExternalDeviceTreeNode(clientId);
        root.add(node);
        rootNodeStructureChanged();
    }

    private void addSimulatedDeviceTreeNode(String subdirname) {
        File subdir = new File(networkdir, subdirname);
        if (subdir.isDirectory()) {
            DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
            try {
                DeviceTreeNode node = new SimulatedDeviceTreeNode(this.networkdir, subdirname, this.directorywatcher, this.treemodel);
                root.add(node);
                rootNodeStructureChanged();
            } catch (IOException e) {
                System.err.println("Error reading " + subdirname);                    
                System.err.println(e);
            }
        }
    }

    private void removeDeviceTreeNode(int clientId) {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        DeviceTreeNode device = findDeviceTreeNode(clientId);
        if (device != null) {
            root.remove(device);
            rootNodeStructureChanged();
        }
    }

    private DeviceTreeNode findDeviceTreeNode(int clientId) {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        for (int i=0; i<root.getChildCount(); i++) {
            if (root.getChildAt(i) instanceof DeviceTreeNode) {
                DeviceTreeNode device = (DeviceTreeNode)root.getChildAt(i);
                if (device.getClientId() == clientId) {
                    return device;
                }
            }
        }
        return null;
    }

    // Methods to scan the file system for nodes
    // Should probably be moved to a separate class (subclass of DefaultTreeModel? still not sure about how to properly do a tree in Java)
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
                if (filename.startsWith("node_")) {
                    int clientId = Integer.parseInt(filename.substring(5));
                    DeviceTreeNode device = findDeviceTreeNode(clientId);
                    if (device != null && device instanceof ExternalDeviceTreeNode) {
                        // This may really have been a simulated node that hadn't created
                        // its IO directory yet when the node connected to the simulator.
                        // Remove the external device node and replace it with a simulated one.
                        this.removeDeviceTreeNode(clientId);
                    }
                    this.addSimulatedDeviceTreeNode(filename);
                }
            } else if(e.kind() == StandardWatchEventKind.ENTRY_DELETE){
                Path context = (Path)e.context();
                String filename = context.toString();
                if (filename.startsWith("node_")) {
                    int clientId = Integer.parseInt(filename.substring(5));
                    this.removeDeviceTreeNode(clientId);
                }
            } else if(e.kind() == StandardWatchEventKind.OVERFLOW){
                System.err.println("OVERFLOW: more changes happened than we could retreive");
            }
        }
    }

    private void createNodes() {
		File folder = new File(this.networkdir);
		File[] listOfFiles = folder.listFiles(); 
		for (int i = 0; i < listOfFiles.length; i++) {
			if (listOfFiles[i].getName().startsWith("node_")) {
                addSimulatedDeviceTreeNode(listOfFiles[i].getName());
			}
		}
        directorywatcher.watchDirectory(this.networkdir, this);
        rootNodeStructureChanged();
    }
        
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event dispatch thread.
     */
    private static void createAndShowGUI(String networkdir) {
        if (useSystemLookAndFeel) {
            try {
                UIManager.setLookAndFeel(
                    UIManager.getSystemLookAndFeelClassName());
            } catch (Exception e) {
                System.err.println("Couldn't use system look and feel.");
            }
        }

        //Create and set up the window.
        JFrame frame = new JFrame("WKNetworkUI");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        //Add content to the window.
        WKNetworkUI ui = new WKNetworkUI(networkdir);
        frame.add(ui);

        //Display the window.
        frame.pack();
        frame.setVisible(true);
        ui.setSelectedSensorTreeNode(null);
    }

    public static void runMasterServer(String masterdir, String appdir) {
        ArrayList<String> commandline = new ArrayList<String>();
        commandline.add("python");
        commandline.add("-u");        
        commandline.add("master_server.py");
        if (appdir != null)
            commandline.add("-appdir=" + appdir);
        WKNetworkUI.childProcessManager.addChildProcess("master", commandline, masterdir, 1);
    }

    public static void runVMs(String vmdir, String networkdir, java.util.List<NetworkConfigParser.VMNode> vmsToStart) {
        for(NetworkConfigParser.VMNode vm : vmsToStart) {
            ArrayList<String> commandline = new ArrayList<String>();
            commandline.add("./darjeeling.elf");
            commandline.add("-i");
            commandline.add(new Integer(vm.clientId).toString());
            commandline.add("-d");
            commandline.add(networkdir);
            commandline.add("-e");
            commandline.add(vm.enabledWuClassesXML.getAbsolutePath());
            WKNetworkUI.childProcessManager.addChildProcess("vm " + vm.clientId, commandline, vmdir, vm.clientId);
        }
    }

    public static void main(final String[] args) {
        WKNetworkUI.childProcessManager = new ChildProcessManager();

        //Schedule a job for the event dispatch thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                boolean start_vms = true;
                boolean start_master = true;
                String wukong_root = "../../..";
                String scenario = "dollhouse";
                String networkdir = null;
                String scenarioname = null;
                String masterdir = null;
                String vmdir = null;
                String standardlibraryxml = null;
                String appdir = null;
                java.util.List<NetworkConfigParser.VMNode> vmsToStart = null;

                for (int i=0; i<args.length; i++) {
                    if (args[i].equals("-w")) {
                        // Override the wukong root if the simulator isn't started from <wkroot>/wukong/tools/network_ui
                        wukong_root = args[i+1];
                        i++; // skip wukong root dir
                    } else if (args[i].equals("-s")) {
                        // Load a network config
                        scenarioname = args[i+1];
                        i++; // skip master dir
                    } else if (args[i].equals("-d")) {
                        // Set a network directory to monitor if no network config is loaded (or override it, but that's not very useful)
                        networkdir = args[i+1];
                        i++; // skip network dir
                    } else if (args[i].equals("-m")) {
                        // Override the master directory
                        masterdir = args[i+1];
                        i++; // skip master dir
                    } else if (args[i].equals("-l")) {
                        // Override the standard library
                        standardlibraryxml = args[i+1];
                        i++; // skip standard library
                    } else if (args[i].equals("-no-vms")) {
                        // Don't start the VMs in the network config (so we can start them manually)
                        start_vms = false;
                    } else if (args[i].equals("-no-master")) {
                        // Don't start the master server (so we can start it manually)
                        start_master = false;
                    } else {
                        System.out.println("Options:");
                        System.out.println("\t-w <wkroot>\tOverride the wukong root if the simulator isn't started from <wkroot>/wukong/tools/network_ui");
                        System.out.println("\t-s <scenario>\tSet the scenario to load to <wkroot>/simulator_scenarios/<scenario>. Defaults to <wkroot>/wukong/dollhouse");
                        System.out.println("\t-d <dir>\tOverride the simulated IO directory to monitor. Defaults to the scenario directory.");
                        System.out.println("\t-m <dir>\tOverride the directory where the master server is located. Defaults to <wkroot>/wukong/master");
                        System.out.println("\t-m <file>\tOverride the standard library location. Defaults to <wkroot>/wukong/ComponentDefinitions/WuKongStandardLibrary.xml");
                        System.out.println("\t-no-vms\t\tStop the network UI from starting the VMs, so they may be run separately, for instance in a debugger.");
                        System.out.println("\t-no-master\tStop the network UI from starting the master, so it may be run separately.");
                        System.exit(1);
                    }
                }

                if (networkdir == null && scenarioname == null) {
                    System.out.println("USING DEFAULT DOLLHOUSE SCENARIO.");
                    scenarioname = "dollhouse";
                }
                if (scenarioname != null) {
                    System.out.println("LOADING " + scenarioname + ".");
                    NetworkConfigParser config = new NetworkConfigParser(wukong_root + "/wukong/simulator_scenarios/" + scenarioname + "/networkconfig.xml");
                    networkdir = config.pathToNetworkDirectory.getAbsolutePath();
                    vmsToStart = config.nodes;
                    if (config.pathToMasterApplicationsDirectory != null)
                        appdir = config.pathToMasterApplicationsDirectory.getAbsolutePath();
                }

                if (masterdir == null)
                    masterdir = new File(wukong_root + "/wukong/master").getAbsolutePath();
                if (vmdir == null)
                    vmdir = new File(wukong_root + "/src/config/native-simulator").getAbsolutePath();
                if (standardlibraryxml == null)
                    standardlibraryxml = new File(wukong_root + "/wukong/ComponentDefinitions/WuKongStandardLibrary.xml").getAbsolutePath();

                if (networkdir == null) {
                    System.err.println("Please specify at least the network directory.");
                    System.exit(1);
                }

                WKNetworkUI.standardLibrary = new StandardLibraryParser(standardlibraryxml);
                WKNetworkUI.networkServer = new NetworkServer();
                WKNetworkUI.networkServer.start();
                createAndShowGUI(networkdir);
                if (start_master)
                    runMasterServer(masterdir, appdir);
                if (start_vms && vmsToStart != null)
                    runVMs(vmdir, networkdir, vmsToStart);
            }
        });
    }
}