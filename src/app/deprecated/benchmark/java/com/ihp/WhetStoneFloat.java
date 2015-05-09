/**
 * 
 */
package com.ihp;

/**
 * Whetstone benchmark in Java. This program is a translation of a translation
 * of the original Algol version in "A Synthetic Benchmark" by H.J. Curnow and
 * B.A. Wichman in Computer Journal, Vol 19 #1, February 1976. Found at
 * http://www.netlib.org/benchmark/whetstonec
 * 
 * @author Michael Maaser
 * 
 */
public class WhetStoneFloat implements BenchmarkImplementation {

	float x1, x2, x3, x4, x, y, z, t, t1, t2;
	float e1[] = new float[4];
	int i, j, k, l, n1, n2, n3, n4, n6, n7, n8, n9, n10, n11;

	/*
	 * (non-Javadoc)
	 * 
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int ITERATIONS) {

		/* initialize constants */

		t = 0.499975F;
		t1 = 0.50025F;
		t2 = 2.0F;

		/* set values of module weights */

		n1 = 0 * ITERATIONS;
		n2 = 12 * ITERATIONS;
		n3 = 14 * ITERATIONS;
		n4 = 345 * ITERATIONS;
		n6 = 210 * ITERATIONS;
		n7 = 32 * ITERATIONS;
		n8 = 899 * ITERATIONS;
		n9 = 616 * ITERATIONS;
		n10 = 0 * ITERATIONS;
		n11 = 93 * ITERATIONS;

		/* MODULE 1: simple identifiers */

		x1 = 1.0F;
		x2 = x3 = x4 = -1.0F;

		for (i = 1; i <= n1; i += 1) {
			x1 = (x1 + x2 + x3 - x4) * t;
			x2 = (x1 + x2 - x3 - x4) * t;
			x3 = (x1 - x2 + x3 + x4) * t;
			x4 = (-x1 + x2 + x3 + x4) * t;
		}
		// System.out.println(""+n1+" "+n1+" "+n1+" "+x1+" "+x2+" "+x3+" "+x4);

		/* MODULE 2: array elements */

		e1[0] = 1.0F;
		e1[1] = e1[2] = e1[3] = -1.0F;

		for (i = 1; i <= n2; i += 1) {
			e1[0] = (e1[0] + e1[1] + e1[2] - e1[3]) * t;
			e1[1] = (e1[0] + e1[1] - e1[2] + e1[3]) * t;
			e1[2] = (e1[0] - e1[1] + e1[2] + e1[3]) * t;
			e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t;
		}
		// System.out.println(n2+" "+n3+" "+n2+" "+e1[0]+" "+e1[1]+" "+e1[2]+" "+e1[3]);

		/* MODULE 3: array as parameter */

		for (i = 1; i <= n3; i += 1) {
			pa(e1);
		}
		// System.out.println(n3+" "+n2+" "+n2+" "+e1[0]+" "+ e1[1]+" "+
		// e1[2]+" "+ e1[3]);

		/* MODULE 4: conditional jumps */

		j = 1;
		for (i = 1; i <= n4; i += 1) {
			if (j == 1) {
				j = 2;
			} else {
				j = 3;
			}
			if (j > 2) {
				j = 0;
			} else {
				j = 1;
			}
			if (j < 1) {
				j = 1;
			} else {
				j = 0;
			}
		}
		// System.out.println(n4+" "+j+" "+ j+" "+x1+" "+x2+" "+x3+" "+x4);

		/* MODULE 5: omitted */

		/* MODULE 6: integer arithmetic */

		j = 1;
		k = 2;
		l = 3;

		for (i = 1; i <= n6; i += 1) {
			j = j * (k - j) * (l - k);
			k = l * k - (l - j) * k;
			l = (l - k) * (k + j);

			e1[l - 2] = j + k + l; /* C arrays are zero based */
			e1[k - 2] = j * k * l;
		}
		// System.out.println(n6+" "+j+" "+k+" "+e1[0]+" "+e1[1]+" "+e1[2]+" "+e1[3]);

