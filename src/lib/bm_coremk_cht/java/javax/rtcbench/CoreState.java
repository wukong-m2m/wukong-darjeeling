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

/*
Topic: Description
	Simple state machines like this one are used in many embedded products.
	
	For more complex state machines, sometimes a state transition table implementation is used instead, 
	trading speed of direct coding for ease of maintenance.
	
	Since the main goal of using a state machine in CoreMark is to excercise the switch/if behaviour,
	we are using a small moore machine. 
	
	In particular, this machine tests type of string input,
	trying to determine whether the input is a number or something else.
	(see core_state.png).
*/

public class CoreState {
	public static final byte CORE_STATE_START = 0;
	public static final byte CORE_STATE_INVALID = 1;
	public static final byte CORE_STATE_S1 = 2;
	public static final byte CORE_STATE_S2 = 3;
	public static final byte CORE_STATE_INT = 4;
	public static final byte CORE_STATE_FLOAT = 5;
	public static final byte CORE_STATE_EXPONENT = 6;
	public static final byte CORE_STATE_SCIENTIFIC = 7;
	public static final byte NUM_CORE_STATES = 8;

	private static int[] final_counts;
	private static int[] track_counts;

	/* Function: core_bench_state
		Benchmark function

		Go over the input twice, once direct, and once after introducing some corruption. 
	*/
	static void core_state_transition_loop(ShortWrapper p, byte[] memblock, int[] track_counts, int[] final_counts) {
		while (memblock[p.value]!=0) {
			byte fstate=core_state_transition(p,memblock,track_counts);
			final_counts[fstate]++;
		}
	}

	static short core_bench_state(int blksize, byte[] memblock, 
			short seed1, short seed2, short step, short crc) 
	{
		ShortWrapper p = new ShortWrapper();
		short pValue; // Within this method we use this so the local variable can be pinned by markloop.
		p.value=0;    // We use this to pass to core_state_transition since it needs to be able to modify the index (it's a double pointer in C).
		int i;


		for (i=0; i<NUM_CORE_STATES; i++) {
			final_counts[i]=track_counts[i]=0;
		}
		/* run the state machine over the input */
		core_state_transition_loop(p,memblock,track_counts,final_counts);

		// p=memblock;
		pValue=0; // Stays within this method, so use pValue
		while (pValue < blksize) { /* insert some corruption */
			if (memblock[pValue]!=',')
				memblock[pValue]^=(byte)seed1;
			pValue+=step;
		}
		// p=memblock;
		p.value=0;
		/* run the state machine over the input again */
		core_state_transition_loop(p,memblock,track_counts,final_counts);

		// p=memblock;
		pValue=0; // Stays within this method, so use pValue
		while (pValue < blksize) { /* undo corruption if seed1 and seed2 are equal */
			if (memblock[pValue]!=',')
				memblock[pValue]^=(byte)seed2;
			pValue+=step;
		}
		/* end timing */
		for (i=0; i<NUM_CORE_STATES; i++) {
			crc=CoreUtil.crcu32(final_counts[i],crc);
			crc=CoreUtil.crcu32(track_counts[i],crc);
		}
		return crc;
	}

	// NOT STANDARD COREMARK CODE: make this into functions to prevent them taking up memory at runtime.
	// not used during the benchmark so this won't influence the results.
	/* Default initialization patterns */
	// private static Object[] intpat  ={    "5012".getBytes(),    "1234".getBytes(),    "-874".getBytes(),    "+122".getBytes()};
	// private static Object[] floatpat={"35.54400".getBytes(),".1234500".getBytes(),"-110.700".getBytes(),"+0.64400".getBytes()};
	// private static Object[] scipat  ={"5.500e+3".getBytes(),"-.123e-2".getBytes(),"-87e+832".getBytes(),"+0.6e-12".getBytes()};
	// private static Object[] errpat  ={"T0.3e-1F".getBytes(),"-T.T++Tq".getBytes(),"1T3.4e4z".getBytes(),"34.0e-T^".getBytes()};
	private static byte[] intpat(int i) {
			 if (i==0) return     "5012".getBytes();
		else if (i==1) return     "1234".getBytes();
		else if (i==2) return     "-874".getBytes();
		else           return     "+122".getBytes();
	}
	private static byte[] floatpat(int i) {
			 if (i==0) return "35.54400".getBytes();
		else if (i==1) return ".1234500".getBytes();
		else if (i==2) return "-110.700".getBytes();
		else           return "+0.64400".getBytes();
	}
	private static byte[] scipat(int i) {
			 if (i==0) return "5.500e+3".getBytes();
		else if (i==1) return "-.123e-2".getBytes();
		else if (i==2) return "-87e+832".getBytes();
		else           return "+0.6e-12".getBytes();
	}
	private static byte[] errpat(int i) {
			 if (i==0) return "T0.3e-1F".getBytes();
		else if (i==1) return "-T.T++Tq".getBytes();
		else if (i==2) return "1T3.4e4z".getBytes();
		else           return "34.0e-T^".getBytes();
	}

