/*
 * WKPFTableTask.java
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */ 

package org.csiro.darjeeling.wkpftable.ant;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;


import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;

/**
 * 
 * Ant task that converts a text file containing the WuKong link table and component map to a
 * binary file in the format wkpf_links.c expects.
 */
public class WKPFTableTask extends Task
{
	// Number of output values per line
	private final static int LINESIZE = 16;
	
	// Source, destination files and output array name.
	private String src, dest;
	// Desired size of the array. Defaults to the size of the archive if 0. Error if !=0 but < archive size.
	private int arraysize;

	// Keyword list. 
	private ArrayList<String> keywords = new ArrayList<String>();
	
	// If true, the array will be made constant using the 'const' keyword. 
	private boolean constKeyword = true;
	
	/**
	 * Ant execute entry point.
	 */
	public void execute()
	{
				// make sure properties are set
		if (src==null) throw new BuildException("Source file name not set");
		if (dest==null) throw new BuildException("Destination file name not set");

        // Check if the file needs to be regenerated
		long srcDate =new File(src).lastModified();
        long destDate=new File(dest).lastModified();
        if(destDate > srcDate) {
            log("This file is up to date",Project.MSG_VERBOSE);
            return ;
        }

        try {
			File fXmlFile = new File(src);
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(fXmlFile);
			doc.getDocumentElement().normalize();

			writeFile(dest + ".wkpf_linktable", makeLinkTable(doc));
			writeFile(dest + ".wkpf_componentmap", makeComponentMap(doc));
			writeFile(dest + ".wkpf_initvalues", makeInitValues(doc));
		} catch (FileNotFoundException fnfex) {
			throw new org.apache.tools.ant.BuildException("File not found: " + src);
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while reading: " + src);
		} catch (Exception e) {
			e.printStackTrace();
		}



	}

	private void writeFile(String filename, ArrayList<Byte> data) {
		try {
			FileOutputStream fout = new FileOutputStream(filename);
			for (int i=0; i<data.size(); i++)
				fout.write((byte)data.get(i));
			fout.close();
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while writing: " + src);
		}
	}


	private ArrayList<Byte> makeLinkTable(Document doc) {
		NodeList links = ((Element)doc.getElementsByTagName("links").item(0)).getElementsByTagName("link");

		ArrayList<Byte> links_bytes = new ArrayList<Byte>();

		// Two bytes: number of links, little endian
		links_bytes.add((byte)(links.getLength() % 256));
		links_bytes.add((byte)(links.getLength() / 256));
		// The link table
		for (int i=0; i<links.getLength(); i++) {
			Node node = links.item(i);
			if (node.getNodeType() == Node.ELEMENT_NODE) {
				Element link = (Element)node;
				// six bytes per link
				links_bytes.add((byte)(Short.parseShort(link.getAttribute("fromComponent")) % 256));
				links_bytes.add((byte)(Short.parseShort(link.getAttribute("fromComponent")) / 256));
				links_bytes.add(Byte.parseByte(link.getAttribute("fromProperty")));
				links_bytes.add((byte)(Short.parseShort(link.getAttribute("toComponent")) % 256));
				links_bytes.add((byte)(Short.parseShort(link.getAttribute("toComponent")) / 256));
				links_bytes.add(Byte.parseByte(link.getAttribute("toProperty")));
			}
		}
		return links_bytes;
	}

