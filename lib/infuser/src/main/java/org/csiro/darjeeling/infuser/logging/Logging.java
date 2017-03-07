/*
 * Logging.java
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
 
package org.csiro.darjeeling.infuser.logging;

import java.io.PrintStream;
import java.util.HashSet;
import java.util.Set;

/**
 * 
 * Logging class. I know I know, singletons are bad, m'kay. But for typical cross-cutting concerns like this I think the benefits of cleaner code
 * outweigh the drawbacks.
 * 
 * You can set the 'current file name' in your visitor code before going into processing stuff so that you don't have to get that information every time
 * you're reporting errors or warnings.  
 * 
 * @author Niels Brouwers
 *
 */
public enum Logging
{
	
	// thread-safe singleton  
	instance;
	
	public enum VerboseOutputType
	{
		GENERAL,
		ARGUMENTS_PARSING,
		BYTECODE_PROCESSING,
	};
	
	private String currentFileName;

	// output is directed to these PrintWriter instances. Default for errors and warnings is System.err, 
	// default println goes to System.out (for notifications like usage, etc) 
	private PrintStream err = System.err;
	private PrintStream out = System.out;
	
	private Set<VerboseOutputType> verbose = new HashSet<VerboseOutputType>();
	
	private int warnings = 0, errors = 0;
	
	/**
	 * Resets the warning- and error count.
	 */
	public void reset()
	{
		warnings = errors = 0;
	}
	
	/**
	 * @return the number of reported warnings.
	 */
	public int getWarnings()
	{
		return warnings;
	}
	
	/**
	 * @return the number of reported errors.
	 */
	public int getErrors()
	{
		return errors;
	}

	/**
	 * Turns on verbose output of a given type
	 * @param type the type of verbose output to enable
	 */
	public void addVerbose(VerboseOutputType type)
	{
		this.verbose.add(type);
	}
	
	/**
	 * Turns off verbose output of a given type
	 * @param type the type of verbose output to turn off
	 */
	public void removeVerbose(VerboseOutputType type)
	{
		this.verbose.remove(type);
	}
	
	/**
	 * Sets the current file name the infuser is processing. Set to null to clear.
	 * @param currentFileName file that is currently being processed.
	 */
	public void setCurrentFileName(String currentFileName)
	{
		this.currentFileName = currentFileName;
	}
	
	/**
	 * Set the PrintWriter to use for 'normal' output. Warning and error messages will be printed through the error output.
	 * @param out PrintWriter to use for output
	 */
	public void setStandardOutput(PrintStream out)
	{
		this.out = out;
	}

	/**
	 * Set the PrintWriter to use for printing warnings and errors output. Other messages will be printed through the standard output.
	 * @param err PrintWriter to use for printing warnings and errors
	 */
	public void setErrorOutput(PrintStream err)
	{
		this.err = err;
	}
	
	private void report(String type, String message)
	{
		err.println(String.format("%s: %s", type, message));
	}
	
	private void report(String type, String fileName, String message)
	{
		err.println(String.format("%s: %s: %s", type, fileName, message));
	}
	
	private void report(String type, String fileName, int lineNumber, String message)
	{
		err.println(String.format("%s: %s: %d: %s", type, fileName, lineNumber, message));
	}
	
	/**
	 * Prints an error message
	 * @param message error message
	 */
	public void error(String message)
	{
		if (currentFileName==null)
			report("error", message);
		else
			report("error", currentFileName, message);

		errors ++;
	}

	/**
	 * Prints an error message with a given line number. Throws an exception when the current file is not set. 
	 * @throws IllegalStateException
	 * @param message error message
	 */
	public void error(int lineNumber, String message)
	{
		if (currentFileName==null)
			throw new IllegalStateException("Cannot report error, line number given but file name not set");
		else
			report("error", currentFileName, lineNumber, message);
		
		errors ++;
	}
	
	/**
	 * Prints a warning message
	 * @param message warning message
	 */
	public void warning(String message)
	{
		if (currentFileName==null)
			report("warning", message);
		else
			report("warning", currentFileName, message);

		warnings ++;
	}

	/**
	 * Prints a warning message with a given line number. Throws an exception when the current file is not set. 
	 * @throws IllegalStateException
	 * @param message warning message
	 */
	public void warning(int lineNumber, String message)
	{
		if (currentFileName==null)
			throw new IllegalStateException("Cannot report warning, line number given but file name not set");
		else
			report("warning", currentFileName, lineNumber, message);
		
		warnings ++;
	}
	
	public void println(String message)
	{
		out.println(message);
	}
	
	public void printlnVerbose(VerboseOutputType type, String message)
	{
		if (verbose.contains(type))
			out.println(message);
	}
	
	public void printResult()
	{
		out.println(String.format("%d warnings, %d errors", warnings, errors));
	}
	
}
