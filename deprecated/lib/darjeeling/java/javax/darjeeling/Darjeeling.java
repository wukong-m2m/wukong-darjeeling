/*
 * Darjeeling.java
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
 
package javax.darjeeling ;

public class Darjeeling
{
	
	public static native void assertTrue(int testNr, boolean value);

	public static native void gc();
	
	public static void print(String str) {
		printBytesAsString(str.toZeroTerminatedByteArray());
	}
	public static native void print(int i);

	public static void print(byte[] array){
		for (int i = 0; i < array.length; i ++)
			print((int)(0xFF & array[i]));
	}
	public static native int getTime();
	public static native int getMemFree();
	public static native int getNodeId();
	public static native int getBoardTemperature();
	public static native int getVoltage();
	
	public static native int getSecond();
	public static native int getMinute();
	public static native int getHour();

	public static native int getBatteryVoltage();
	public static native int getSolarVoltage();
	public static native int getSolarCurrent();
	public static native int getADC(short chan);

	public static native int random();
	
	public static native short getNrThreads();
	public static native Thread getThread(short nr);
	//public static native void setLed();
    
    public static native short getEcho(short chan);
    public static native short getHumidity();
    public static native short getTemperature();
    public static native int getPulseCounter();

    public static native void setExpansionPower(short on);
    
    private static native void printBytesAsString(byte[] str);
}
