/*
 * LibInitTask.java
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

package org.csiro.darjeeling.libinit.ant;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;

/**
 * 
 * Ant task that generates some C code to call <libname>_init for all included libraries and create the list
 * of native handlers for libraries that contain Java
 */
public class LibInitTask extends Task
{
	private String dest;
	private String libDir;
	private String libraries;

	/**
	 * Ant execute entry point.
	 */
	public void execute()
	{
		// make sure properties are set
		if (dest==null) throw new BuildException("Destination file name not set");
		if (libDir==null) throw new BuildException("Library directory not set");
		if (libraries==null) throw new BuildException("List of libraries not set");

		ArrayList<String> librariesArray = new ArrayList<String>();
		for (String library : libraries.replace(',', ' ').split(" ")) {
			if (library.trim().length() != 0) {
				librariesArray.add(library);
			}
		}

		ArrayList<String> javaLibrariesArray = new ArrayList<String>();
		for (String library : librariesArray) {
			File javaDir = new File(libDir + "/" + library + "/java");
			if (javaDir.exists())
				javaLibrariesArray.add(library);
		}

		try {
			PrintWriter fout = new PrintWriter(new FileWriter(dest));

			fout.println("#include \"types.h\"");
			for (String library : javaLibrariesArray) {
				fout.println("#include \"jlib_" + library + ".h\"");
			}
			fout.println();

			fout.println("dj_named_native_handler java_library_native_handlers[] = {");
			for (String library : javaLibrariesArray) {
				fout.println("\t{ \"" + library + "\", " + library + "_native_method_function_pointers },");
			}
			fout.println("};");
			fout.println("uint8_t java_library_native_handlers_length = " + javaLibrariesArray.size() + ";");
			fout.println();

			for (String library : librariesArray) {
				fout.println("extern void " + library + "_init();");
			}
			fout.println("void dj_libraries_init() {");
			for (String library : librariesArray) {
				fout.println("\t" + library + "_init();");
			}
			fout.println("}");

			fout.close();
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while writing: " + dest);
		}
		
	}

	/**
	 * Sets the destination file to generate the C code.
	 * @param dest destination file name
	 */
	public void setDest(String dest)
	{
		this.dest = dest;
	}

	/**
	 * Sets the directory where the libraries are found.
	 * @param libDir libraries directory
	 */
	public void setLibDir(String libDir)
	{
		this.libDir = libDir;
	}

	/**
	 * Sets the space or comma separated list of libraries to include
	 * @param libraries the list of libraries
	 */
	public void setLibraries(String libraries)
	{
		this.libraries = libraries;
	}
}
