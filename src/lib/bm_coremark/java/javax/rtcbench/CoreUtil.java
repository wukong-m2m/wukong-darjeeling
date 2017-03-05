package javax.rtcbench;
import javax.rtc.RTC;

/*
Author : Shay Gal-On, EEMBC

This file is part of  EEMBC(R) and CoreMark(TM), which are Copyright (C) 2009 
All rights reserved.                            

EEMBC CoreMark Software is a product of EEMBC and is provided under the terms of the
CoreMark License that is distributed with the official EEMBC COREMARK Software release. 
If you received this EEMBC CoreMark Software without the accompanying CoreMark License, 
you must discontinue use and download the official release from www.coremark.org.  

Also, if you are publicly displaying scores generated from the EEMBC CoreMark software, 
make sure that you are in compliance with Run and Reporting rules specified in the accompanying readme.txt file.

EEMBC 
4354 Town Center Blvd. Suite 114-200
El Dorado Hills, CA, 95762 
*/ 

/* Function: get_seed
	Get a values that cannot be determined at compile time.

	Since different embedded systems and compilers are used, 3 different methods are provided:
	1 - Using a volatile variable. This method is only valid if the compiler is forced to generate code that
	reads the value of a volatile variable from memory at run time. 
	Please note, if using this method, you would need to modify core_portme.c to generate training profile.
	2 - Command line arguments. This is the preferred method if command line arguments are supported.
	3 - System function. If none of the first 2 methods is available on the platform,
	a system function which is not a stub can be used.
	
	e.g. read the value on GPIO pins connected to switches, or invoke special simulator functions.
*/

public class CoreUtil {
	public static int seed1_volatile;
	public static int seed2_volatile;
	public static int seed3_volatile;
	public static int seed4_volatile;
	public static int seed5_volatile;

	public static int get_seed_32(int i) {
		int retval;
		switch (i) {
			case 1:
				retval=seed1_volatile;
				break;
			case 2:
				retval=seed2_volatile;
				break;
			case 3:
				retval=seed3_volatile;
				break;
			case 4:
				retval=seed4_volatile;
				break;
			case 5:
				retval=seed5_volatile;
				break;
			default:
				retval=0;
				break;
		}
		return retval;
	}

	/* Function: crc*
		Service functions to calculate 16b CRC code.

	*/
	static short crcu8(byte data, short crc )
	{
		byte i=0,x16=0,carry=0;

		for (i = 0; i < 8; i++)
	    {
			x16 = (byte)((data & 1) ^ ((byte)crc & 1));
			data >>>= 1;

			if (x16 == 1)
			{
			   crc ^= 0x4002;
			   carry = 1;
			}
			else 
				carry = 0;
			crc >>>= 1;
			if (carry != 0)
			   crc |= 0x8000;
			else
			   crc &= 0x7fff;
	    }
		return crc;
	} 
	static short crcu16(short newval, short crc) {
		crc=crcu8((byte) (newval), crc);
		crc=crcu8((byte) ((newval)>>>8), crc);
		return crc;
	}
	static short crcu32(int newval, short crc) {
		crc=crc16((short) newval, crc);
		crc=crc16((short) (newval>>>16), crc);
		return crc;
	}
	static short crc16(short newval, short crc) {
		return crcu16((short)newval, crc);
	}

	// ee_u8 check_data_types() {
	// 	ee_u8 retval=0;
	// 	if (sizeof(ee_u8) != 1) {
	// 		ee_printf("ERROR: ee_u8 is not an 8b datatype!\n");
	// 		retval++;
	// 	}
	// 	if (sizeof(ee_u16) != 2) {
	// 		ee_printf("ERROR: ee_u16 is not a 16b datatype!\n");
	// 		retval++;
	// 	}
	// 	if (sizeof(ee_s16) != 2) {
	// 		ee_printf("ERROR: ee_s16 is not a 16b datatype!\n");
	// 		retval++;
	// 	}
	// 	if (sizeof(ee_s32) != 4) {
	// 		ee_printf("ERROR: ee_s32 is not a 32b datatype!\n");
	// 		retval++;
	// 	}
	// 	if (sizeof(ee_u32) != 4) {
	// 		ee_printf("ERROR: ee_u32 is not a 32b datatype!\n");
	// 		retval++;
	// 	}
	// 	if (sizeof(ee_ptr_int) != sizeof(int *)) {
	// 		ee_printf("ERROR: ee_ptr_int is not a datatype that holds an int pointer!\n");
	// 		retval++;
	// 	}
	// 	if (retval>0) {
	// 		ee_printf("ERROR: Please modify the datatypes in core_portme.h!\n");
	// 	}
	// 	return retval;
	// }


}