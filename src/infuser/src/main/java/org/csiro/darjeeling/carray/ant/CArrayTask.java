/*
 * CArrayTask.java
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

package org.csiro.darjeeling.carray.ant;

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
public class CArrayTask extends Task
{
	// Number of output values per line
	private final static int LINESIZE = 16;
	
	// Source, destination files and output array name.
	private String src, dest, arrayName;
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
		byte[] bytes;
		
		// make sure properties are set
		if (src==null) throw new BuildException("Source file name not set");
		if (dest==null) throw new BuildException("Destination file name not set");

        // Check if the file needs to be regenerated
		long srcDate =new File(src).lastModified();
        long destDate=new File(dest).lastModified();
        if(destDate > srcDate)
        {
            log("This file is up to date",Project.MSG_VERBOSE);
            return ;
        }
        
		// open input file
		try {
			FileInputStream fileInput = new FileInputStream(src);
			bytes = new byte[fileInput.available()];
			fileInput.read(bytes);
			fileInput.close();
		} catch (FileNotFoundException fnfex)
		{
			throw new org.apache.tools.ant.BuildException("File not found: " + src);
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while reading: " + src);
		}

		if (arraysize!=0 && bytes.length>arraysize)
			throw new BuildException("Archive doesn't fit in specified array size: " + arraysize);
		log("Converting "+src+" to "+dest+", "+bytes.length+" bytes",Project.MSG_INFO);
		
		// write C-style array definition
		try {
			FileOutputStream fout = new FileOutputStream(dest);
			PrintWriter writer = new PrintWriter(fout);
			writeArray(writer, bytes, arraysize);
			writer.flush();
			writer.close();
			fout.close();
		} catch (IOException ioex) {
			throw new org.apache.tools.ant.BuildException("IO error while writing: " + src);
		}
		
	}
	
	/*
	 * Does the actual writing.
	 */
	private void writeArray(PrintWriter out, byte[] bytes, int arraysize)
	{
		// If the array name is not set, use the source file name instead.
		String name = arrayName;
		boolean targetIsAvr = false, pageAlign = false;
		if (name==null) name = src;

		// If the avr-specific PROGMEM keyword is used, include the required pgmspace header file.
		for (String keyword : keywords) {
			if ("AVR".equals(keyword)) {
				targetIsAvr = true;
			}
			if ("PAGEALIGN".equals(keyword)) {
				pageAlign = true;
			}
		}

		if (targetIsAvr) {
			// avr-gcc arrays are limited to 32KB, so use inline assembly instead
			out.printf("#include <avr/pgmspace.h>\n");
			out.printf("\n");
			out.printf("__asm__(\n");
			out.printf("\".global %s_data \\n\\r\"\n", name);
			out.printf("\"        .section        .progmem.data,\\\"a\\\",@progbits \\n\\r\"\n");
			if (pageAlign) {
				out.printf("\"        .p2align        8 \\n\\r\"\n");
			}
			out.printf("\"        .type   %s_data, @object \\n\\r\"\n", name);
			out.printf("\"        .size   %s_data, %d \\n\\r\"\n", name, Math.max(arraysize, bytes.length));
			out.printf("\"%s_data: \\n\\r\"\n", name);
			for (int i=0; i<bytes.length; i++) {
				out.printf("\"        .byte   %d \\n\\r\"\n", bytes[i]);
			}
			out.printf(");\n");
		} else {
			// Create the keywords string.
			String keywordString = "";
			for (String keyword : keywords)
				keywordString += keyword + " ";

			// Print standard headers.
			out.printf("#include <stddef.h>\n");
			
			// Print the Array declaration.
			int length = bytes.length+4; // Add four bytes for archive size
			out.printf(
					"%sunsigned char %s%s_data[%d] = {\n", 
					constKeyword ? "const ":"", 
					keywordString,
					name,
					Math.max(arraysize, length)
			);

			// Print the actual data.
			int left = bytes.length;
			int pos = 0;
			out.printf("\t/* archive data */\n");
			while (left>0)
			{
				int lineLength = Math.min(left, LINESIZE);
				out.print("\t");
				for (int i=0; i<lineLength; i++)
				{
					out.printf("0x%02x, ", bytes[pos]);
					pos++;
				}
				out.print("\n");
				left-=lineLength;
			}

			// Close array.
			out.printf("};\n\n");			
		}		
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
	
	/**
	 * Sets the output array name.
	 * @param arrayName output array name
	 */
	public void setArrayname(String arrayName)
	{
		this.arrayName = arrayName;
	}
	
	/**
	 * Sets whether to omit the 'const' keyword. 
	 * @param value if true the 'const' keyword before the array definition will be omitted.
	 */
	public void setOmitConst(boolean value)
	{
		constKeyword = false;
	}
	
	/**
	 * Sets a space-separated list of keywords to use in the array declaration.
	 * @param value space-separated list of keywords
	 */
	public void setKeywords(String value)
	{
		Collections.addAll(keywords, value.split(" "));
	}

	/**
	 * Sets the desired size of the array. Defaults to the size of the archive if 0. Error if !=0 but < archive size.
	 * @param arraysize the desired size of the array
	 */
	public void setArraysize(int arraysize)
	{
		this.arraysize = arraysize;
	}

}
