/*
 * InfuserArguments.java
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
 

package org.csiro.darjeeling.infuser;
import static org.csiro.darjeeling.infuser.logging.Logging.VerboseOutputType.ARGUMENTS_PARSING;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.bcel.classfile.ClassParser;
import org.apache.bcel.classfile.JavaClass;
import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalHeader;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

/**
 * Set of arguments and switches for the Infuser class. 
 * 
 * 
 * @author Niels Brouwers
 *
 */
public class InfuserArguments
{
	
	// Name of the infusion
	private String infusionName;
	
	// Infusion verion
	private int infusionVersion;
	
	// List of output file names. If any of these are left NULL no output will be generated 
	// for the corresponding type. 
	private String infusionOutputFile, headerOutputFile, cHeaderOutputFile, cCodeOutputFile, debugOutputFile;
	
	// Lists of input files
	private ArrayList<String> classFiles, headerFiles;
	
	// Used for caching the last modified time so that it is not recalculated
	// for every getLastModified call
	private long lastModified = 0;
	
	// Regexp pattern for -key=value style arguments 
	private final static Pattern optionPattern = Pattern.compile("-(\\p{Alpha}+)=(.*)");
	
	/**
	 *  Creates a new, empty InfuserArguments instance. Can be used to programatically drive
	 *  the infuser rather than through command line arguments. 
	 */
	public InfuserArguments()
	{
		infusionName = null;
		infusionVersion = 0;
		classFiles = new ArrayList<String>();
		headerFiles = new ArrayList<String>();
	}
	
	/**
	 *  Creates a new instance from a set of command line arguments.
	 *  These can be in the form of
	 *  <ul>
	 *  	<li>A file name, i.e. CTP.class, base.dih</li>
	 *  	<li>A key/value pair, i.e. -key=value</li>
	 *  </ul>
	 *  This method will not check for completeness or correctness, it will not 
	 *  check if the infusion name is set, if the list of classes is empty, etc. 
	 *  @param args command line arguments. 
	 *  @throws FileNotFoundException
	 */
	public static InfuserArguments parse(String args[]) throws FileNotFoundException
	{
		InfuserArguments arguments = new InfuserArguments();
		
		for (String arg : args)
			arguments.parseArgument(arg);
		
		return arguments;
	}
	
	/**
	 * Parses a single command line argument. 
	 *  These can be in the form of
	 *  <ul>
	 *  	<li>A file name, i.e. CTP.class, base.dih</li>
	 *  	<li>A key/value pair, i.e. -key=value</li>
	 *  </ul>
	 * @param arg command line argument
	 * @throws FileNotFoundException
	 */
	public void parseArgument(String arg) throws FileNotFoundException
	{
		
		// check if the argument is an option string of the form -[name]=[value]
		Matcher matcher = optionPattern.matcher(arg);
		if (matcher.matches())
		{
			String optionName = matcher.group(1);
			String optionValue = matcher.group(2);
			applyOption(optionName, optionValue);
		} 
		// check if the argument is a file name of type .class
		else if (arg.endsWith(".class"))
		{
			if (!fileExists(arg)) throw new FileNotFoundException("Class file " + arg + " does not exist");
			classFiles.add(arg);
		}
		else if (arg.endsWith(".dih"))
		{
			if (!fileExists(arg)) throw new FileNotFoundException("Header file " + arg + " does not exist");
			headerFiles.add(arg);
		} else
			Logging.instance.printlnVerbose(ARGUMENTS_PARSING, String.format("Ignoring unknown option %s", arg));
		
	}
	
	// applies a key/value pair
	private void applyOption(String name, String value)
	{
		
		if (name.equals("name")) { this.infusionName = value; return; }
		if (name.equals("o")) { this.infusionOutputFile = value; return; }
		if (name.equals("h")) { this.headerOutputFile = value; return; }
		if (name.equals("d")) { this.cHeaderOutputFile = value; return; }
		if (name.equals("n")) { this.cCodeOutputFile = value; return; }
	
		// infusion version
		if (name.equals("infusionversion"))
		{
			try {
				this.infusionVersion = Integer.parseInt(value);
				return;
			} catch (NumberFormatException ex)
			{
				throw new ArgumentParseException("The value for option 'infusionversion' should be numeric");
			}
		}

		// unknown option
		Logging.instance.printlnVerbose(ARGUMENTS_PARSING, String.format("Ignoring unknown option %s", name));
		
	}
	
	// Convenience method
	private static boolean fileExists(String name)
	{
		File file = new File(name);
		return file.exists();
	}
	
