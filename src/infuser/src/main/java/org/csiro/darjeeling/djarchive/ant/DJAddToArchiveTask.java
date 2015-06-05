/*
 * DJAddToArchiveTask.java
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

package org.csiro.darjeeling.djarchive.ant;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;

/**
 * 
 * Ant task that converts a binary file to a c-style constant array. It can add user-specified keywords such 
 * as 'PROGMEM' (required for AVR targets), or for putting the array in specific segments (i.e. FAR on msp430x). 
 * When the 'PROGMEM' keyword is used, the appropriate avr include file will be automatically included.
 * 
 * The resulting array will be declared constant using the 'const' keyword, which usually has an effect on where
 * the data will be placed by the linker. This can be omitted using 'omitconst=true' in the ant build file. 
 * 
 * @author Niels Brouwers
 *
 * NR 20130224: Replacing the size from a separate variable to the first 4 bytes of the C array.
 *              This will make wireless reprogramming easier since we can just upload a whole new
 *              array, and will free a little bit of memory as well.
 *              Also adding an extra parameter "arraysize" so we can easily reserve space for
 *              uploading larger archives (will only be useful for the app archive). When it's
 *              empty we'll create an array as large as the archive. When it's set, but smaller
 *              than the archive, throw an error.
 */
public class DJAddToArchiveTask extends Task
{
	private String dest;
	private byte filetype;
	private String src;
	private String mode;

	/**
	 * Ant execute entry point.
	 */
	public void execute()
	{
		// make sure properties are set
		if (dest==null) throw new BuildException("Destination file name not set");
		if (mode==null) throw new BuildException("Mode file name not set");

		try {
			byte[] oldBytes = {};

			if (mode.equals("append")) {
				// Read the data already in the file. There's probably a better way to do this in Java
				FileInputStream fileInput = new FileInputStream(dest);
				byte[] bytes = new byte[fileInput.available()];
				fileInput.read(bytes);
				fileInput.close();
				if (bytes[bytes.length-1] != 0
						|| bytes[bytes.length-2] != 0)
					throw new org.apache.tools.ant.BuildException(dest + "is not a valid DJ archive (it doesn't end in 0 0).");
				oldBytes = arrayPart(bytes, bytes.length-2); // Trim the last two bytes of the archive
			}

			FileOutputStream fout = new FileOutputStream(dest);
			if (mode.equals("append")) {
				System.out.println("Adding to archive '" + dest + "'");
				fout.write(oldBytes);
			} else {
				System.out.println("Creating archive '" + dest + "'");
			}

			appendFiles(fout, src, filetype);
			// Close the file with a 00 00
			fout.write((byte)0);
			fout.write((byte)0);
			fout.close();
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while writing: " + dest);
		}
		
	}

	private static byte[] arrayPart(byte[] array, int size) {
	    byte[] part = new byte[size];
	    System.arraycopy(array, 0, part, 0, size);
	    return part;
	}

	public void appendFiles(FileOutputStream fout, String files, byte filetype) throws IOException {
		if (files == null)
			return;
		String[] filenames = files.split(" ");
		for (String filename: filenames) {
			if (filename == null || filename.isEmpty() || filename.trim().isEmpty())
				continue;
			System.out.println("Adding '" + filename + "'");
			byte[] bytes;
			// open input file
			try {
				FileInputStream fileInput = new FileInputStream(filename);
				bytes = new byte[fileInput.available()];
				fileInput.read(bytes);
				fileInput.close();
			} catch (FileNotFoundException fnfex)
			{
				throw new org.apache.tools.ant.BuildException("File not found: " + filename);
			} catch (IOException ioex) {
				throw new org.apache.tools.ant.BuildException("IO error while reading: " + filename);
			}
			// First write the length of each file
			fout.write((byte)((bytes.length>>0)%256));
			fout.write((byte)((bytes.length>>8)%256));
			// Then write the file type
			fout.write(filetype);
			// Then write the file
			fout.write(bytes);
		}
	}


	/**
	 * Sets the list of file add to the archive.
	 * @param libInfusions list of library infusions
	 */
	public void setSrc(String src)
	{
		this.src = src;
	}

	/**
	 * Sets the filetype for the files to be added to the archive
	 * @param appInfusions list of application infusions
	 */
	public void setFiletype(String filetype)
	{
		this.filetype = (byte)Byte.parseByte(filetype);
	}

	/**
	 * Sets the destination archive to write to.
	 * @param dest destination file name
	 */
	public void setDest(String dest)
	{
		this.dest = dest;
	}

	/**
	 * "append" to append to existing DJ archive, or "create" to overwrite it if it exists
	 * @param mode either "append" or "create"
	 */
	public void setMode(String mode)
	{
		if (!mode.equals("append") && !mode.equals("create"))
			throw new org.apache.tools.ant.BuildException("DJAddToArchiveTask mode should be either 'append' or 'create', not " + mode);
		this.mode = mode;
	}
}
