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
#include "coremark.h"
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

#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
ee_s16 matrix_test(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val);
ee_s16 matrix_sum(ee_u16 N, MATRES *C, MATDAT clipval);
void matrix_mul_const(ee_u16 N, MATRES *C, MATDAT *A, MATDAT val);
void matrix_mul_vect(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix_bitextract(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_add_const(ee_u16 N, MATDAT *A, MATDAT val);
#else
ee_s16 matrix_test(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val);
ee_s16 matrix_sum(ee_u32 N, MATRES *C, MATDAT clipval);
void matrix_mul_const(ee_u32 N, MATRES *C, MATDAT *A, MATDAT val);
void matrix_mul_vect(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix_bitextract(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_add_const(ee_u32 N, MATDAT *A, MATDAT val);
#endif


#define matrix_test_next(x) (x+1)
#define matrix_clip(x,y) ((y) ? (x) & 0x0ff : (x) & 0x0ffff)
#define matrix_big(x) (0xf000 | (x))
#define bit_extract(x,from,to) (((x)>>(from)) & (~(0xffffffff << (to))))

#if CORE_DEBUG
void printmat(MATDAT *A, ee_u32 N, char *name) {
	ee_u32 i,j;
	ee_printf("Matrix %s [%dx%d]:\n",name,N,N);
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			if (j!=0)
				ee_printf(",");
			ee_printf("%d",A[i*N+j]);
		}
		ee_printf("\n");
	}
}
void printmatC(MATRES *C, ee_u32 N, char *name) {
	ee_u32 i,j;
	ee_printf("Matrix %s [%dx%d]:\n",name,N,N);
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			if (j!=0)
				ee_printf(",");
			ee_printf("%d",C[i*N+j]);
		}
		ee_printf("\n");
	}
}
#endif
/* Function: core_bench_matrix
	Benchmark function

	Iterate <matrix_test> N times, 
	changing the matrix values slightly by a constant amount each time.
*/
ee_u16 core_bench_matrix(mat_params *p, ee_s16 seed, ee_u16 crc) {
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
	ee_u16 N=p->N;
#else
	ee_u32 N=p->N;
#endif
	MATRES *C=p->C;
	MATDAT *A=p->A;
	MATDAT *B=p->B;
	MATDAT val=(MATDAT)seed;

	crc=crc16(matrix_test(N,C,A,B,val),crc);

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
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
ee_s16 matrix_test(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val) {
#else
ee_s16 matrix_test(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val) {
#endif
	ee_u16 crc=0;
	MATDAT clipval=matrix_big(val);

	matrix_add_const(N,A,val); /* make sure data changes  */
#if CORE_DEBUG
	printmat(A,N,"matrix_add_const");
#endif
	matrix_mul_const(N,C,A,val);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_const");
#endif
	matrix_mul_vect(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_vect");
#endif
	matrix_mul_matrix(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_matrix");
#endif
	matrix_mul_matrix_bitextract(N,C,A,B);
	crc=crc16(matrix_sum(N,C,clipval),crc);
#if CORE_DEBUG
	printmatC(C,N,"matrix_mul_matrix_bitextract");
#endif
	
	matrix_add_const(N,A,-val); /* return matrix to initial value */
	return crc;
}

/* Function : matrix_init
	Initialize the memory block for matrix benchmarking.

	Parameters:
	blksize - Size of memory to be initialized.
	memblk - Pointer to memory block.
	seed - Actual values chosen depend on the seed parameter.
	p - pointers to <mat_params> containing initialized matrixes.

	Returns:
	Matrix dimensions.
	
	Note:
	The seed parameter MUST be supplied from a source that cannot be determined at compile time
*/
ee_u32 core_init_matrix(ee_u32 blksize, void *memblk, ee_s32 seed, mat_params *p) {
	ee_u32 N=0;
	MATDAT *A;
	MATDAT *B;
#ifdef CORE_OPTIMISATION_MANUALLY_INLINE_BIT_EXTRACT // This has nothing to do with bit_extract, but it was done in the same commit for the Java version.
	ee_u16 order=1;
#else
	ee_s32 order=1;
#endif
	MATDAT val;
	ee_u32 i=0,j=0;
	if (seed==0)
		seed=1;
	while (j<blksize) {
		i++;
		j=i*i*2*4;		
	}
	N=i-1;
	A=(MATDAT *)align_mem(memblk);
	B=A+N*N;

#ifdef CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP
	ee_u16 i_times_N = 0;
#endif
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			seed = ( ( order * seed ) % 65536 );
			val = (seed + order);
			val=matrix_clip(val,0);
#ifdef CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP
			B[i_times_N+j] = val;
#else
			B[i*N+j] = val;
#endif
			val =  (val + order);
			val=matrix_clip(val,1);
#ifdef CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP
			A[i_times_N+j] = val;
#else
			A[i*N+j] = val;
#endif
			order++;
		}
#ifdef CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP
		i_times_N += N;
#endif

	}

	p->A=A;
	p->B=B;
	p->C=(MATRES *)align_mem(B+N*N);
	p->N=N;
#if CORE_DEBUG
	printmat(A,N,"A");
	printmat(B,N,"B");
#endif
	return N;
}

/* Function: matrix_sum
	Calculate a function that depends on the values of elements in the matrix.

	For each element, accumulate into a temporary variable.
	
	As long as this value is under the parameter clipval, 
	add 1 to the result if the element is bigger then the previous.
	
	Otherwise, reset the accumulator and add 10 to the result.
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
ee_s16 matrix_sum(ee_u16 N, MATRES *C, MATDAT clipval) {
#else
ee_s16 matrix_sum(ee_u32 N, MATRES *C, MATDAT clipval) {
#endif
	MATRES tmp=0,prev=0,cur=0;
	ee_s16 ret=0;
	ee_u32 i,j;
#if defined(CORE_OPTIMISATION_AVOID_SOME_ADDITIONS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		ee_u16 i_times_N_plus_N = i_times_N + N;
		for (j=i_times_N; j<i_times_N_plus_N; j++) {
			cur=C[j];
			tmp+=cur;
			if (tmp>clipval) {
				ret+=10;
				tmp=0;
			} else {
				ret += (cur>prev) ? 1 : 0;
			}
			prev=cur;
		}
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			cur=C[i_times_N+j];
			tmp+=cur;
			if (tmp>clipval) {
				ret+=10;
				tmp=0;
			} else {
				ret += (cur>prev) ? 1 : 0;
			}
			prev=cur;
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			cur=C[i*N+j];
			tmp+=cur;
			if (tmp>clipval) {
				ret+=10;
				tmp=0;
			} else {
				ret += (cur>prev) ? 1 : 0;
			}
			prev=cur;
		}
	}
#endif
	return ret;
}

/* Function: matrix_mul_const
	Multiply a matrix by a constant.
	This could be used as a scaler for instance.
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
void matrix_mul_const(ee_u16 N, MATRES *C, MATDAT *A, MATDAT val) {
	ee_u16 i,j;
#else
void matrix_mul_const(ee_u32 N, MATRES *C, MATDAT *A, MATDAT val) {
	ee_u32 i,j;
#endif
#if defined(CORE_OPTIMISATION_AVOID_SOME_ADDITIONS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		ee_u16 i_times_N_plus_N = i_times_N + N;
		for (j=i_times_N; j<i_times_N_plus_N; j++) {
			C[j]=(MATRES)A[j] * (MATRES)val;
		}
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i_times_N+j]=(MATRES)A[i_times_N+j] * (MATRES)val;
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i*N+j]=(MATRES)A[i*N+j] * (MATRES)val;
		}
	}
#endif
}

/* Function: matrix_add_const
	Add a constant value to all elements of a matrix.
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
void matrix_add_const(ee_u16 N, MATDAT *A, MATDAT val) {
	ee_u16 i,j;
#else
void matrix_add_const(ee_u32 N, MATDAT *A, MATDAT val) {
	ee_u32 i,j;
#endif
#if defined(CORE_OPTIMISATION_AVOID_SOME_ADDITIONS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		ee_u16 i_times_N_plus_N = i_times_N + N;
		for (j=i_times_N; j<i_times_N_plus_N; j++) {
			A[j] += val;
		}
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			A[i_times_N+j] += val;
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			A[i*N+j] += val;
		}
	}
#endif
}

/* Function: matrix_mul_vect
	Multiply a matrix by a vector.
	This is common in many simple filters (e.g. fir where a vector of coefficients is applied to the matrix.)
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
void matrix_mul_vect(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u16 i,j;
#else
void matrix_mul_vect(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u32 i,j;
#endif
#if defined(CORE_OPTIMISATION_REDUCE_C_ARRAY_ACCESS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		MATRES C_value=0;
		for (j=0; j<N; j++) {
			C_value+=(MATRES)A[i_times_N+j] * (MATRES)B[j];
		}
		C[i]=C_value;
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		C[i]=0;
		for (j=0; j<N; j++) {
			C[i]+=(MATRES)A[i_times_N+j] * (MATRES)B[j];
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		C[i]=0;
		for (j=0; j<N; j++) {
			C[i]+=(MATRES)A[i*N+j] * (MATRES)B[j];
		}
	}
#endif
}

/* Function: matrix_mul_matrix
	Multiply a matrix by a matrix.
	Basic code is used in many algorithms, mostly with minor changes such as scaling.
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
void matrix_mul_matrix(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u16 i,j,k;
#else
void matrix_mul_matrix(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u32 i,j,k;
#endif
#if defined(CORE_OPTIMISATION_REDUCE_C_ARRAY_ACCESS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			MATRES C_value=0;
			for(k=0;k<N;k++)
			{
				C_value+=(MATRES)A[i_times_N+k] * (MATRES)B[k*N+j];
			}
			C[i_times_N+j]=C_value;
		}
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i_times_N+j]=0;
			for(k=0;k<N;k++)
			{
				C[i_times_N+j]+=(MATRES)A[i_times_N+k] * (MATRES)B[k*N+j];
			}
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i*N+j]=0;
			for(k=0;k<N;k++)
			{
				C[i*N+j]+=(MATRES)A[i*N+k] * (MATRES)B[k*N+j];
			}
		}
	}
#endif
}

/* Function: matrix_mul_matrix_bitextract
	Multiply a matrix by a matrix, and extract some bits from the result.
	Basic code is used in many algorithms, mostly with minor changes such as scaling.
*/
#ifdef CORE_OPTIMISATION_SHORT_ARRAY_INDEX
void matrix_mul_matrix_bitextract(ee_u16 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u16 i,j,k;
#else
void matrix_mul_matrix_bitextract(ee_u32 N, MATRES *C, MATDAT *A, MATDAT *B) {
	ee_u32 i,j,k;
#endif
#if defined(CORE_OPTIMISATION_REDUCE_C_ARRAY_ACCESS)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			MATRES C_value=0;
			for(k=0;k<N;k++)
			{
				MATRES tmp=(MATRES)A[i_times_N+k] * (MATRES)B[k*N+j];

#ifdef CORE_OPTIMISATION_MANUALLY_INLINE_BIT_EXTRACT
				C_value+=((tmp>>2) & (~(0xffffffff << 4))) * ((tmp>>5) & (~(0xffffffff << 7)));
#else
				C_value+=bit_extract(tmp,2,4)*bit_extract(tmp,5,7);
#endif
			}
			C[i_times_N+j]=C_value;
		}
		i_times_N += N;
	}
#elif defined(CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP)
	ee_u16 i_times_N = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i_times_N+j]=0;
			for(k=0;k<N;k++)
			{
				MATRES tmp=(MATRES)A[i_times_N+k] * (MATRES)B[k*N+j];

#ifdef CORE_OPTIMISATION_MANUALLY_INLINE_BIT_EXTRACT
				C[i_times_N+j]+=((tmp>>2) & (~(0xffffffff << 4))) * ((tmp>>5) & (~(0xffffffff << 7)));
#else
				C[i_times_N+j]+=bit_extract(tmp,2,4)*bit_extract(tmp,5,7);
#endif
			}
		}
		i_times_N += N;
	}
#else
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			C[i*N+j]=0;
			for(k=0;k<N;k++)
			{
				MATRES tmp=(MATRES)A[i*N+k] * (MATRES)B[k*N+j];
#ifdef CORE_OPTIMISATION_MANUALLY_INLINE_BIT_EXTRACT
				C[i*N+j]+=((tmp>>2) & (~(0xffffffff << 4))) * ((tmp>>5) & (~(0xffffffff << 7)));
#else
				C[i*N+j]+=bit_extract(tmp,2,4)*bit_extract(tmp,5,7);
#endif
			}
		}
	}
#endif
}