	/**
	 * Creates a new InternalInfusion object from the parameters in this class. It creates a header
	 * with the given name and version, parses and adds all the class files and header files.
	 * The resulting Infusion object is then ready for further processing.
	 * @return a new InternalInfusion instance with classes and headers loaded
	 * @throws InfuserException
	 */
	public InternalInfusion createInfusion() throws InfuserException
	{
		// create an infusion and create the header
		InternalHeader header = new InternalHeader(infusionName, infusionVersion);
		InternalInfusion infusion = new InternalInfusion(header);
		
		// add header files
		for (String headerFileName : headerFiles)
		{
			Logging.instance.printlnVerbose(ARGUMENTS_PARSING, String.format("Loading header file %s", headerFileName));
			if (!fileExists(headerFileName))
			{
				throw new InfuserException(String.format("File %s does not exist", headerFileName));
			} else
			{
				try {
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
					DocumentBuilder db = dbf.newDocumentBuilder();
					Document dom = db.parse(headerFileName);
					ExternalInfusion headerInfusion = ExternalInfusion.fromDocument(dom, headerFileName);
					infusion.addInfusion(headerInfusion);
				} catch(SAXException ex)
				{
					throw new InfuserException(String.format("Unable to parse class file %s", headerFileName), ex);
				} catch (ParserConfigurationException ex)
				{
					throw new InfuserException(String.format("Unable to parse class file %s", headerFileName), ex);
				} catch (IOException ex)
				{
					throw new InfuserException(String.format("Unable to parse class file %s", headerFileName), ex);
				}
			}
		}

		// add class files
		for (String classFileName : classFiles)
		{
			Logging.instance.printlnVerbose(ARGUMENTS_PARSING, String.format("Loading class file %s", classFileName));
			if (!fileExists(classFileName))
			{
				throw new InfuserException(String.format("File %s does not exist", classFileName));
			} else
			{
				try {
					ClassParser parser = new ClassParser(classFileName);
					JavaClass javaClass = parser.parse();
					infusion.addJavaClass(javaClass);
				} catch (IOException ex)
				{
					throw new InfuserException(String.format("Unable to parse class file %s", classFileName), ex);
				}
			}
		}
		
		return infusion;
	}
	
	private long getFileLastModified(String fileName)
	{
		File file = new File(fileName);
		return file.lastModified();
	}
	
	private long getLastModified()
	{
		long fileLastModified;

		if (lastModified==0)
		{
			for (String className : classFiles)
				if ((fileLastModified=getFileLastModified(className))>lastModified) lastModified = fileLastModified;
	
			for (String headerFile : headerFiles)
				if ((fileLastModified=getFileLastModified(headerFile))>lastModified) lastModified = fileLastModified;
		}
		
		return lastModified;			
	}
	
	public boolean isUpToDate()
	{
		String[] files = new String[] {	infusionOutputFile, headerOutputFile, cHeaderOutputFile, cCodeOutputFile };
		for (String fileName : files)
		{
			if (fileName!=null)
			{
				File file = new File(fileName);
				if (file.lastModified()<getLastModified())
					return false;
			}
		}
		
		return true;
	}
	
	/**
	 * @return the infusionName
	 */
	public String getInfusionName()
	{
		return infusionName;
	}

	/**
	 * @return the infusionVersion
	 */
	public int getInfusionVersion()
	{
		return infusionVersion;
	}

	/**
	 * @return the classFiles
	 */
	public Collection<String> getClassFiles()
	{
		return classFiles;
	}

	/**
	 * @return the headerFiles
	 */
	public Collection<String> getHeaderFiles()
	{
		return headerFiles;
	}

	public String getInfusionOutputFile()
	{
		return infusionOutputFile;
	}
	
	public String getHeaderOutputFile()
	{
		return headerOutputFile;
	}
	
	public String getDefinitionOutputFile()
	{
		return cHeaderOutputFile;
	}
	
	public void setInfusionOutputFile(String infusionOutputFile)
	{
		this.infusionOutputFile = infusionOutputFile;
	}
	
	public void setHeaderOutputFile(String headerOutputFile)
	{
		this.headerOutputFile = headerOutputFile;
	}
	
	public void setDefinitionOutputFile(String definitionOutputFile)
	{
		this.cHeaderOutputFile = definitionOutputFile;
	}
		
	public void setInfusionVersion(int infusionVersion)
	{
		this.infusionVersion = infusionVersion;
	}
	
	public void setInfusionName(String infusionName)
	{
		this.infusionName = infusionName;
	}
	
	public void setNativeOutputFile(String nativeOutputFile)
	{
		this.cCodeOutputFile = nativeOutputFile;
	}
	
	public String getNativeOutputFile()
	{
		return cCodeOutputFile;
	}
	
	public void setDebugOutputFile(String debugOutputFile)
	{
		this.debugOutputFile = debugOutputFile;
	}
	
	public String getDebugOutputFile()
	{
		return debugOutputFile;
	}

}
