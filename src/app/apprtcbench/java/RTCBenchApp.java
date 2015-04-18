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

	public static void perftestBubbleSort() {
		// System.out.println("java:");
		// RTC.useRTC(false);
		// RTCTestBubbleSort.test_bubblesort();
		RTC.useRTC(true);
		// Stopwatch.resetAndStart();
		// Stopwatch.measure();
		System.out.println("native sort:");
		RTCTestBubbleSort.test_bubblesort_native();
		System.out.println("rtc sort:");
		RTCTestBubbleSort.test_bubblesort();
		System.out.println("native fft:");
		RTCTestFixFFT.test_fixfft_native();
		System.out.println("rtc fft:");
		RTCTestFixFFT.test_fixfft();
		System.out.println("done.");
	}
}
