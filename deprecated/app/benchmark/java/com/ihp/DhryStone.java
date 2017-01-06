/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 * 
 */
public class DhryStone implements BenchmarkImplementation {

	private abstract class RecordType {
		private RecordType PointerComp;

		protected final void copyFrom(RecordType pointerGlob) {
			this.PointerComp = pointerGlob.PointerComp;
			copyImpl(pointerGlob);
		}

		protected abstract void copyImpl(RecordType pointerGlob);
	}

	private class RecordType1 extends RecordType {
		private byte EnumComp;
		private int IntComp;
		private char[] StringComp;
		protected void copyImpl(RecordType copySrc) {
			RecordType1 src = (RecordType1) copySrc;
			this.EnumComp = src.EnumComp;
			this.IntComp = src.IntComp;
			this.StringComp = new char[src.StringComp.length];
			System.arraycopy(src.StringComp, 0, this.StringComp, 0, src.StringComp.length);
		}
		public String toString() {
			return "0";
		}
	}

	private class RecordType2 extends RecordType {
		private byte Enum2Comp;
		private char[] String2Comp;
		protected void copyImpl(RecordType copySrc) {
			RecordType2 src = (RecordType2) copySrc;
			this.Enum2Comp = src.Enum2Comp;
			this.String2Comp = new char[src.String2Comp.length];
			System.arraycopy(src.String2Comp, 0, this.String2Comp, 0, src.String2Comp.length);
		}
	}

	private class RecordType3 extends RecordType {
		private char Char1Comp;
		private char Char2Comp;
		protected void copyImpl(RecordType copySrc) {
			RecordType3 src = (RecordType3) copySrc;
			this.Char1Comp = src.Char1Comp;
			this.Char2Comp = src.Char2Comp;
		}
	}

	private class RecordType4 extends RecordType3 {
		protected void copyImpl(RecordType copySrc) {
		}
	}

