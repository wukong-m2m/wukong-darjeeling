/*
 * Copyright (c) 1995, 2008, Oracle and/or its affiliates. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   - Neither the name of Oracle or the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 



import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public class WKNetworkUI extends JPanel implements TreeSelectionListener {
    private JPanel sensorPanel;
    private JTextField sensorTextField;
    private JTree tree;
    private DefaultTreeModel treemodel;
    private URL helpURL;
    private static boolean DEBUG = false;

    //Optionally play with line styles.  Possible values are
    //"Angled" (the default), "Horizontal", and "None".
    private static boolean playWithLineStyle = false;
    private static String lineStyle = "Horizontal";
    
    //Optionally set the look and feel.
    private static boolean useSystemLookAndFeel = false;

	private String networkdir = "/Users/niels/git/darjeeling/src/config/native-dollhouse/dollhouse/";

    private DefaultMutableTreeNode selectedSensorTreeNode = null;

    public WKNetworkUI() {
        super(new GridLayout(1,0));

        //Create the nodes.
        DefaultMutableTreeNode top = new DefaultMutableTreeNode(networkdir);
        treemodel = new DefaultTreeModel(top);
        createNodes(top);

        //Create a tree that allows one selection at a time.
        tree = new JTree(treemodel);
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		for (int i = 0; i < tree.getRowCount(); i++) {
		    tree.expandRow(i);
		}

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
		sensorTextField.addActionListener(new SensorTextHandler());
		sensorTextField.setEnabled(false);
		sensorPanel.add(sensorTextField);
        JScrollPane sensorView = new JScrollPane(sensorPanel);

        //Add the scroll panes to a split pane.
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        splitPane.setTopComponent(treeView);
        splitPane.setBottomComponent(sensorView);

        Dimension minimumSize = new Dimension(100, 50);
        sensorView.setMinimumSize(minimumSize);
        treeView.setMinimumSize(minimumSize);

        //Add the split pane to this panel.
        add(splitPane);
    }

	private class SensorTextHandler implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (e.getSource() == sensorTextField) {
				String text = sensorTextField.getText();
				try {
					int newval = Integer.parseInt(sensorTextField.getText());
                    Sensor selectedSensor = (Sensor)selectedSensorTreeNode.getUserObject();
					selectedSensor.setValue(newval);
                    treemodel.nodeChanged(selectedSensorTreeNode);
				} catch (NumberFormatException ex) {
				}
			}
		}
	}

    private void setSelectedSensorTreeNode(DefaultMutableTreeNode s) {
    	if (s == null) {
    		sensorTextField.setText("");
    		sensorTextField.setEnabled(false);
    	} else {
    		sensorTextField.setText(((Sensor)s.getUserObject()).getValue().toString());
    		sensorTextField.setEnabled(true);
    	}
    	this.selectedSensorTreeNode = s;
    }

    /** Required by TreeSelectionListener interface. */
    public void valueChanged(TreeSelectionEvent e) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                           tree.getLastSelectedPathComponent();

        if (node == null) return;

        if (node.getUserObject() instanceof Sensor) {
        	this.setSelectedSensorTreeNode(node);
        } else {
        	this.setSelectedSensorTreeNode(null);
        }

    }

    private void createNodes(DefaultMutableTreeNode top) {
		File folder = new File(this.networkdir);
		File[] listOfFiles = folder.listFiles(); 
		for (int i = 0; i < listOfFiles.length; i++) {
			if (listOfFiles[i].isDirectory() && listOfFiles[i].getName().startsWith("node_")) {
				String dirname = listOfFiles[i].getName();
				try {
					Node node = new Node(this.networkdir + dirname);
				    DefaultMutableTreeNode treenode = new DefaultMutableTreeNode(dirname);
				    top.add(treenode);
					treenode.add(new DefaultMutableTreeNode("Location: " + node.location));
				    if (node.sensors.size() > 0) {
					    DefaultMutableTreeNode treenode_sensors = new DefaultMutableTreeNode("sensors");
					    treenode.add(treenode_sensors);
					    for (Sensor s : node.sensors.values()) {
						    treenode_sensors.add(new DefaultMutableTreeNode(s));
						}
				    }
				    if (node.actuators.size() > 0) {
					    DefaultMutableTreeNode treenode_actuators = new DefaultMutableTreeNode("actuators");
					    treenode.add(treenode_actuators);
					    for (Actuator a : node.actuators.values()) {
						    treenode_actuators.add(new DefaultMutableTreeNode(a));
						}
				    }
				} catch (IOException e) {
					top.add(new DefaultMutableTreeNode("Failed to read " + dirname));
				}
			}
		}
    }
        
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event dispatch thread.
     */
    private static void createAndShowGUI() {
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
        frame.add(new WKNetworkUI());

        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }

    private class Actuator {
    	protected String name;
    	protected String fullfilename;
    	protected Integer value;

    	public Actuator(String dir, String file) {
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

    private class Sensor extends Actuator {
    	public Sensor (String dir, String file) {
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


	private class Node {
		private String node_directory = "UNKNOWN";
		public String location = "UNKNOWN";
		public Map<String, Sensor> sensors;
		public Map<String, Actuator> actuators;

		public Node(String node_directory) throws java.io.IOException {
			this.node_directory = node_directory;
			this.sensors = new HashMap<String, Sensor>();
			this.actuators = new HashMap<String, Actuator>();
			this.update_info();
		}

		private void update_info() throws java.io.IOException {
			File folder = new File(this.node_directory);
			File[] listOfFiles = folder.listFiles(); 

			for (int i = 0; i < listOfFiles.length; i++) 
			{
				if (listOfFiles[i].isFile()) 
				{
					String filename = listOfFiles[i].getName();
					if (filename.startsWith("IN_") || filename.startsWith("OUT_")) {
						if (filename.startsWith("IN_")) {
							if (this.sensors.containsKey(filename))
								this.sensors.get(filename).update_info();
							else
								this.sensors.put(filename, new Sensor(node_directory, filename));
						}
						else {
							if (this.actuators.containsKey(filename))
								this.actuators.get(filename).update_info();
							else
								this.actuators.put(filename, new Actuator(node_directory, filename));
						}
					}
					if (filename.equals("config.txt")) {
						try {
							BufferedReader br = new BufferedReader(new FileReader(new File(node_directory, filename)));

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

    public static void main(String[] args) {
        //Schedule a job for the event dispatch thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                createAndShowGUI();
            }
        });
    }
}