	// TODO: This is a very ugly hack to prevent ProGuard from inlining core_bench_state, which causes other problems when it happens.
	//       It would be nice to have more direct control over what gets inlined or not.
	private static byte vliegtuig = 1;

	/* Function: core_init_state
		Initialize the input data for the state machine.

		Populate the input with several predetermined strings, interspersed.
		Actual patterns chosen depend on the seed parameter.
		
		Note:
		The seed parameter MUST be supplied from a source that cannot be determined at compile time
	*/
	static byte[] core_init_state(int size, short seed) {
		byte[] p=new byte[size];
		int total=0,next=0,i;
		byte[] buf=null;

		final_counts = new int[NUM_CORE_STATES];
		track_counts = new int[NUM_CORE_STATES];

		size--;
		next=0;
		while ((total+next+1)<size) {
			if (next>0) {
				for(i=0;i<next;i++)
					p[total+i]=buf[i];
				p[total+i]=',';
				total+=next+1;
			}
			seed++;
			switch (seed & 0x7) {
				case 0: /* int */
				case 1: /* int */
				case 2: /* int */
					buf=intpat((seed>>3) & 0x3);
					next=4;
				break;
				case 3: /* float */
				case 4: /* float */
					buf=floatpat((seed>>3) & 0x3);
					next=8;
				break;
				case 5: /* scientific */
				case 6: /* scientific */
					buf=scipat((seed>>3) & 0x3);
					next=8;
				break;
				case 7: /* invalid */
					buf=errpat((seed>>3) & 0x3);
					next=8;
				break;
				default: /* Never happen, just to make some compilers happy */
				break;
			}
		}
		size++;
		while (total<size) { /* fill the rest with 0 */
			p[total]=0;
			total++;
		}
		return p;
	}

	static boolean ee_isdigit(byte c) {
		boolean retval;
		retval = ((c>='0') & (c<='9')) ? true : false;
		return retval;
	}

	/* Function: core_state_transition
		Actual state machine.

		The state machine will continue scanning until either:
		1 - an invalid input is detected.
		2 - a valid number has been detected.
		
		The input pointer is updated to point to the end of the token, and the end state is returned (either specific format determined or invalid).
	*/

	private static class CoreStateTransitionParam {
		public byte[] instr;
		public short index;
	}

	// State core_state_transition( ee_u8 **instr , int[] transition_count) {
	static byte core_state_transition(ShortWrapper indexWrapper, byte[] str, int[] transition_count) {
		short index = indexWrapper.value;
		byte NEXT_SYMBOL;
		byte state=CORE_STATE_START;
		// for( ; *str && state != CORE_INVALID; str++ ) {
		for( ; (NEXT_SYMBOL = str[index])!=0 && state != CORE_STATE_INVALID; index++ ) {
			if (NEXT_SYMBOL==',') /* end of this input */ {
				index++;
				break;
			}
			switch(state) {
				case CORE_STATE_START:
					if(ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_INT;
					}
					else if( NEXT_SYMBOL == '+' || NEXT_SYMBOL == '-' ) {
						state = CORE_STATE_S1;
					}
					else if( NEXT_SYMBOL == '.' ) {
						state = CORE_STATE_FLOAT;
					}
					else {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_INVALID]++;
					}
					transition_count[CORE_STATE_START]++;
					break;
				case CORE_STATE_S1:
					if(ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_INT;
						transition_count[CORE_STATE_S1]++;
					}
					else if( NEXT_SYMBOL == '.' ) {
						state = CORE_STATE_FLOAT;
						transition_count[CORE_STATE_S1]++;
					}
					else {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_S1]++;
					}
					break;
				case CORE_STATE_INT:
					if( NEXT_SYMBOL == '.' ) {
						state = CORE_STATE_FLOAT;
						transition_count[CORE_STATE_INT]++;
					}
					else if(!ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_INT]++;
					}
					break;
				case CORE_STATE_FLOAT:
					if( NEXT_SYMBOL == 'E' || NEXT_SYMBOL == 'e' ) {
						state = CORE_STATE_S2;
						transition_count[CORE_STATE_FLOAT]++;
					}
					else if(!ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_FLOAT]++;
					}
					break;
				case CORE_STATE_S2:
					if( NEXT_SYMBOL == '+' || NEXT_SYMBOL == '-' ) {
						state = CORE_STATE_EXPONENT;
						transition_count[CORE_STATE_S2]++;
					}
					else {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_S2]++;
					}
					break;
				case CORE_STATE_EXPONENT:
					if(ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_SCIENTIFIC;
						transition_count[CORE_STATE_EXPONENT]++;
					}
					else {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_EXPONENT]++;
					}
					break;
				case CORE_STATE_SCIENTIFIC:
					if(!ee_isdigit(NEXT_SYMBOL)) {
						state = CORE_STATE_INVALID;
						transition_count[CORE_STATE_INVALID]++;
					}
					break;
				default:
					break;
			}
		}
		// *instr=str;
		indexWrapper.value = index;
		return state;
	}
}
