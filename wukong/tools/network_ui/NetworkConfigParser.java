import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import java.io.*;
import java.util.*;

public class NetworkConfigParser {
	private File pathToNetworkConfigFile = null;

	public File pathToVM = null;
	public File pathToMasterServer = null;
	public File pathToNetworkDirectory = null;
	public List<VMNode> nodes = null;

	public NetworkConfigParser(String filename) {
		File configFile = new File(filename);
		if (!configFile.isFile()) {
			System.err.println("Config file " + configFile + " not found.");
			System.exit(1);
		}
		this.pathToNetworkConfigFile = configFile.getParentFile();

		try {
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(configFile);
			Node configNode = doc.getElementsByTagName("WuKongNetworkSimulatorConfig").item(0);
			NodeList configNodeChilds = configNode.getChildNodes();
 			for (int count = 0; count < configNodeChilds.getLength(); count++) {
				Node tempNode = configNodeChilds.item(count);
				if (tempNode.getNodeType() == Node.ELEMENT_NODE
						&& tempNode.getNodeName().equals("Paths")) {
					NamedNodeMap nodeMap = tempNode.getAttributes();
					for (int i = 0; i < nodeMap.getLength(); i++) {
						Node node = nodeMap.item(i);
						if (node.getNodeName().equals("master_server"))
							this.pathToMasterServer = getPathRelativeToConfigFile(node.getNodeValue());
						else if (node.getNodeName().equals("vm"))
							this.pathToVM = getPathRelativeToConfigFile(node.getNodeValue());
						else if (node.getNodeName().equals("network_directory"))
							this.pathToNetworkDirectory = getPathRelativeToConfigFile(node.getNodeValue());
					}
				} else if (tempNode.getNodeType() == Node.ELEMENT_NODE
						&& tempNode.getNodeName().equals("Nodes")) {
					this.nodes = parseNodes(tempNode);
				}
	 		}
		} catch (Exception e) {
			System.err.println("Exception while parsing config file.");
			System.err.println(e);
			System.exit(1);
		}
	}

	public class VMNode {
		public int clientId;
		public File enabledWuClassesXML;
	}

	private java.util.List<VMNode> parseNodes(Node nodesTag) {
		java.util.List<VMNode> list = new ArrayList<VMNode>();
		Node node = nodesTag.getChildNodes().item(0);
		while (node != null) {
			if (node.getNodeName().equals("Node")) {
				VMNode vmnode = new VMNode();
				NamedNodeMap nodeMap = node.getAttributes();
				for (int i = 0; i < nodeMap.getLength(); i++) {
					Node attributeNode = nodeMap.item(i);
					if (attributeNode.getNodeName().equals("enabled_wuclasses"))
						vmnode.enabledWuClassesXML = getPathRelativeToConfigFile(attributeNode.getNodeValue());
					else if (attributeNode.getNodeName().equals("id"))
						vmnode.clientId = Integer.parseInt(attributeNode.getNodeValue());
				}
				list.add(vmnode);
			}
			node = node.getNextSibling();
		}
		return list;
	}

	private File getPathRelativeToConfigFile(String path) {
		// Paths in the config file are relative to the config file.
		File d = new File(this.pathToNetworkConfigFile, path);
		return d;
	}
}