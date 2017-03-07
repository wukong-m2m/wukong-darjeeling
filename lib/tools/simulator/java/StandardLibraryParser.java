import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import java.io.*;
import java.util.*;

public class StandardLibraryParser {
	private Map<Integer, WuClass> wuclasses;

	public class WuClass {
		public String name;
		public int id;
		private Map<Integer, Property> properties;

		public WuClass(Node wuclassNode) {
			NamedNodeMap nodeMap = wuclassNode.getAttributes();
			for (int i = 0; i < nodeMap.getLength(); i++) {
				Node node = nodeMap.item(i);
				if (node.getNodeName().equals("name"))
					this.name = node.getNodeValue();
				else if (node.getNodeName().equals("id"))
					this.id = Integer.parseInt(node.getNodeValue());
			}

			this.properties = new HashMap<Integer, Property>();
			int nextPropertyId = 0;
			NodeList wuclassChilds = wuclassNode.getChildNodes();
 			for (int count = 0; count < wuclassChilds.getLength(); count++) {
				Node node = wuclassChilds.item(count);
				if (node.getNodeType() == Node.ELEMENT_NODE
						&& node.getNodeName().equals("property")) {
					Property property = new Property(node, nextPropertyId++);
					this.properties.put(property.id, property);
				}
			}
		}

		Property getProperty(Integer id) {
			return properties.get(id);
		}
	}

	public class Property {
		public String name;
		public int id;		

		public Property(Node propertyNode, int id) {
			this.id = id;

			NamedNodeMap nodeMap = propertyNode.getAttributes();
			for (int i = 0; i < nodeMap.getLength(); i++) {
				Node node = nodeMap.item(i);
				if (node.getNodeName().equals("name"))
					this.name = node.getNodeValue();
			}
		}
	}

	public WuClass getWuClass(Integer id) {
		return this.wuclasses.get(id);
	}

	public StandardLibraryParser(String filename) {
		File standardLibraryFile = new File(filename);
		if (!standardLibraryFile.isFile()) {
			System.err.println("Config file " + standardLibraryFile + " not found.");
			System.exit(1);
		}

		this.wuclasses = new HashMap<Integer, WuClass>();

		try {
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(standardLibraryFile);
			Node standardLibraryNode = doc.getElementsByTagName("WuKong").item(0);
			NodeList standardLibraryChilds = standardLibraryNode.getChildNodes();
 			for (int count = 0; count < standardLibraryChilds.getLength(); count++) {
				Node node = standardLibraryChilds.item(count);
				if (node.getNodeType() == Node.ELEMENT_NODE
						&& node.getNodeName().equals("WuClass")) {
					WuClass wuclass = new WuClass(node);
					this.wuclasses.put(wuclass.id, wuclass);
				}
			}
		} catch (Exception e) {
			System.err.println("Exception while parsing standard library file.");
			System.err.println(e);
			e.printStackTrace();
			System.exit(1);
		}
	}
}
