import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;
import name.pachler.nio.file.*;

public class WKNetworkUI extends JPanel implements TreeSelectionListener, ActionListener, DirectoryWatcherListener, NetworkServerMessagesListener {
    private DirectoryWatcher directorywatcher;
    private NetworkServer networkServer;

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
            private Icon sensorIcon = UIManager.getIcon("InternalFrame.maximizeIcon");
            private Icon actuatorIcon = UIManager.getIcon("InternalFrame.closeIcon");
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

        //Create the scroll pane and add the tree to it. 
        JScrollPane treeView = new JScrollPane(tree);

        //Create the HTML viewing pane.
        sensorPanel = new JPanel();
		sensorTextField = new JTextField("", 20);
		sensorTextField.addActionListener(this);
		sensorTextField.setEnabled(false);
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
        networkServer = new NetworkServer();
        networkServer.addMessagesListener(new UIMessagesListener(textArea, tree, treemodel));
        networkServer.addMessagesListener(this);
        networkServer.start();

        //Add the split pane to this panel.
        add(mainPane);
    }

    /** Required by TreeSelectionListener interface. */
    public void valueChanged(TreeSelectionEvent e) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                           tree.getLastSelectedPathComponent();

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
        } else {
            sensorTextField.setText(s.getValue().toString());
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
    public void clientConnected(int client) {}
    public void clientDisconnected(int client) {}

    private void rootNodeStructureChanged() {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        this.treemodel.nodeStructureChanged(root);
        // Expand everything (default is collapsed, how annoying.)
        for (int i = 0; i < tree.getRowCount(); i++) {
            tree.expandRow(i);
        }
    }

    private void addDeviceTreeNode(String subdirname) {
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

    private void removeDeviceTreeNode(String subdirname) {
        int client = Integer.parseInt(subdirname.substring(5));
        DefaultMutableTreeNode root = (DefaultMutableTreeNode)this.tree.getModel().getRoot();
        for (int i=0; i<root.getChildCount(); i++) {
            if (root.getChildAt(i) instanceof DeviceTreeNode) {
                DeviceTreeNode device = (DeviceTreeNode)root.getChildAt(i);
                if (device.getClientId() == client) {
                    root.remove(device);
                    rootNodeStructureChanged();
                }
            }
        }
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
                if (filename.startsWith("node_"))
                    this.addDeviceTreeNode(filename);
            } else if(e.kind() == StandardWatchEventKind.ENTRY_DELETE){
                Path context = (Path)e.context();
                String filename = context.toString();
                if (filename.startsWith("node_"))
                    this.removeDeviceTreeNode(filename);
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
                addDeviceTreeNode(listOfFiles[i].getName());
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
        frame.add(new WKNetworkUI(networkdir));

        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }

    public static void main(final String[] args) {
        //Schedule a job for the event dispatch thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                if (args.length > 0)
                    createAndShowGUI(args[0]);
                else
                    createAndShowGUI("/Users/niels/git/darjeeling/src/config/native-dollhouse/dollhouse/");
            }
        });
    }
}