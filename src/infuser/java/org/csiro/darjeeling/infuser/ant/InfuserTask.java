/*
 * InfuserTask.java
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
 
package org.csiro.darjeeling.infuser.ant;

import java.util.ArrayList;
import java.util.List;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.FileList;
import org.apache.tools.ant.types.FileSet;
import org.csiro.darjeeling.infuser.Infuser;
import org.csiro.darjeeling.infuser.InfuserArguments;
import org.csiro.darjeeling.infuser.InfuserException;

/**
 * 
 * Apache Ant interface for the infuser tool.
 * 
 * @author Niels Brouwers
 *
 */
public class InfuserTask extends Task
{
	
	// Input file sets and lists (because it's somehow better to have both?)
	private List<FileSet> fileSets;
	private List<FileList> fileLists;
	
	// arguments for the infuser
	protected InfuserArguments infuserArguments;
	
	/**
	 * Creates a new Infuser Ant task
	 */
	public InfuserTask()
	{
		fileSets = new ArrayList<FileSet>();
		fileLists = new ArrayList<FileList>();
		infuserArguments = new InfuserArguments();
	}
	
	// iterates over all the input file names and adds them to the infuser arguments
	// object
	protected void iterateResourceCollections()
	{
		for (FileSet fileSet : fileSets)
		{
			String files[] = fileSet.getDirectoryScanner(getProject()).getIncludedFiles();
			for (String file : files)
			{
				String path = fileSet.getDir(getProject()) + "/" + file;
				try {
					infuserArguments.parseArgument(path);
				}
                catch (Exception ex)
				{
					throw new BuildException("Error loading file: " + path, ex);
				}
			}
		}
		for (FileList fileList : fileLists)
		{
			for (String file : fileList.getFiles(getProject()))
			{
				String path = fileList.getDir(getProject()) + "/" + file;
				try {
					infuserArguments.parseArgument(path);
				}
                catch (Exception ex)
				{
					throw new BuildException("Error loading file: " + path, ex);
				}
			}
		}
	}
	
	public void execute()
	{
		// print greeting
		log(Infuser.greeting, Project.MSG_VERBOSE);
		
		// load files into the infuserArguments object 
		iterateResourceCollections();
		
		// create Infuser and process
		Infuser infuser = new Infuser(infuserArguments);
		try {
			infuser.process();
		} catch (InfuserException ex)
		{
			throw new BuildException(ex.getMessage(), ex);
		} catch (Exception e)
		{
			throw new BuildException(e);
		}

	}
	
	public void setName(String name)
	{
		infuserArguments.setInfusionName(name);
	}
	
	public void setInfusionFile(String name)
	{
		infuserArguments.setInfusionOutputFile(name);
	}
	
	public void setHeaderFile(String name)
	{	
		infuserArguments.setHeaderOutputFile(name);
	}
	
	public void setHFile(String name)
	{
		infuserArguments.setDefinitionOutputFile(name);
	}

	public void setCFile(String name)
	{
		infuserArguments.setNativeOutputFile(name);
	}
	
	public void setInfusionVersion(int value)
	{
		infuserArguments.setInfusionVersion(value);
	}
    
	public void addFileset(FileSet fileset)
	{
		fileSets.add(fileset);
    }
	
	public void addFilelist(FileList filelist)
	{
		fileLists.add(filelist);
    }
	
	public void setDebugFile(String debug)
	{
		infuserArguments.setDebugOutputFile(debug);
	}
	
}
