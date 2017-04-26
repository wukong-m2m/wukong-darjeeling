package javax.rtcbench;

import javax.rtc.*;

public class RTCBenchmark {
    public static String name = "FIX_FFT";
    public static native void test_native();
    public static boolean test_java() {
    	final int RTCTEST_FFT_ARRAYSIZE = 6;
		final int NUMNUMBERS = 1<<RTCTEST_FFT_ARRAYSIZE;
		byte data[] = new byte[NUMNUMBERS];
		byte im[] = new byte[NUMNUMBERS];

		// Fill the array
		for (int i=0; i<NUMNUMBERS; i++) {
			data[i] = (byte)(i*16);
			im[i] = (byte)0;
		}

		// System.out.println("BEFORE FFT"); for (int i=0; i<NUMNUMBERS; i++) { System.out.println("-----" + data[i] + " " + im[i]); }

		// Do the actual FFT
		rtcbenchmark_measure_java_performance(data, im, (byte)RTCTEST_FFT_ARRAYSIZE, false);

        final byte desiredOutputData[] = new byte[] { -9, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0, -7, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0, -8, 0, 0, 0, 7, 0, 0, 0 };
        final byte desiredOutputIm[] = new byte[] { 0, 0, 0, 0, -40, 0, -1, 0, 20, 0, 0, 0, -12, 0, -1, 0, 8, 0, 0, 0, -5, 0, -1, 0, 4, 0, 0, 0, -2, 0, -1, 0, 0, 0, 0, 0, 2, 0, -1, 0, -4, 0, 0, 0, 4, 0, -1, 0, -8, 0, 0, 0, 11, 0, -1, 0, -20, 0, 0, 0, 38, 0, -1, 0 };

		// System.out.println("AFTER FFT"); for (int i=0; i<NUMNUMBERS; i++) { System.out.println("-----" + data[i] + " " + im[i]); }
		
		for (int i=0; i<NUMNUMBERS; i++) {
			if (desiredOutputData[i] != data[i] || desiredOutputIm[i] != im[i]) {
				return false;
			}
		}

		return true;
	}

	private final static short N_WAVE = 256;    // full length of Sinewave[]
	private final static short N_WAVE_HALF = 128;
	private final static short LOG2_N_WAVE = 8; // log2(N_WAVE)

	// Pseudo-cosine function for 2pi equalling N_WAVE = 256.
	// Shifting by "pi/2", e.g. N_WAVE/4, gives sine.

	// signed 8-bit values written into Arduino's 32k EEPROM memory
	// Technically we could get along with a quarter of this data table but I think we shouldn't.
	private final static byte Sinewave[] = new byte[] {
	0, 3, 6, 9, 12, 15, 18, 21,
	24, 28, 31, 34, 37, 40, 43, 46,
	48, 51, 54, 57, 60, 63, 65, 68,
	71, 73, 76, 78, 81, 83, 85, 88,
	90, 92, 94, 96, 98, 100, 102, 104,
	106, 108, 109, 111, 112, 114, 115, 117,
	118, 119, 120, 121, 122, 123, 124, 124,
	125, 126, 126, 127, 127, 127, 127, 127,

	127, 127, 127, 127, 127, 127, 126, 126,
	125, 124, 124, 123, 122, 121, 120, 119,
	118, 117, 115, 114, 112, 111, 109, 108,
	106, 104, 102, 100, 98, 96, 94, 92,
	90, 88, 85, 83, 81, 78, 76, 73,
	71, 68, 65, 63, 60, 57, 54, 51,
	48, 46, 43, 40, 37, 34, 31, 28,
	24, 21, 18, 15, 12, 9, 6, 3,

	0, -3, -6, -9, -12, -15, -18, -21,
	-24, -28, -31, -34, -37, -40, -43, -46,
	-48, -51, -54, -57, -60, -63, -65, -68,
	-71, -73, -76, -78, -81, -83, -85, -88,
	-90, -92, -94, -96, -98, -100, -102, -104,
	-106, -108, -109, -111, -112, -114, -115, -117,
	-118, -119, -120, -121, -122, -123, -124, -124,
	-125, -126, -126, -127, -127, -127, -127, -127,

	-127, -127, -127, -127, -127, -127, -126, -126,
	-125, -124, -124, -123, -122, -121, -120, -119,
	-118, -117, -115, -114, -112, -111, -109, -108,
	-106, -104, -102, -100, -98, -96, -94, -92,
	-90, -88, -85, -83, -81, -78, -76, -73,
	-71, -68, -65, -63, -60, -57, -54, -51,
	-48, -46, -43, -40, -37, -34, -31, -28,
	-24, -21, -18, -15, -12, -9, -6, -3, 
	};

	// // SIN8 and COS8 - 8-bit pseudo sine and cosine for better handling. 
	// // Normalized to y * N_WAVE_HALF and x * N_WAVE / 2pi .
	// // Returns char value which can be used for integer arithmetic
	// private static final byte SIN8(short n)
	// {
	//   n = (short)(n % N_WAVE);
	//   return Sinewave[n];
	// }

	// private static final byte COS8(short n)
	// {
	//   n = (short)((n + N_WAVE/4) % N_WAVE);
	//   return Sinewave[n];
	// }


