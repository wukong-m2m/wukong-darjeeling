/*
 * HelloWorld.java
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

import javax.rtc.RTCTest;

public class HelloWorld
{
	public static void main(String args[])
	{
		System.out.println("Hello, world! LT" + RTCTest.f_LT((short)-1));
		System.out.println("Hello, world! LT" + RTCTest.f_LT((short)0));
		System.out.println("Hello, world! LT" + RTCTest.f_LT((short)1));

		System.out.println("Hello, world! LE" + RTCTest.f_LE((short)-1));
		System.out.println("Hello, world! LE" + RTCTest.f_LE((short)0));
		System.out.println("Hello, world! LE" + RTCTest.f_LE((short)1));

		System.out.println("Hello, world! GT" + RTCTest.f_GT((short)-1));
		System.out.println("Hello, world! GT" + RTCTest.f_GT((short)0));
		System.out.println("Hello, world! GT" + RTCTest.f_GT((short)1));

		System.out.println("Hello, world! GE" + RTCTest.f_GE((short)-1));
		System.out.println("Hello, world! GE" + RTCTest.f_GE((short)0));
		System.out.println("Hello, world! GE" + RTCTest.f_GE((short)1));
	}
}
