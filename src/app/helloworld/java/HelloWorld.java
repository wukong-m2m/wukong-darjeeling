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
		// checkCompareShort0();
		checkStaticShortVariables();
		System.out.println("Doet hij het?\n");
	}

	public static void checkStaticShortVariables()
	{
		System.out.println("test static short -1 ->" + RTCTest.test_static_short((short)-1));
		System.out.println("test static short 42 ->" + RTCTest.test_static_short((short)42));
		System.out.println("test static short  3 ->" + RTCTest.test_static_short((short)3));
	}

	public static void checkCompareShort0()
	{
		System.out.println("-1 EQ" + RTCTest.compare_short_EQ((short)-1));
		System.out.println(" 0 EQ" + RTCTest.compare_short_EQ((short)0));
		System.out.println(" 1 EQ" + RTCTest.compare_short_EQ((short)1));

		System.out.println("-1 NE" + RTCTest.compare_short_NE((short)-1));
		System.out.println(" 0 NE" + RTCTest.compare_short_NE((short)0));
		System.out.println(" 1 NE" + RTCTest.compare_short_NE((short)1));

		System.out.println("-1 LT" + RTCTest.compare_short_LT((short)-1));
		System.out.println(" 0 LT" + RTCTest.compare_short_LT((short)0));
		System.out.println(" 1 LT" + RTCTest.compare_short_LT((short)1));

		System.out.println("-1 LE" + RTCTest.compare_short_LE((short)-1));
		System.out.println(" 0 LE" + RTCTest.compare_short_LE((short)0));
		System.out.println(" 1 LE" + RTCTest.compare_short_LE((short)1));

		System.out.println("-1 GT" + RTCTest.compare_short_GT((short)-1));
		System.out.println(" 0 GT" + RTCTest.compare_short_GT((short)0));
		System.out.println(" 1 GT" + RTCTest.compare_short_GT((short)1));

		System.out.println("-1 GE" + RTCTest.compare_short_GE((short)-1));
		System.out.println(" 0 GE" + RTCTest.compare_short_GE((short)0));
		System.out.println(" 1 GE" + RTCTest.compare_short_GE((short)1));
	}
}
