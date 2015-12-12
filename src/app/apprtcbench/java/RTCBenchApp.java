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

import javax.rtc.*;
import javax.rtcbench.*;
import javax.darjeeling.Stopwatch;

public class RTCBenchApp
{
	public static void main(String args[])
	{
		perftestBubbleSort();
		RTC.avroraBreak();
	}

	public static void nop() {}

	public static void perftestBubbleSort() {
		System.out.println("START BENCHMARK: " + RTCBenchmark.name);

		System.out.println("Java stopwatch overhead");
		Stopwatch.resetAndStart();
		Stopwatch.measure();

		System.out.println("Java function call overhead");
		Stopwatch.resetAndStart();
		nop();
		Stopwatch.measure();

		Stopwatch.setTimerNumber((byte)101);
		System.out.println("native:");
		RTCBenchmark.test_native();
		System.out.println("native done.\r\n");

		Stopwatch.setTimerNumber((byte)102);
		System.out.println("rtc:");
		RTC.useRTC(true);
		if (RTCBenchmark.test_java()) {
			System.out.println("RTC OK.\r\n");
		} else {
			System.out.println("RTC FAILED.\r\n");			
		}


		Stopwatch.setTimerNumber((byte)103);
		System.out.println("java:");
		RTC.useRTC(false);
		if (RTCBenchmark.test_java()) {
			System.out.println("JAVA OK.\r\n");
		} else {
			System.out.println("JAVA FAILED.\r\n");			
		}

		System.out.println("FINISHED BENCHMARK: " + RTCBenchmark.name);
	}
}
