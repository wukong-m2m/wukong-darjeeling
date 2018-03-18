package javax.rtcbench;
import javax.rtc.RTC;
import javax.rtc.Lightweight;

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
	Matrix manipulation benchmark
	
	This very simple algorithm forms the basis of many more complex algorithms. 
	
	The tight inner loop is the focus of many optimizations (compiler as well as hardware based) 
	and is thus relevant for embedded processing. 
	
	The total available data space will be divided to 3 parts:
	NxN Matrix A - initialized with small values (upper 3/4 of the bits all zero).
	NxN Matrix B - initialized with medium values (upper half of the bits all zero).
	NxN Matrix C - used for the result.

	The actual values for A and B must be derived based on input that is not available at compile time.
*/

public class CoreMatrix {
	// C MATDAT -> Java short
	// C MATRES -> Java int

	public static class MatParams {
		public short N;
		public short[] A;
		public short[] B;
		public int[] C;
	};

	static short matrix_clip(short x, boolean y) { return (y ? (short)(x & 0x0ff) : (short)(x & 0x0ffff)); }
	// static short matrix_big(short x) { return (short)(0xf000 | x); }
	// static int bit_extract(int x, byte from, byte to) { return ((x>>from) & (~(0xffffffff << to))); }

	/* Function: core_bench_matrix
		Benchmark function

		Iterate <matrix_test> N times, 
		changing the matrix values slightly by a constant amount each time.
	*/
	static short core_bench_matrix(MatParams p, short seed, short crc) {
		short N=p.N;
		int[] C=p.C;
		short[] A=p.A;
		short[] B=p.B;
		short val=seed;

		crc=CoreUtil.crc16(matrix_test(N,C,A,B,val),crc);

		return crc;
	}

	/* Function: matrix_test
		Perform matrix manipulation.

		Parameters:
		N - Dimensions of the matrix.
		C - memory for result matrix.
		A - input matrix
		B - operator matrix (not changed during operations)

		Returns:
		A CRC value that captures all results calculated in the function.
		In particular, crc of the value calculated on the result matrix 
		after each step by <matrix_sum>.

		Operation:
		
		1 - Add a constant value to all elements of a matrix.
		2 - Multiply a matrix by a constant.
		3 - Multiply a matrix by a vector.
		4 - Multiply a matrix by a matrix.
		5 - Add a constant value to all elements of a matrix.

		After the last step, matrix A is back to original contents.
	*/
	static short matrix_test(short N, int[] C, short[] A, short[] B, short val) {
		short crc=0;
		// short clipval=matrix_big(val);
		short clipval=(short)(0xf000 | val);

		matrix_add_const(N,A,val); /* make sure data changes  */

		matrix_mul_const(N,C,A,val);
		crc=CoreUtil.crc16(matrix_sum(N,C,clipval),crc);

		matrix_mul_vect(N,C,A,B);
		crc=CoreUtil.crc16(matrix_sum(N,C,clipval),crc);
	
		matrix_mul_matrix(N,C,A,B);
		crc=CoreUtil.crc16(matrix_sum(N,C,clipval),crc);

		matrix_mul_matrix_bitextract(N,C,A,B);
		crc=CoreUtil.crc16(matrix_sum(N,C,clipval),crc);
		
		matrix_add_const(N,A,(short)-val); /* return matrix to initial value */
		return crc;
	}



	//  Function : matrix_init
	// 	Initialize the memory block for matrix benchmarking.

	// 	Parameters:
	// 	blksize - Size of memory to be initialized.
	// 	memblk - Pointer to memory block.
	// 	seed - Actual values chosen depend on the seed parameter.
	// 	p - pointers to <mat_params> containing initialized matrixes.

	// 	Returns:
	// 	Matrix dimensions.
		
