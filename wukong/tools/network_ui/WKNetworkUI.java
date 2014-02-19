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
    private JPanel sensorPanel;
    private JTextField sensorTextField;
    private JTree tree;
    private DefaultTreeModel treemodel;

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
            private Icon sensorIcon = new ImageIcon("sensor.png");
            private Icon actuatorIcon = new ImageIcon("actuator.png");
            private Icon connectedDeviceTreeNodeIcon = UIManager.getIcon("InternalFrame.maximizeIcon");
            private Icon disconnectedDeviceTreeNodeIcon = UIManager.getIcon("InternalFrame.closeIcon");
            private Icon externalDeviceTreeNodeIcon = UIManager.getIcon("InternalFrame.minimizeIcon");
            @Override
            public Component getTreeCellRendererComponent(JTree tree, Object value, boolean selected, boolean expanded, boolean isLeaf, int row, boolean focused) {
                Component c = super.getTreeCellRendererComponent(tree, value, selected, expanded, isLeaf, row, focused);
                if (value instanceof SensorTreeNode) {
                    setIcon(sensorIcon);
                } else if (value instanceof ActuatorTreeNode) {
                    setIcon(actuatorIcon);
                } else if (value instanceof ExternalDeviceTreeNode) {
                    setIcon(externalDeviceTreeNodeIcon);
                } else if (value instanceof SimulatedDeviceTreeNode) {
                    int clientId = ((DeviceTreeNode)value).getClientId();
                    if (networkServer != null && networkServer.getConnectedClients().contains(clientId))
                        setIcon(connectedDeviceTreeNodeIcon);
                    else
                        setIcon(disconnectedDeviceTreeNodeIcon);
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
                        System.out.println("DOUBLE CLICK " + selRow + " path " + selPath);
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

        //Create the HTML viewing pane.
        sensorPanel = new JPanel();
		sensorTextField = new JTextField("", 20);
		sensorTextField.addActionListener(this);
		sensorPanel.add(sensorTextField);
        JScrollPane sensorView = new JScrollPane(sensorPanel);

        //Add the scroll panes to a split pane.
        JSplitPane devicesPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        devicesPane.setTopComponent(treeView);
        devicesPane.setBottomComponent(sensorView);

        Dimension minimumSize = new Dimension(100, 50);
        sensorView.setMinimumSize(minimumSize);
        treeView.setMinimumSize(minimumSize);

        JSplitPane mainPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        mainPane.setTopComponent(devicesPane);
        TextArea textArea = new TextArea();
        mainPane.setBottomComponent(textArea);

        // Start the network server
        WKNetworkUI.networkServer.addMessagesListener(new UIMessagesListener(textArea, WKNetworkUI.standardLibrary));
        WKNetworkUI.networkServer.addMessagesListener(this);

        //Add the split pane to this panel.
        add(mainPane);
    }

    public void deviceTreeNodeDoubleClicked(DeviceTreeNode node) {
        // niels
    }

    /** Required by TreeSelectionListener interface. */
    public void valueChanged(TreeSelectionEvent e) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)tree.getLastSelectedPathComponent();
        if (node instanceof SensorTreeNode) {
        	this.setSelectedSensorTreeNode((SensorTreeNode)node);
        } else {
        	this.setSelectedSensorTreeNode(null);
        }
    }
    private void setSelectedSensorTreeNode(SensorTreeNode s) {
        if (s == null) {
            sensorTextField.setText("");
            sensorTextField.setEnabled(false);
            sensorTextField.setVisible(false);
        } else {
            sensorTextField.setText(s.getValue().toString());
            sensorTextField.setVisible(true);
            sensorTextField.setEnabled(true);
        }
        this.selectedSensorTreeNode = s;
    }

    /** Required by ActionListener interface */
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == sensorTextField) {
            String text = sensorTextField.getText();
            try {
                int newval = Integer.parseInt(sensorTextField.getText());
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
            if (device instanceof SimulatedDeviceTreeNode) {
                // Simulated node disconnected. Update the icon.
                this.treemodel.nodeChanged(device);
            } else {
                // External device disconnected. Remove it from the tree.
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

    public static void runMasterServer(String masterdir) {
        try {
            WKNetworkUI.childProcessManager.forkChildProcess("master server", "python master_server.py", masterdir);
        } catch (IOException e) {
            System.err.println("Can't start master server");
            System.err.println(e);
            System.exit(1);
        }
    }

    public static void runVMs(String vmdir, String networkdir, java.util.List<NetworkConfigParser.VMNode> vmsToStart) {
        for(NetworkConfigParser.VMNode vm : vmsToStart) {
            String commandline = String.format("./darjeeling.elf -i %d -d %s -e %s",
                                                vm.clientId,
                                                networkdir,
                                                vm.enabledWuClassesXML.getAbsolutePath());
            try {
                WKNetworkUI.childProcessManager.forkChildProcess("vm " + vm.clientId, commandline, vmdir);
            } catch (IOException e) {
                System.err.println("Can't start node VM " + vm.clientId);
                System.err.println(e);
                System.exit(1);
            }
        }
    }

    public static void main(final String[] args) {
        WKNetworkUI.childProcessManager = new ChildProcessManager();

        //Schedule a job for the event dispatch thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                String networkdir = null;
                String masterdir = null;
                String vmdir = null;
                String standardlibraryxml = null;
                java.util.List<NetworkConfigParser.VMNode> vmsToStart = null;

                for (int i=0; i<args.length; i++) {
                    if (args[i].equals("-d")) {
                        networkdir = args[i+1];
                        i++; // skip network dir
                    } else if (args[i].equals("-m")) {
                        masterdir = args[i+1];
                        i++; // skip master dir
                    } else if (args[i].equals("-c")) {
                        NetworkConfigParser config = new NetworkConfigParser(args[i+1]);
                        networkdir = config.pathToNetworkDirectory.getAbsolutePath();
                        masterdir = config.pathToMasterServer.getAbsolutePath();
                        vmdir = config.pathToVM.getAbsolutePath();
                        standardlibraryxml = config.pathToStandardLibrary.getAbsolutePath();
                        vmsToStart = config.nodes;
                        i++; // skip master dir
                    } else if (args[i].equals("-l")) {
                        standardlibraryxml = args[i+1];
                        i++; // skip standard library
                    }
                }

                if (networkdir == null) {
                    System.err.println("Please specify at least the network directory.");
                    System.exit(1);
                }

                if (standardlibraryxml != null)
                    WKNetworkUI.standardLibrary = new StandardLibraryParser(standardlibraryxml);
                WKNetworkUI.networkServer = new NetworkServer();
                WKNetworkUI.networkServer.start();
                createAndShowGUI(networkdir);
                if (masterdir != null)
                    runMasterServer(masterdir);
                if (vmsToStart != null)
                    runVMs(vmdir, networkdir, vmsToStart);
            }
        });
    }
}