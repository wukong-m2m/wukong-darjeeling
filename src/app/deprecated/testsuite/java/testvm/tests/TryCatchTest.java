/*
 * TryCatchTest.java
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
 
package testvm.tests;

import javax.darjeeling.actuators.Leds;

public class TryCatchTest
{
	public static void test(int testBase)
	{
		boolean state = true;
		while (true)
		{
			try
			{
				for (short i = 0; i < 3; i++)
				{
					Leds.set(i, state);
					Thread.sleep(1000);
				}
				state = !state;
			} catch (Throwable e)
			{
			}
			// state = state;
		}
	}
}