		/* MODULE 7: trig. functions */
		// atan is not implemented for TAKATUKA and DARJEELING's Math
		// implementation is lacking any method which should accept floating
		// point numbers
		// x = y = 0.5;
		//
		// for(i = 1; i <= n7; i +=1) {
		// x = t *
		// Math.atan(t2*Math.sin(x)*Math.cos(x)/(Math.cos(x+y)+Math.cos(x-y)-1.0));
		// y = t *
		// Math.atan(t2*Math.sin(y)*Math.cos(y)/(Math.cos(x+y)+Math.cos(x-y)-1.0));
		// }
		// System.out.println(n7+" "+j+" "+k+" "+x+" "+x+" "+y+" "+y);

		/* MODULE 8: procedure calls */

		x = y = z = 1.0F;

		for (i = 1; i <= n8; i += 1) {
			z = p3(x, y);
		}
		// System.out.println(n8+" "+j+" "+k+" "+x+" "+y+" "+z+" "+z);

		/* MODULE9: array references */

		j = 1;
		k = 2;
		l = 3;

		e1[0] = 1.0F;
		e1[1] = 2.0F;
		e1[2] = 3.0F;

		for (i = 1; i <= n9; i += 1) {
			p0();
		}
		// System.out.println(n9+" "+j+" "+k+" "+e1[0]+" "+e1[1]+" "+e1[2]+" "+e1[3]);

		/* MODULE10: integer arithmetic */

		j = 2;
		k = 3;

		for (i = 1; i <= n10; i += 1) {
			j = j + k;
			k = j + k;
			j = k - j;
			k = k - j - j;
		}
		// System.out.println(n10+" "+j+" "+k+" "+ x1+" "+x2+" "+x3+" "+x4);

		/* MODULE11: standard functions */

		// exp and log are not implemented for TAKATUKA and DARJEELING's Math
		// implementation is lacking any method which should accept floating
		// point numbers
		// x = 0.75;
		// for(i = 1; i <= n11; i +=1)
		// x = Math.sqrt( Math.exp( Math.log(x) / t1));
		//
		// System.out.println(n11+" "+j+" "+k+" "+x+" "+x+" "+x+" "+x);
	}

	private void pa(float[] e) {
		int j = 0;
		do {
			e[0] = (e[0] + e[1] + e[2] - e[3]) * t;
			e[1] = (e[0] + e[1] - e[2] + e[3]) * t;
			e[2] = (e[0] - e[1] + e[2] + e[3]) * t;
			e[3] = (-e[0] + e[1] + e[2] + e[3]) / t2;
			j += 1;
		} while (j < 6);
	}

	private float p3(float x, float y) {
		x = t * (x + y);
		y = t * (x + y);
		return (x + y) / t2;
	}

	private void p0() {
		e1[j] = e1[k];
		e1[k] = e1[l];
		e1[l] = e1[j];
	}

	/*
	 * these should be implemented in java.lang.Math private float pow(float x,
	 * int n) { if (n == 0) return 1; else if (n % 2 == 0) return pow(x * x, n
	 * >> 1); else return x * pow(x * x, n >> 1); }
	 * 
	 * private float atan(float x) {
	 * 
	 * float temp1 = x >= 0 ? x : -x; float temp2 = temp1 <= 1.0 ? temp1 :
	 * (temp1 - 1) / (temp1 + 1); float sum = temp2;
	 * 
	 * for (int i = 1; i != 6; i++) { sum += ((i % 2 > 0) ? -1 : 1) * pow(temp2,
	 * (i << 1) + 1) / ((i << 1) + 1); } if (temp1 > 1.0) sum += 0.785398;
	 * return x >= 0 ? sum : -sum; }
	 */

	/*
	 * (non-Javadoc)
	 * 
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Whetstone (Float)";
	}

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new WhetStoneFloat();
//		test.runTest(1);
//	}

}
