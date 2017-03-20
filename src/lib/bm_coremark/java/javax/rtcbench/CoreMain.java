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
/* File: core_main.c
	This file contains the framework to acquire a block of memory, seed initial parameters, tun t he benchmark and report the results.
*/

/* Function: iterate
	Run the benchmark for a specified number of iterations.

	Operation:
	For each type of benchmarked algorithm:
		a - Initialize the data block for the algorithm.
		b - Execute the algorithm N times.

	Returns:
	NULL.
*/

public class CoreMain {
	private static short list_known_crc[]   = {(short)0xd4b0,(short)0x3340,(short)0x6a79,(short)0xe714,(short)0xe3c1};
	private static short matrix_known_crc[] = {(short)0xbe52,(short)0x1199,(short)0x5608,(short)0x1fd7,(short)0x0747};
	private static short state_known_crc[]  = {(short)0x5e47,(short)0x39bf,(short)0xe5a4,(short)0x8e3a,(short)0x8d84};

	private static void iterate(CoreResults pres) {
		int i;
		short crc;
		CoreResults res=pres;
		int iterations=res.iterations;
		res.crc=0;
		res.crclist=0;
		res.crcmatrix=0;
		res.crcstate=0;

		for (i=0; i<iterations; i++) {
			crc=CoreListJoinB.core_bench_list(res,(short)1);
			res.crc=CoreUtil.crcu16(crc,res.crc);
			crc=CoreListJoinB.core_bench_list(res,(short)-1);
			res.crc=CoreUtil.crcu16(crc,res.crc);
			if (i==0) res.crclist=res.crc;
		}
		return;
	}

	/* Function: main
		Main entry routine for the benchmark.
		This function is responsible for the following steps:

		1 - Initialize input seeds from a source that cannot be determined at compile time.
		2 - Initialize memory block for use.
		3 - Run and time the benchmark.
		4 - Report results, testing the validity of the output if the seeds are known.

		Arguments:
		1 - first seed  : Any value
		2 - second seed : Must be identical to first for iterations to be identical
		3 - third seed  : Any value, should be at least an order of magnitude less then the input size, but bigger then 32.
		4 - Iterations  : Special, if set to 0, iterations will be automatically determined such that the benchmark will run between 10 to 100 secs
	*/

	private static short MULTITHREAD=1; // Single thread

	private static int get_seed_32(int i) {
		return CoreUtil.get_seed_32(i);
	}
	private static short get_seed(int x) {
		return (short)CoreUtil.get_seed_32(x);
	}

