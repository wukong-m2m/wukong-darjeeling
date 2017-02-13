/*
 * Infuser.java
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

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.csiro.darjeeling.infuser.checkphase.ClassResolveVisitor;
import org.csiro.darjeeling.infuser.checkphase.InstructionsImplementedCheckVisitor;
import org.csiro.darjeeling.infuser.checkphase.JavaClassCheckVisitor;
import org.csiro.darjeeling.infuser.checkphase.ListSizeLimitationsCheckVisitor;
import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.outputphase.CFileVisitor;
import org.csiro.darjeeling.infuser.outputphase.CHeaderVisitor;
import org.csiro.darjeeling.infuser.outputphase.DIWriterVisitor;
import org.csiro.darjeeling.infuser.outputphase.DebugVisitor;
import org.csiro.darjeeling.infuser.outputphase.HeaderVisitor;
import org.csiro.darjeeling.infuser.processingphase.ClassInitialiserResolutionVisitor;
import org.csiro.darjeeling.infuser.processingphase.CodeBlockVisitor;
import org.csiro.darjeeling.infuser.processingphase.FieldMapVisitor;
import org.csiro.darjeeling.infuser.processingphase.FindEntryPointVisitor;
import org.csiro.darjeeling.infuser.processingphase.HeaderResolutionVisitor;
import org.csiro.darjeeling.infuser.processingphase.IndexVisitor;
import org.csiro.darjeeling.infuser.processingphase.InterfaceListFlattenVisitor;
import org.csiro.darjeeling.infuser.processingphase.StringTableVisitor;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.w3c.dom.Document;

/**
 * Entry point for the infuser. All the transformation stuff is started from here. Front-ends must create
 * an InfuserArguments instance and create an Infuser object with it. Then the process function can be called
 * that will generate the desired output files.
 *
 * @author Niels Brouwers
 *
 */
public class Infuser
{
	// Version of the infusion format. VM should check if it matches the VM's format
	public static final int infusionFormatVersion = Opcode.Use32BitArrayIndex ? 106 : 107;

	// never start with version 1.0.0 :-)
	public static final String version = "1.1.12";
	
	// friendly greeting
    public static final String greeting = String.format("This is Darjeeling Infuser v%s", version);
    
	// Infuser argument instance
    private InfuserArguments infuserArguments;
    
	/** 
	 * Constructs a new Infuser that outputs verbose messages to System.out 
	 * @param infuserArguments InfuserArguments instance
	 */
	public Infuser(InfuserArguments infuserArguments)
	{
		this.infuserArguments = infuserArguments;
	}
	
	/**
	 * Creates an infusion file (.di)
	 * @param infusion the Infusion to output
	 * @throws InfuserException 
	 */
	private void createInfusionFile(InternalInfusion infusion) throws InfuserException
	{
		String outFile = infuserArguments.getInfusionOutputFile();
		if (outFile!=null)
		{
			Logging.instance.println("Writing infusion file: " + outFile);
			try {
				FileOutputStream fout = new FileOutputStream(outFile);
				infusion.accept(new DIWriterVisitor(fout, infusion));
				fout.close();
			} catch (IOException ex)
			{
				throw new InfuserException("Error writing infusion file", ex);
			}
		}
	}
	
	/**
	 * Creates an infusion header file (.dih)
	 * @param infusion the Infusion to output
	 * @throws InfuserException 
	 */
	private void createInfusionHeaderFile(InternalInfusion infusion) throws InfuserException
	{
		String outFile = infuserArguments.getHeaderOutputFile();
		if (outFile!=null)
		{
			Logging.instance.println("Writing infusion header: " + outFile);
			try {
				FileOutputStream outStream = new FileOutputStream(outFile);
				DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
				Document doc = builder.newDocument();

				infusion.accept(new HeaderVisitor(doc));

				// Write it out again
				Transformer transformer = TransformerFactory.newInstance().newTransformer();
				transformer.setOutputProperty(OutputKeys.INDENT, "yes");
				transformer.setOutputProperty("{http://xml.apache.org/xslt}" + "indent-amount", "2");
				Source input = new DOMSource(doc);
				Result output = new StreamResult(outStream);
				transformer.transform(input, output);
				
				outStream.close();

			} catch (IOException ex)
			{
				throw new InfuserException("IO Error writing infusion file", ex);
			} catch (TransformerConfigurationException e)
			{
				throw new InfuserException("Error writing infusion file", e);
			} catch (ParserConfigurationException e)
			{
				throw new InfuserException("Error writing infusion file", e);
			} catch (TransformerException e)
			{
				throw new InfuserException("Error writing infusion file", e);
			} 
		}
	}
	