	private class RecordType5 extends RecordType3 {
		protected void copyImpl(RecordType copySrc) {			
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see com.ihp.TestImplementation#runTest(int)
	 */

	/* Ada version: Variables local in Proc_0 */

	private int Int1Glob, Int2Glob, Int3Glob;
	char CharIndex;
	byte EnumGlob;
	char[] String1Glob;
	char[] String2Glob;
	/* Ada version: Variables global in Pack_1 */
	RecordType PointerGlob;
	RecordType NextPointerGlob;
	int IntGlob;
	boolean BoolGlob;
	short /*char*/ Char1Glob, Char2Glob;
	int[] Array1Glob = new int[50];
	/**
	 * actually this should be int[][] but Takatuka cannot handle multidimensional arrays, so we have to go this workaround
	 * 
	 * this will consume 10000 bytes of RAM, as there is only a single element 8,7 used we might shrink it to a more convenient size for msp430
	 */
	Object[] Array2Glob = new Object[30];
	{
		for (int i = 0; i < Array2Glob.length; i++) {
			Array2Glob[i]= new int[20];
		}
	}

	/* Variables for measurement */
	int RunIndex, NumberOfRuns;
	int I;
	/* end of variables for measurement */

	private static final byte Ident1 = 0;
	private static final byte Ident2 = 1;
	private static final byte Ident3 = 2;
	private static final byte Ident4 = 3;
	private static final byte Ident5 = 4;

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new DhryStone();
//		test.runTest(1);
//	}
	
	public void runTest(int times) {
		PointerGlob = new RecordType1();
		NextPointerGlob = new RecordType1();
		PointerGlob.PointerComp = NextPointerGlob;
		((RecordType1) PointerGlob).EnumComp = Ident3;
		((RecordType1) PointerGlob).IntComp = 40;
		((RecordType1) PointerGlob).StringComp = "DHRYSTONE PROGRAM, SOME STRING"
				.toCharArray();

		String1Glob = "DHRYSTONE PROGRAM, 1'ST STRING".toCharArray();
		((int[])Array2Glob[8])[7] = 10;

		NumberOfRuns = times*100;
		
		for (RunIndex = 1; RunIndex <= NumberOfRuns; RunIndex++) {

			Proc5();
			Proc4();
			/* Char1Glob = 'A', Char2Glob = 'B', BoolGlob = false */
			Int1Glob = 2;
			Int2Glob = 3;
			String2Glob = "DHRYSTONE PROGRAM, 2'ND STRING".toCharArray();
			EnumGlob = Ident2;
			BoolGlob = !Func2(String1Glob, String2Glob);

			/* BoolGlob = true */
			while (Int1Glob < Int2Glob) { /* loop body executed once */

				Int3Glob = 5 * Int1Glob - Int2Glob;
				/* Int3Glob = 7 */
				Int3Glob = Proc7(Int1Glob, Int2Glob, Int3Glob);

				/* Int3Glob = 7 */
				Int1Glob = Int1Glob + 1;
			}

			/* Int1Glob = 3 */
			Proc8(Array1Glob, Array2Glob, Int1Glob, Int3Glob);

			/* IntGlob = 5 */
			Proc1(PointerGlob);

			char CharIndex = 'A'; //FIXME due to a bug in Takatuka we use here a (local) variable instead of a field/global variable

			for (CharIndex = 'A'; CharIndex <= Char2Glob; CharIndex++) { 
				if (EnumGlob == Func1(CharIndex, 'C')) {
					EnumGlob=Proc6(Ident1, EnumGlob);
					String2Glob = "DHRYSTONE PROGRAM, 3'RD STRING"
							.toCharArray();
					Int2Glob = RunIndex;
					IntGlob = RunIndex;
				}
			}

			/* Int1Glob = 3, Int2Glob = 3, Int3Glob = 7 */
			Int2Glob = Int2Glob * Int1Glob;
			Int1Glob = Int2Glob / Int3Glob;
			Int2Glob = 7 * (Int2Glob - Int3Glob) - Int1Glob;
			/* Int1Glob = 1, Int2Glob = 13, Int3Glob = 7 */
			Int1Glob = Proc2(Int1Glob);
		}
	}

	private void gcNSleep() {
		System.gc();
		try {
			Thread.sleep(200);
		} catch (Exception e) {
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Dhrystone";
	}

	private void Proc1(RecordType PointerParVal) {
		RecordType withPtr = PointerParVal.PointerComp;
		
		PointerParVal.PointerComp.copyFrom(PointerGlob);
		((RecordType1) PointerParVal).IntComp = 5;
		((RecordType1) withPtr).IntComp = ((RecordType1) PointerParVal).IntComp;
		withPtr.PointerComp = PointerParVal.PointerComp;
		withPtr.PointerComp = Proc3(withPtr.PointerComp);
		/* PointerParVal.PointerComp.PointerComp = PointerGlob.PointerComp */
		if (withPtr instanceof RecordType1) {
			((RecordType1) withPtr).IntComp = 6;
			((RecordType1) withPtr).EnumComp=Proc6(((RecordType1) PointerParVal).EnumComp,
					((RecordType1) withPtr).EnumComp);
			withPtr.PointerComp = PointerGlob.PointerComp;
			((RecordType1) withPtr).IntComp = Proc7(((RecordType1) withPtr).IntComp, 10,
					((RecordType1) withPtr).IntComp);
		} else {
			PointerParVal = PointerParVal.PointerComp;
		}
	}

	private int Proc2(int int1Glob2) {
		/* executed once */
		/* InParRef = 3, becomes 7 */

		int IntLoc;
		byte EnumLoc = -1;
		IntLoc = int1Glob2 + 10;
		do {
			if (Char1Glob == 'A') {
				IntLoc = IntLoc - 1;
				int1Glob2 =  IntLoc - IntGlob;
				EnumLoc = Ident1;
			}
		} while (EnumLoc != Ident1);
		return int1Glob2;
	}

	private RecordType Proc3(RecordType PointerParRef) {
		if (PointerGlob != null) {
			PointerParRef = PointerGlob.PointerComp;
		}
		((RecordType1) PointerGlob).IntComp= Proc7(10, IntGlob, ((RecordType1) PointerGlob).IntComp);
		return PointerParRef;
	}

	private void Proc4() {
		boolean BoolLoc;

		BoolLoc = Char1Glob == 'A';
		BoolGlob = BoolLoc || BoolGlob;
		Char2Glob = 'B';
	}

	private void Proc5() {
		Char1Glob = 'A';
		BoolGlob = false;
	}

	private byte Proc6(byte EnumParVal, byte EnumParRef) {
		/* executed once */
		/* EnumParVal = Ident3, EnumParRef becomes Ident2 */

		EnumParRef = EnumParVal;
		if (!Func3(EnumParVal)) {
			EnumParRef = Ident4;
		}
		switch (EnumParVal) {
		case Ident1:
			return Ident1;
		case Ident2:
			if (IntGlob > 100) {
				return Ident1;
			} else {
				return Ident4;
			}
		case Ident3:
			return Ident2;
		case Ident5:
			return Ident3;
		case Ident4:
			;
		}
		return EnumParRef;
	}

	private int Proc7(int j, int intGlob2, int intComp) {
		int IntLoc;

		IntLoc = j + 2;
		intComp = intGlob2 + IntLoc;
		return intComp;
	}

	private void Proc8(int[] Array1ParRef, Object[] array2Glob2,
						int Int1ParVal, int Int2ParVal) {
		int IntIndex;
		int IntLoc;
		IntLoc =  Int1ParVal + 5;
		Array1ParRef[IntLoc] = Int2ParVal;
		Array1ParRef[IntLoc + 1] = Array1ParRef[IntLoc];
		Array1ParRef[IntLoc + 30] = IntLoc;
		for (IntIndex = IntLoc; IntIndex <= IntLoc + 1; IntIndex++) {
			((int[])array2Glob2[IntLoc])[IntIndex] = IntLoc;
		}
		((int[])array2Glob2[IntLoc])[IntLoc - 1] = ((int[])array2Glob2[IntLoc])[IntLoc - 1] + 1;
		((int[])array2Glob2[IntLoc + 20])[IntLoc] = Array1ParRef[IntLoc];
		IntGlob = 5;
	}

	private byte Func1(char Char1ParVal, char Char2ParVal) {
		char Char1Loc, Char2Loc;
		Char1Loc = Char1ParVal;
		Char2Loc = Char1Loc;
		if (Char2Loc != Char2ParVal) {
			return Ident1;
		} else {
			Char1Glob = (short) Char1Loc;
			return Ident2;
		}
	}

	private boolean Func2(char[] String1ParRef, char[] String2ParRef) {
		int IntLoc;
		char CharLoc = 0;
		IntLoc = 2;
		while (IntLoc <= 2) {
			if (Func1(String1ParRef[IntLoc], String2ParRef[IntLoc + 1]) == Ident1) {
				CharLoc = 'A';
				IntLoc = IntLoc + 1;
			}
		}
		if ((CharLoc >= 'W') && (CharLoc < 'Z')) {
			IntLoc = 7;
		}
		if (CharLoc == 'R') {
			return true;
		} else {
			if (new String(String1ParRef).compareTo(new String(String2ParRef)) > 0) {
				IntLoc = IntLoc + 7;
				IntGlob = IntLoc;
				return true;
			} else {
				return false;
			}
		}
	}

	private boolean Func3(byte EnumParVal) {
		byte EnumLoc = EnumParVal;
		return (EnumLoc == Ident3);
	}
}