	private ArrayList<Byte> makeComponentMap(Document doc) {
		NodeList components = ((Element)doc.getElementsByTagName("components").item(0)).getElementsByTagName("component");

		ArrayList<Integer> components_offsets = new ArrayList<Integer>();
		ArrayList<Byte> component_map_bytes = new ArrayList<Byte>();

		// The component map
		int expected_component_id = 0;
		for (int i=0; i<components.getLength(); i++) {
			Node node = components.item(i);
			if (node.getNodeType() == Node.ELEMENT_NODE) {
				Element component = (Element)node;
				// Check if this is the component we're expecting.
				// The XML should contain a continuous range starting at 0.
				int component_id = Integer.parseInt(component.getAttribute("id"));
				if (component_id != expected_component_id)
					throw new org.apache.tools.ant.BuildException("Unexpected component is: " + component_id + " expected: " + expected_component_id + ". Component ids should be a continuous range starting at 0.");
				components_offsets.add(component_map_bytes.size()); // Offset of this component

				NodeList endpoints = component.getElementsByTagName("endpoint");
				component_map_bytes.add((byte)endpoints.getLength()); // Number of endpoints
				component_map_bytes.add((byte)(Short.parseShort(component.getAttribute("wuclassId")) % 256));
				component_map_bytes.add((byte)(Short.parseShort(component.getAttribute("wuclassId")) / 256));
				for (int j=0; j<endpoints.getLength(); j++) {
					Node node2 = endpoints.item(j);
					if (node2.getNodeType() == Node.ELEMENT_NODE) {
						Element endpoint = (Element)node2;
						component_map_bytes.add((byte)(Short.parseShort(endpoint.getAttribute("node")) % 256));
						component_map_bytes.add((byte)(Short.parseShort(endpoint.getAttribute("node")) / 256));
						component_map_bytes.add(Byte.parseByte(endpoint.getAttribute("port")));
					}
				}
				expected_component_id++;
			}
		}

		ArrayList<Byte> components_bytes = new ArrayList<Byte>();
		// Two bytes: number of components, little endian
		components_bytes.add((byte)(components.getLength() % 256));
		components_bytes.add((byte)(components.getLength() / 256));
		// Offset table containing an offset from the beginning of the component map
		for(int i=0; i<components_offsets.size(); i++) {
			int offset = components_offsets.get(i);
			offset += 2; // for the number of components
			offset += components_offsets.size()*2; // for the offset table
			// offset is now a true offset from the beginning of the components table
			components_bytes.add((byte)(offset % 256));
			components_bytes.add((byte)(offset / 256));
		}
		// The actual component map
		components_bytes.addAll(component_map_bytes);
		return components_bytes;
	}

	private ArrayList<Byte> makeInitValues(Document doc) {
		NodeList initvalues = ((Element)doc.getElementsByTagName("initvalues").item(0)).getElementsByTagName("initvalue");

		ArrayList<Byte> initvalues_bytes = new ArrayList<Byte>();

		// Two bytes: number of init values, little endian
		initvalues_bytes.add((byte)(initvalues.getLength() % 256));
		initvalues_bytes.add((byte)(initvalues.getLength() / 256));
		// The initial values table
		for (int i=0; i<initvalues.getLength(); i++) {
			Node node = initvalues.item(i);
			if (node.getNodeType() == Node.ELEMENT_NODE) {
				Element initvalue = (Element)node;
				initvalues_bytes.add((byte)(Short.parseShort(initvalue.getAttribute("componentId")) % 256));
				initvalues_bytes.add((byte)(Short.parseShort(initvalue.getAttribute("componentId")) / 256));
				initvalues_bytes.add((byte)(Byte.parseByte(initvalue.getAttribute("propertyNumber"))));
				byte valuesize = (byte)Byte.parseByte(initvalue.getAttribute("valueSize"));
				initvalues_bytes.add(valuesize);
				if (valuesize == 1) { // TODO: make this more flexible, but for now we only have 1 or 2 byte values
					try {
						initvalues_bytes.add(Byte.parseByte(initvalue.getAttribute("value")));
					} catch(Exception e) {
						initvalues_bytes.add((byte)0);
					}
				} else if (valuesize == 2) {
					try {
						initvalues_bytes.add((byte)(Short.parseShort(initvalue.getAttribute("value")) % 256));
						initvalues_bytes.add((byte)(Short.parseShort(initvalue.getAttribute("value")) / 256));
					} catch(Exception e) {
						initvalues_bytes.add((byte)0);
						initvalues_bytes.add((byte)0);
					}
				} else {
					throw new org.apache.tools.ant.BuildException("Initvalue found with valueSize: " + valuesize);
				}
			}
		}

		return initvalues_bytes;
	}
	
	/**
	 * Sets the source file name.
	 * @param src source file name
	 */
	public void setSrc(String src)
	{
		this.src = src;
	}

	/**
	 * Sets the destination file name.
	 * @param dest destination file name
	 */
	public void setDest(String dest)
	{
		this.dest = dest;
	}
}