	// 	Note:
	// 	The seed parameter MUST be supplied from a source that cannot be determined at compile time
	static short core_init_matrix(short blksize, int seed, MatParams p) {
		short N=0;
		short[] A;
		short[] B;
		short order=1;
		short val;
		short i=0,j=0;
		if (seed==0)
			seed=1;
		while (j<blksize) {
			i++;
			j=(short)(i*i*2*4);
		}
		N=(short)(i-1);
		A=new short[N*N];
		B=new short[N*N];

		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				seed = ( ( order * seed ) % 65536 );
				val = (short)(seed + order);
				val=matrix_clip(val,false);
				B[i*N+j] = val;
				val = (short)(val + order);
				val=matrix_clip(val,true);
				A[i*N+j] = val;
				order++;
			}
		}

		p.A=A;
		p.B=B;
		p.C=new int[N*N];
		p.N=N;

		return N;
	}

	/* Function: matrix_sum
		Calculate a function that depends on the values of elements in the matrix.

		For each element, accumulate into a temporary variable.
		
		As long as this value is under the parameter clipval, 
		add 1 to the result if the element is bigger then the previous.
		
		Otherwise, reset the accumulator and add 10 to the result.
	*/
	@Lightweight(rank=4) // Needs to come after crc
	static short matrix_sum(short N, int[] C, short clipval) {
		int tmp=0,prev=0,cur=0;
		short ret=0;
		short i,j;
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				cur=C[i*N+j];
				tmp+=cur;
				if (tmp>clipval) {
					ret+=10;
					tmp=0;
				} else {
					if (cur>prev) {
						ret++;
					}
					// ret += (cur>prev) ? 1 : 0;
				}
				prev=cur;
			}
		}
		return ret;
	}

	/* Function: matrix_mul_const
		Multiply a matrix by a constant.
		This could be used as a scaler for instance.
	*/
	static void matrix_mul_const(short N, int[] C, short[] A, short val) {
		short i,j;
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				C[i*N+j]=(int)A[i*N+j] * (int)val;
			}
		}
	}

	/* Function: matrix_add_const
		Add a constant value to all elements of a matrix.
	*/
	static void matrix_add_const(short N, short[] A, short val) {
		short i,j;
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				A[i*N+j] += val;
			}
		}
	}

	/* Function: matrix_mul_vect
		Multiply a matrix by a vector.
		This is common in many simple filters (e.g. fir where a vector of coefficients is applied to the matrix.)
	*/
	static void matrix_mul_vect(short N, int[] C, short[] A, short[] B) {
		short i,j;
		for (i=0; i<N; i++) {
			C[i]=0;
			for (j=0; j<N; j++) {
				C[i]+=(int)A[i*N+j] * (int)B[j];
			}
		}
	}

	/* Function: matrix_mul_matrix
		Multiply a matrix by a matrix.
		Basic code is used in many algorithms, mostly with minor changes such as scaling.
	*/
	static void matrix_mul_matrix(short N, int[] C, short[] A, short[] B) {
		short i,j,k;
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				C[i*N+j]=0;
				for(k=0;k<N;k++)
				{
					C[i*N+j]+=(int)A[i*N+k] * (int)B[k*N+j];
				}
			}
		}
	}

	/* Function: matrix_mul_matrix_bitextract
		Multiply a matrix by a matrix, and extract some bits from the result.
		Basic code is used in many algorithms, mostly with minor changes such as scaling.
	*/
	static void matrix_mul_matrix_bitextract(short N, int[] C, short[] A, short[] B) {
		short i,j,k;
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				C[i*N+j]=0;
				for(k=0;k<N;k++)
				{
					int tmp=(int)A[i*N+k] * (int)B[k*N+j];

					// C[i_times_N+j]+=bit_extract(tmp,(byte)2,(byte)4)*bit_extract(tmp,(byte)5,(byte)7);
					// Good case where ProGuard inlining doesn't work: when bit_extract is a method, we save the call,
					// but it doesn't realise 'from' and 'to' are now constants.

					C[i*N+j]+= (short)((tmp>>2) & (~(0xffffffff << 4))) * (short)((tmp>>5) & (~(0xffffffff << 7)));
				}
			}
		}
	}
}





