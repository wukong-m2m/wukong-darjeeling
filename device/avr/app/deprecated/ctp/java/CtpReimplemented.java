/*
 * CtpReimplemented.java
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
 
import javax.darjeeling.Darjeeling;
import javax.radio.Radio;

import org.csiro.ctp.Ctp;
import org.csiro.ctp.CtpDataConsumer;
import org.csiro.ctp.CtpDataProvider;
import org.csiro.debug.Debug;

public class CtpReimplemented
{

	private static Ctp ctp;
	private static short seq = 0;
	
	private static class ExampleConsumer implements CtpDataConsumer
	{
		public void dataReceived(short[] data, short origin, short timeHasLived)
		{
			Darjeeling.print("DATA RECEIVED FROM ID: " + origin +
					" THL: " + timeHasLived + " DATA: ");
			
				for (int i = 0; i < data.length; i ++)
					Darjeeling.print(data[i] + ".");
		}
	}

	private static class ExampleProvider implements CtpDataProvider
	{
		public short[] getData()
		{
			Darjeeling.gc();
			return new short[] 
			{
					(short)Darjeeling.getMemFree(),
					seq++,
					ctp.getRoutingEngine().getParent()
			};
		}
	}
	
	public static void main(String args[])
    {
    	Radio.init();

    	// wait for radio to be initialised (hack for TNodes)
    	Debug.setDisplayOutput(false);
    	Debug.print("Main thread is : " + Thread.getCurrentThreadId());
    	Thread.sleep(1000);
    	
    	// Debug.setDisplayOutput(Darjeeling.getNodeId()==1);

    	// create a new CTP object
    	//TODO: please check if getNodeId is implemented in your desired port,
    	//if it is not implemented it returns BASE_CDEF_java_lang_VirtualMachineError
    	if (Darjeeling.getNodeId()==1)
    	{
    		// create sink
        	ctp = new Ctp(new ExampleConsumer());
        	ctp.start();
    	} else
    	{
    		// create leaf
        	ctp = new Ctp(new ExampleProvider());
        	ctp.start();
    	}
    	
    }
	
}