	// // FIX_MPY() - fixed-point multiplication & scaling.
	// // Substitute inline assembly for hardware-specific
	// // optimization suited to a particluar DSP processor.
	// // Scaling ensures that result remains 16-bit.
	// private static byte FIX_MPY(byte a, byte b)
	// { 
	// 	// Multiply, then scale back to one signed 8-bit value

	// 	// Original
	//     // // shift right one less bit (i.e. 7-1)
	//     // short c = (short)((short)((short)a * (short)b) >> 6);
	//     // // last bit shifted out = rounding-bit
	//     // b = (byte)(c & 0x01);
	//     // // last shift + rounding bit
	//     // a = (byte)((c >> 1) + b);
	//     // return a;

	// 	// Reformatted to make it easier to inline
	//     short c = (short)(((short)a * (short)b) >> 6);
	//     return (byte)((c >> 1) + ((c & 0x01)));
	// }
	


	// fix_fft() - perform forward/inverse fast Fourier transform.
	// fr[n],fi[n] are real and imaginary arrays, both INPUT AND
	// RESULT (in-place FFT), with 0 <= n < 2**m; set inverse to
	// 0 for forward transform (FFT), or 1 for iFFT.
	public static short rtcbenchmark_measure_java_performance(byte[] fr, byte[] fi, short m, boolean inverse) {
		RTC.startBenchmarkMeasurement_AOT();

		// Can't convert FFT to short array index since I wrote it that way from the beginning. Just adding a comment so all benchmarks will show up in the git commit.
		short mr, nn, i, j, l, k, istep, n, scale;	//int is 16-bit on Arduino (32bit on original system), using short in Java
		boolean shift;
		short qr, qi;
		byte tr, ti, wr, wi;						// byte is 8-bit signed

		n = (short)(1 << m);

		// max FFT size = N_WAVE
		if (n > N_WAVE)
			return -1;

		mr = 0;
		nn = (short)(n - 1);
		scale = 0;

		// decimation in time - re-order data
		for (m=1; m<=nn; ++m) {
			l = n;
			do {
				l >>= 1;
			} while ((short)(mr+l) > nn);
			mr = (short)((mr & (l-1)) + l);
			if (mr <= m)
				continue;
			tr = fr[m];
			fr[m] = fr[mr];
			fr[mr] = tr;
			ti = fi[m];
			fi[m] = fi[mr];
			fi[mr] = ti;
		}

		l = 1;
		k = LOG2_N_WAVE-1;

		while (l < n) {
			if (inverse) {
				// variable scaling, depending upon data
				shift = false;
				for (i=0; i<n; ++i) {
					j = fr[i];
					if (j < 0)
						j = (short)-j;
					m = fi[i];
					if (m < 0)
						m = (byte)-m;
					if (j > 127 || m > 127) {
						shift = true;
						break;
					}
				}
				if (shift)
					++scale;
			} else {
				// fixed scaling, for proper normalization --
				// there will be log2(n) passes, so this results
				// in an overall factor of 1/n, distributed to
				// maximize arithmetic accuracy.
				shift = true;
			}

			// it may not be obvious, but the shift will be
			// performed on each data point exactly once,
			// during this pass.
			istep = (short)(l << 1);
			for (m=0; m<l; ++m) {
				j = (short)(m << k);
				// 0 <= j < N_WAVE/2
				// wr =  COS8(j);
				// wi = (byte)-SIN8(j);
				// wr =  Sinewave[((short)(j + N_WAVE/4) % N_WAVE)]; // COS8(j)
				// wi = (byte)-Sinewave[(j % N_WAVE)]; // -SIN8(j)
				wr =  Sinewave[((short)(j + N_WAVE/4) & 0xFF)]; // COS8(j)
				wi = (byte)-Sinewave[(j & 0xFF)]; // -SIN8(j)
				if (inverse)
					wi = (byte)-wi;
				if (shift) {
					wr >>= 1;
					wi >>= 1;
				}
				for (i=m; i<n; i+=istep) {
					j = (short)(i + l);
					// tr = (byte)(FIX_MPY(wr,fr[j]) - FIX_MPY(wi,fi[j]));
					// ti = (byte)(FIX_MPY(wr,fi[j]) + FIX_MPY(wi,fr[j]));
					// Inlined FIX_MPY

					short c3 = fr[j];
					short c1 = (short)((short)(wr * c3) >> 6);
					short c4 = (short)((short)(wi * c3) >> 6);
					c3 = fi[j];
					short c2 = (short)((short)(wi * c3) >> 6);
					      c3 = (short)((short)(wr * c3) >> 6);

					tr = (byte)(((c1 >> 1) + ((c1 & 0x01))) - ((c2 >> 1) + ((c2 & 0x01))));
					ti = (byte)(((c3 >> 1) + ((c3 & 0x01))) + ((c4 >> 1) + ((c4 & 0x01))));
					qr = fr[i];
					qi = fi[i];
					if (shift) {
						qr >>= 1;
						qi >>= 1;
					}
					fr[j] = (byte)(qr - tr);
					fi[j] = (byte)(qi - ti);
					fr[i] = (byte)(qr + tr);
					fi[i] = (byte)(qi + ti);
				}
			}
			--k;
			l = istep;
		}

		RTC.stopBenchmarkMeasurement();

		return scale;
	}
}