	/**
	 * Creates an infusion definitions file (.h)
	 * @param infusion the Infusion to output
	 * @throws InfuserException 
	 */
	private void createDefinitionFile(InternalInfusion infusion) throws InfuserException
	{
		String outFile = infuserArguments.getDefinitionOutputFile();
		if (outFile!=null)
		{
			Logging.instance.println("Writing C definitions header: " + outFile);
			try {
				FileOutputStream fout = new FileOutputStream(outFile);
				PrintWriter writer = new PrintWriter(fout);
				infusion.accept(new CHeaderVisitor(writer));
				writer.close();
				fout.close();
			} catch (IOException ex)
			{
				throw new InfuserException("IO Error writing infusion file", ex);
			}
		}
	}
	
	/**
	 * Creates an infusion native definitions file (.h)
	 * @param infusion the Infusion to output
	 * @throws InfuserException 
	 */
	private void createNativeFile(InternalInfusion infusion) throws InfuserException
	{
		// create native definitions file
		String outFile = infuserArguments.getNativeOutputFile();
		if (outFile != null)
		{
			Logging.instance.println("Writing native definitions header: " + outFile);
			try
			{
				FileOutputStream fout = new FileOutputStream(outFile);
				PrintWriter writer = new PrintWriter(fout);
				infusion.accept(new CFileVisitor(writer));
				writer.close();
				fout.close();
			} catch (IOException ex)
			{
				throw new InfuserException("IO Error writing infusion file", ex);
			}
		}
	}
	
	
	/**
	 * Creates an debug file (.debug)
	 * @param infusion the Infusion to output
	 * @throws InfuserException 
	 */
	private void createDebugFile(InternalInfusion infusion) throws InfuserException
	{
		// create native definitions file
		String outFile = infuserArguments.getDebugOutputFile();
		if (outFile != null)
		{
			Logging.instance.println("Writing debug: " + outFile);
			try
			{
				FileOutputStream fout = new FileOutputStream(outFile);
				PrintWriter writer = new PrintWriter(fout);
				infusion.accept(new DebugVisitor(writer));
				writer.close();
				fout.close();
			} catch (IOException ex)
			{
				throw new InfuserException("IO Error writing infusion file", ex);
			}
		}
	}
	
	/**
	 * Prepares the Infusion for processing. 
	 * @param infusion
	 * @return
	 */
	private void prepare(InternalInfusion infusion) throws InfuserException
	{
		Logging.instance.reset();
		
		// Check that the source version of the input class is 1.6
		infusion.accept(new JavaClassCheckVisitor());
		
		// Check if all the instructions in the input files are implemented by the VM.
		infusion.accept(new InstructionsImplementedCheckVisitor());
		
		// Check if the number of methods and classes does not exceed the maximum.
		infusion.accept(new ListSizeLimitationsCheckVisitor());
		
		// Resolved super class links between AbstractClassDefinition elements.
		infusion.accept(new ClassResolveVisitor(infusion));
		
		if (Logging.instance.getErrors()>0)
		{
			// report errors and warnings
			Logging.instance.printResult();
			
			// throw exception to notify the caller that something went bonkers
			throw new InfuserException("Errors during preparation");
		}
	}
	
	/**
	 * Processes the infusion.  
	 * @param infusion
	 * @return
	 */
	private void process(InternalInfusion infusion) throws InfuserException
	{
		// fix included infusions
		infusion.accept(new HeaderResolutionVisitor(infusion));

		// mapping and resolution
		infusion.accept(new IndexVisitor(infusion));
		infusion.accept(new FieldMapVisitor());
		infusion.accept(new FindEntryPointVisitor(infusion.getHeader()));
		infusion.accept(new InterfaceListFlattenVisitor());
		infusion.accept(new ClassInitialiserResolutionVisitor(infusion));
		infusion.accept(new StringTableVisitor(infusion));
		
		// process bytecode
		infusion.accept(new CodeBlockVisitor(infusion));

		// some profiling output for the SenSys paper
		// infusion.accept(new StackSizeVisitor());
	}
	
	/**
	 * Process the input and produce output :-)
	 * @throws InfuserException
	 */
	public void process() throws InfuserException
	{
		// check if the -name option was given
		if (infuserArguments.getInfusionName()==null)
			throw new InfuserException("error: infusion name not provided");

		// check for dependencies
		if (infuserArguments.isUpToDate())
		{
			Logging.instance.println("Infusion is up to date");
			return;
		}
		
		// Logging.instance.addVerbose(VerboseOutputType.ARGUMENTS_PARSING);
		
		// create an infusion
		InternalInfusion infusion = infuserArguments.createInfusion();

		// prepare the infusion for processing
		prepare(infusion);
		
		// process the infusion
		process(infusion);
			
		// create an infusion file (optional)
		createInfusionFile(infusion);
		
		// create infusion header file (optional)
		createInfusionHeaderFile(infusion);

		// create definition file
		createDefinitionFile(infusion);
		
		// create native file
		createNativeFile(infusion);

		// debug
		createDebugFile(infusion);
	}


}