	public static void core_mark_main() {
		// int argc=0;
		// char *argv[1];
		short i,j=0,num_algorithms=0;
		short known_id=-1,total_errors=0;
		short seedcrc=0;
		int total_time;
		CoreResults[] results = new CoreResults[MULTITHREAD];

		for (i=0 ; i<MULTITHREAD; i++) {
			results[i] = new CoreResults();
		}

		// /* first call any initializations needed */
		// portable_init(&(results[0].port), &argc, argv);

		// /* First some checks to make sure benchmark will run ok */
		// if (sizeof(struct list_head_s)>128) {
		// 	ee_printf("list_head structure too big for comparable data!\n");
		// 	return MAIN_RETURN_VAL;
		// }

		results[0].seed1=get_seed(1);
		results[0].seed2=get_seed(2);
		results[0].seed3=get_seed(3);
		results[0].iterations=get_seed_32(4);

		results[0].execs=get_seed_32(5);
		if (results[0].execs==0) { /* if not supplied, execute all algorithms */
			results[0].execs=CoreMarkH.ALL_ALGORITHMS_MASK;
		}
		/* put in some default values based on one seed only for easy testing */
		if ((results[0].seed1==0) && (results[0].seed2==0) && (results[0].seed3==0)) { /* validation run */
			results[0].seed1=0;
			results[0].seed2=0;
			results[0].seed3=0x66;
		}
		if ((results[0].seed1==1) && (results[0].seed2==0) && (results[0].seed3==0)) { /* perfromance run */
			results[0].seed1=0x3415;
			results[0].seed2=0x3415;
			results[0].seed3=0x66;
		}

		// results[0].memblock[0]=(void *)static_memblk;
		results[0].size=CoreMarkH.TOTAL_DATA_SIZE;
		results[0].err=0;

		/* Data init */ 
		/* Find out how space much we have based on number of algorithms */
		for (i=0; i<CoreMarkH.NUM_ALGORITHMS; i++) {
			if (((1<<i) & results[0].execs) != 0)
				num_algorithms++;
		}
		for (i=0 ; i<MULTITHREAD; i++) 
			results[i].size=results[i].size/num_algorithms;
		// /* Assign pointers */
		// Skip this because the Java version will allocate memory in the _init functions
		// for (i=0; i<NUM_ALGORITHMS; i++) {
		// 	ee_u32 ctx;
		// 	if ((1<<(ee_u32)i) & results[0].execs) {
		// 		for (ctx=0 ; ctx<MULTITHREAD; ctx++)
		// 			results[ctx].memblock[i+1]=(char *)(results[ctx].memblock[0])+results[0].size*j;
		// 		j++;
		// 	}
		// }
		/* call inits */
		for (i=0 ; i<MULTITHREAD; i++) {
			if ((results[i].execs & CoreMarkH.ID_LIST) != 0) {
				results[i].list_CoreListJoinB=CoreListJoinB.core_list_init(results[0].size,results[i].seed1);
			}
			if ((results[i].execs & CoreMarkH.ID_MATRIX) != 0) {
				results[i].mat = new CoreMatrix.MatParams();
				CoreMatrix.core_init_matrix((short)results[0].size, results[i].seed1 | ((results[i].seed2) << 16), results[i].mat );
			}
			if ((results[i].execs & CoreMarkH.ID_STATE) != 0) {
				results[i].statememblock3 = CoreState.core_init_state(results[0].size,results[i].seed1);
			}
		}
		
		/* automatically determine number of iterations if not set */
		if (results[0].iterations==0) { 
			int secs_passed=0;
			int divisor;
			results[0].iterations=1;
			while (secs_passed < 1) {
				results[0].iterations*=10;
				RTC.coremark_start_time();
				iterate(results[0]);
				RTC.coremark_stop_time();
				secs_passed=CorePortMe.time_in_secs(RTC.coremark_get_time());
			}
			/* now we know it executes for at least 1 sec, set actual run time at about 10 secs */
			divisor=secs_passed;
			if (divisor==0) /* some machines cast float to int as 0 since this conversion is not defined by ANSI, but we know at least one second passed */
				divisor=1;
			results[0].iterations*=1+10/divisor;
		}

		// /* perform actual benchmark */
		RTC.avroraStartCountingCalls();
		RTC.coremark_start_time();
		iterate(results[0]);
		RTC.coremark_stop_time();
		total_time=RTC.coremark_get_time();
		RTC.avroraStopCountingCalls();

		// NOT STANDARD COREMARK CODE
		// Free up some memory here because otherwise we'll get
		// out of memory errors when printing the report. This is
		// not standard CoreMark code, but it should be ok here
		// since the benchmark is already done here.
		results[0].mat = null;
		results[0].statememblock3 = null;
		CoreListJoinB.data = null;

		/* get a function of the input to report */
		seedcrc=CoreUtil.crc16((short)results[0].seed1,seedcrc);
		seedcrc=CoreUtil.crc16((short)results[0].seed2,seedcrc);
		seedcrc=CoreUtil.crc16((short)results[0].seed3,seedcrc);
		seedcrc=CoreUtil.crc16((short)results[0].size,seedcrc);
		
		switch (seedcrc) { /* test known output for common seeds */
			case (short)0x8a02: /* seed1=0, seed2=0, seed3=0x66, size 2000 per algorithm */
				known_id=0;
				System.out.println("6k performance run parameters for coremark.");
				break;
			case (short)0x7b05: /*  seed1=0x3415, seed2=0x3415, seed3=0x66, size 2000 per algorithm */
				known_id=1;
				System.out.println("6k validation run parameters for coremark.");
				break;
			case (short)0x4eaf: /* seed1=0x8, seed2=0x8, seed3=0x8, size 400 per algorithm */
				known_id=2;
				System.out.println("Profile generation run parameters for coremark.");
				break;
			case (short)0xe9f5: /* seed1=0, seed2=0, seed3=0x66, size 666 per algorithm */
				known_id=3;
				System.out.println("2K performance run parameters for coremark.");
				break;
			case (short)0x18f2: /*  seed1=0x3415, seed2=0x3415, seed3=0x66, size 666 per algorithm */
				known_id=4;
				System.out.println("2K validation run parameters for coremark.");
				break;
			default:
				total_errors=-1;
				break;
		}
		if (known_id>=0) {
			for (i=0 ; i<CorePortMe.default_num_contexts; i++) {
				results[i].err=0;
				if ((results[i].execs & CoreMarkH.ID_LIST) != 0 && 
					(results[i].crclist!=list_known_crc[known_id])) {
					System.out.println("[" + i + "]ERROR! list crc 0x" + Integer.toHexString(results[i].crclist) + " - should be 0x" + Integer.toHexString(list_known_crc[known_id]));
					results[i].err++;
				}
				if ((results[i].execs & CoreMarkH.ID_MATRIX) != 0 &&
					(results[i].crcmatrix!=matrix_known_crc[known_id])) {
					System.out.println("[" + i + "]ERROR! matrix crc 0x" + Integer.toHexString(results[i].crcmatrix) + " - should be 0x" + Integer.toHexString(matrix_known_crc[known_id]));
					results[i].err++;
				}
				if ((results[i].execs & CoreMarkH.ID_STATE) != 0 &&
					(results[i].crcstate!=state_known_crc[known_id])) {
					System.out.println("[" + i + "]ERROR! state crc 0x" + Integer.toHexString(results[i].crcstate) + " - should be 0x" + Integer.toHexString(state_known_crc[known_id]));
					results[i].err++;
				}
				total_errors+=results[i].err;
			}
		}
		// This is pointless to test in Java
		// total_errors+=check_data_types();

		/* and report results */
		System.out.println("CoreMark Size    : " + results[0].size);
		System.out.println("Total ticks      : " + total_time);
		System.out.println("Total time (secs): " + CorePortMe.time_in_secs(total_time));
		if (CorePortMe.time_in_secs(total_time) > 0)
			System.out.println("Iterations/Sec   : " + CorePortMe.default_num_contexts*results[0].iterations/CorePortMe.time_in_secs(total_time));
		if (CorePortMe.time_in_secs(total_time) < 10) {
			System.out.println("ERROR! Must execute for at least 10 secs for a valid result!");
			total_errors++;
		}

		System.out.println("Iterations       : " + CorePortMe.default_num_contexts*results[0].iterations);
		// System.out.println("Compiler version : %s\n",COMPILER_VERSION);
		// System.out.println("Compiler flags   : %s\n",COMPILER_FLAGS);
		// System.out.println("Memory location  : %s\n",MEM_LOCATION);
		/* output for verification */
		System.out.println("seedcrc          : " + Integer.toHexString(seedcrc & 0xFFFF)); // & 0xFFFF is just here because there's a bug in Integer.toHexString when printing negative ints. Fix later.
		if ((results[0].execs & CoreMarkH.ID_LIST) != 0) {
			for (i=0 ; i<CorePortMe.default_num_contexts; i++) {
				System.out.println("[" + i + "]crclist       : 0x" + Integer.toHexString(results[i].crclist & 0xFFFF)); // & 0xFFFF is just here because there's a bug in Integer.toHexString when printing negative ints. Fix later.
			}
		}
		if ((results[0].execs & CoreMarkH.ID_MATRIX)  != 0) {
			for (i=0 ; i<CorePortMe.default_num_contexts; i++) {
				System.out.println("[" + i + "]crcmatrix     : 0x" + Integer.toHexString(results[i].crcmatrix & 0xFFFF)); // & 0xFFFF is just here because there's a bug in Integer.toHexString when printing negative ints. Fix later.
			}
		}
		if ((results[0].execs & CoreMarkH.ID_STATE) != 0) {
			for (i=0 ; i<CorePortMe.default_num_contexts; i++) {
				System.out.println("[" + i + "]crcstate      : 0x" + Integer.toHexString(results[i].crcstate & 0xFFFF)); // & 0xFFFF is just here because there's a bug in Integer.toHexString when printing negative ints. Fix later.
			}
		}
		for (i=0 ; i<CorePortMe.default_num_contexts; i++) {
			RTC.avroraPrintHex32(results[i].crc);
			System.out.println("[" + i + "]crcfinal      : 0x" + Integer.toHexString(results[i].crc & 0xFFFF)); // & 0xFFFF is just here because there's a bug in Integer.toHexString when printing negative ints. Fix later.
		}
		if (total_errors==0) {
			System.out.println("Correct operation validated. See readme.txt for run and reporting rules.");
		}
		if (total_errors>0){
			System.out.println("Errors detected");
		}
		if (total_errors<0) {
			System.out.println("Cannot validate operation for these seed values, please compare with results on a known platform.");
		}

		// /* And last call any target specific code for finalizing */
		// portable_fini(&(results[0].port));

		// return MAIN_RETURN_VAL;	
	}
}
