package javax.rtcbench;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    public static String name = "MD5";
    public static native void test_native();
    public static boolean test_java() {
        byte[] input = new byte[] { (byte)'m', (byte)'e', (byte)'s', (byte)'s', (byte)'a', (byte)'g', (byte)'e', (byte)' ', (byte)'d', (byte)'i', (byte)'g', (byte)'e', (byte)'s', (byte)'t' };
        byte[] desiredOutput = new byte[] { (byte)0xf9, (byte)0x6b, (byte)0x69, (byte)0x7d, (byte)0x7c, (byte)0xb7, (byte)0x93, (byte)0x8d, (byte)0x52, (byte)0x5a, (byte)0x2f, (byte)0x31, (byte)0xaa, (byte)0xf1, (byte)0x61, (byte)0xd0 };

        byte[] hash = new byte[16];

        rtcbenchmark_measure_java_performance(input, hash);
        
        for (int i=0; i<hash.length; i++) {
            if (desiredOutput[i]!=hash[i]) {
                return false;
            }
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance(byte input[], byte output[])
    {
        Stopwatch.resetAndStart();

        short i, j, len;
        short length = (short)input.length;
        byte[] buffer = new byte[64];
        int x[] = new int[16];
        int state[];

        // Init
        if (length > 56 || output.length != 16) {
            return;
        }

        for (i=0; i<64; i++) {
            buffer[i]=0;
        }

        // Copy the data
        for (i=0; i<length; i++) {
            buffer[i] = input[i];
        }
        // Append a 1 bit
        buffer[length] = (byte)0x80;

        // Fill with 0 until length = 448 bits
        // NOP

        // Append number of bits in message in 64 bit LE. (since we only accept 56 byte message, we only need to fill the first two bytes)
        buffer[56] = (byte)((length << 3) & 0xff);
        buffer[57] = (byte)(((length << 3) >>> 8) & 0xff);

        //////// Decode/Transform/Encode : Decode (byte array to int array)
        // x = Decode(buffer, 64, 0);
        // private static int[] Decode(byte buffer[], int len, int shift)
        {
        len = 64;

        for (i = j = 0; j < len; i++, j += 4)
        {
            x[i] = ((int) (buffer[(short)(j)] & 0xff))
                    | (((int) (buffer[(short)(j + 1)] & 0xff)) << 8)
                    | (((int) (buffer[(short)(j + 2)] & 0xff)) << 16)
                    | (((int) (buffer[(short)(j + 3)] & 0xff)) << 24);

        }
        // return out;
        }
        //////// Decode/Transform/Encode : End Decode (byte array to int array)

        //////// Decode/Transform/Encode : Transform
        // private static int[] Transform(byte buffer[])
        {
        state =  new int[] { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
        int a = state[0], b = state[1], c = state[2], d = state[3];

        /* Round 1 */
        
        int tmp;
        tmp = a + ((b & c) | (~b & d)) + x[(short)0] + 0xd76aa478;
        a = ((tmp << 7) | (tmp >>> (32 - 7))) + b; /* 1 */
        tmp = d + ((a & b) | (~a & c)) + x[(short)1] + 0xe8c7b756;
        d = ((tmp << 12) | (tmp >>> (32 - 12))) + a; /* 2 */
        tmp = c + ((d & a) | (~d & b)) + x[(short)2] + 0x242070db;
        c = ((tmp << 17) | (tmp >>> (32 - 17))) + d; /* 3 */
        tmp = b + ((c & d) | (~c & a)) + x[(short)3] + 0xc1bdceee;
        b = ((tmp << 22) | (tmp >>> (32 - 22))) + c; /* 4 */
        tmp = a + ((b & c) | (~b & d)) + x[(short)4] + 0xf57c0faf;
        a = ((tmp << 7) | (tmp >>> (32 - 7))) + b; /* 5 */
        tmp = d + ((a & b) | (~a & c)) + x[(short)5] + 0x4787c62a;
        d = ((tmp << 12) | (tmp >>> (32 - 12))) + a; /* 6 */
        tmp = c + ((d & a) | (~d & b)) + x[(short)6] + 0xa8304613;
        c = ((tmp << 17) | (tmp >>> (32 - 17))) + d; /* 7 */
        tmp = b + ((c & d) | (~c & a)) + x[(short)7] + 0xfd469501;
        b = ((tmp << 22) | (tmp >>> (32 - 22))) + c; /* 8 */
        tmp = a + ((b & c) | (~b & d)) + x[(short)8] + 0x698098d8;
        a = ((tmp << 7) | (tmp >>> (32 - 7))) + b; /* 9 */
        tmp = d + ((a & b) | (~a & c)) + x[(short)9] + 0x8b44f7af;
        d = ((tmp << 12) | (tmp >>> (32 - 12))) + a; /* 10 */
        tmp = c + ((d & a) | (~d & b)) + x[(short)10] + 0xffff5bb1;
        c = ((tmp << 17) | (tmp >>> (32 - 17))) + d; /* 11 */
        tmp = b + ((c & d) | (~c & a)) + x[(short)11] + 0x895cd7be;
        b = ((tmp << 22) | (tmp >>> (32 - 22))) + c; /* 12 */
        tmp = a + ((b & c) | (~b & d)) + x[(short)12] + 0x6b901122;
        a = ((tmp << 7) | (tmp >>> (32 - 7))) + b; /* 13 */
        tmp = d + ((a & b) | (~a & c)) + x[(short)13] + 0xfd987193;
        d = ((tmp << 12) | (tmp >>> (32 - 12))) + a; /* 14 */
        tmp = c + ((d & a) | (~d & b)) + x[(short)14] + 0xa679438e;
        c = ((tmp << 17) | (tmp >>> (32 - 17))) + d; /* 15 */
        tmp = b + ((c & d) | (~c & a)) + x[(short)15] + 0x49b40821;
        b = ((tmp << 22) | (tmp >>> (32 - 22))) + c; /* 16 */

        /* Round 2 */
        tmp = a +  ((b & d) | (c & ~d)) +  x[(short)1] +  0xf61e2562;
        a = ((tmp << 5) | (tmp >>> (32 - 5))) +  b; /* 17 */
        tmp = d +  ((a & c) | (b & ~c)) +  x[(short)6] +  0xc040b340;
        d = ((tmp << 9) | (tmp >>> (32 - 9))) +  a; /* 18 */
        tmp = c +  ((d & b) | (a & ~b)) +  x[(short)11] +  0x265e5a51;
        c = ((tmp << 14) | (tmp >>> (32 - 14))) +  d; /* 19 */
        tmp = b +  ((c & a) | (d & ~a)) +  x[(short)0] +  0xe9b6c7aa;
        b = ((tmp << 20) | (tmp >>> (32 - 20))) +  c; /* 20 */
        tmp = a +  ((b & d) | (c & ~d)) +  x[(short)5] +  0xd62f105d;
        a = ((tmp << 5) | (tmp >>> (32 - 5))) +  b; /* 21 */
        tmp = d +  ((a & c) | (b & ~c)) +  x[(short)10] +  0x2441453;
        d = ((tmp << 9) | (tmp >>> (32 - 9))) +  a; /* 22 */
        tmp = c +  ((d & b) | (a & ~b)) +  x[(short)15] +  0xd8a1e681;
        c = ((tmp << 14) | (tmp >>> (32 - 14))) +  d; /* 23 */
        tmp = b +  ((c & a) | (d & ~a)) +  x[(short)4] +  0xe7d3fbc8;
        b = ((tmp << 20) | (tmp >>> (32 - 20))) +  c; /* 24 */
        tmp = a +  ((b & d) | (c & ~d)) +  x[(short)9] +  0x21e1cde6;
        a = ((tmp << 5) | (tmp >>> (32 - 5))) +  b; /* 25 */
        tmp = d +  ((a & c) | (b & ~c)) +  x[(short)14] +  0xc33707d6;
        d = ((tmp << 9) | (tmp >>> (32 - 9))) +  a; /* 26 */
        tmp = c +  ((d & b) | (a & ~b)) +  x[(short)3] +  0xf4d50d87;
        c = ((tmp << 14) | (tmp >>> (32 - 14))) +  d; /* 27 */
        tmp = b +  ((c & a) | (d & ~a)) +  x[(short)8] +  0x455a14ed;
        b = ((tmp << 20) | (tmp >>> (32 - 20))) +  c; /* 28 */
        tmp = a +  ((b & d) | (c & ~d)) +  x[(short)13] +  0xa9e3e905;
        a = ((tmp << 5) | (tmp >>> (32 - 5))) +  b; /* 29 */
        tmp = d +  ((a & c) | (b & ~c)) +  x[(short)2] +  0xfcefa3f8;
        d = ((tmp << 9) | (tmp >>> (32 - 9))) +  a; /* 30 */
        tmp = c +  ((d & b) | (a & ~b)) +  x[(short)7] +  0x676f02d9;
        c = ((tmp << 14) | (tmp >>> (32 - 14))) +  d; /* 31 */
        tmp = b +  ((c & a) | (d & ~a)) +  x[(short)12] +  0x8d2a4c8a;
        b = ((tmp << 20) | (tmp >>> (32 - 20))) +  c; /* 32 */

        /* Round 3 */
        tmp = a +  (b ^ c ^ d) +  x[(short)5] +  0xfffa3942;
        a = ((tmp << 4) | (tmp >>> (32 - 4))) + b; /* 33 */
        tmp = d +  (a ^ b ^ c) +  x[(short)8] +  0x8771f681;
        d = ((tmp << 11) | (tmp >>> (32 - 11))) + a; /* 34 */
        tmp = c +  (d ^ a ^ b) +  x[(short)11] +  0x6d9d6122;
        c = ((tmp << 16) | (tmp >>> (32 - 16))) + d; /* 35 */
        tmp = b +  (c ^ d ^ a) +  x[(short)14] +  0xfde5380c;
        b = ((tmp << 23) | (tmp >>> (32 - 23))) + c; /* 36 */
        tmp = a +  (b ^ c ^ d) +  x[(short)1] +  0xa4beea44;
        a = ((tmp << 4) | (tmp >>> (32 - 4))) + b; /* 37 */
        tmp = d +  (a ^ b ^ c) +  x[(short)4] +  0x4bdecfa9;
        d = ((tmp << 11) | (tmp >>> (32 - 11))) + a; /* 38 */
        tmp = c +  (d ^ a ^ b) +  x[(short)7] +  0xf6bb4b60;
        c = ((tmp << 16) | (tmp >>> (32 - 16))) + d; /* 39 */
        tmp = b +  (c ^ d ^ a) +  x[(short)10] +  0xbebfbc70;
        b = ((tmp << 23) | (tmp >>> (32 - 23))) + c; /* 40 */
        tmp = a +  (b ^ c ^ d) +  x[(short)13] +  0x289b7ec6;
        a = ((tmp << 4) | (tmp >>> (32 - 4))) + b; /* 41 */
        tmp = d +  (a ^ b ^ c) +  x[(short)0] +  0xeaa127fa;
        d = ((tmp << 11) | (tmp >>> (32 - 11))) + a; /* 42 */
        tmp = c +  (d ^ a ^ b) +  x[(short)3] +  0xd4ef3085;
        c = ((tmp << 16) | (tmp >>> (32 - 16))) + d; /* 43 */
        tmp = b +  (c ^ d ^ a) +  x[(short)6] +  0x4881d05;
        b = ((tmp << 23) | (tmp >>> (32 - 23))) + c; /* 44 */
        tmp = a +  (b ^ c ^ d) +  x[(short)9] +  0xd9d4d039;
        a = ((tmp << 4) | (tmp >>> (32 - 4))) + b; /* 45 */
        tmp = d +  (a ^ b ^ c) +  x[(short)12] +  0xe6db99e5;
        d = ((tmp << 11) | (tmp >>> (32 - 11))) + a; /* 46 */
        tmp = c +  (d ^ a ^ b) +  x[(short)15] +  0x1fa27cf8;
        c = ((tmp << 16) | (tmp >>> (32 - 16))) + d; /* 47 */
        tmp = b +  (c ^ d ^ a) +  x[(short)2] +  0xc4ac5665;
        b = ((tmp << 23) | (tmp >>> (32 - 23))) + c; /* 48 */

        /* Round 4 */
        tmp = a +  (c ^ (b | ~d)) +  x[(short)0] + 0xf4292244;
        a = ((tmp << 6) | (tmp >>> (32 - 6))) + b; /* 49 */
        tmp = d +  (b ^ (a | ~c)) +  x[(short)7] + 0x432aff97;
        d = ((tmp << 10) | (tmp >>> (32 - 10))) + a; /* 50 */
        tmp = c +  (a ^ (d | ~b)) +  x[(short)14] + 0xab9423a7;
        c = ((tmp << 15) | (tmp >>> (32 - 15))) + d; /* 51 */
        tmp = b +  (d ^ (c | ~a)) +  x[(short)5] + 0xfc93a039;
        b = ((tmp << 21) | (tmp >>> (32 - 21))) + c; /* 52 */
        tmp = a +  (c ^ (b | ~d)) +  x[(short)12] + 0x655b59c3;
        a = ((tmp << 6) | (tmp >>> (32 - 6))) + b; /* 53 */
        tmp = d +  (b ^ (a | ~c)) +  x[(short)3] + 0x8f0ccc92;
        d = ((tmp << 10) | (tmp >>> (32 - 10))) + a; /* 54 */
        tmp = c +  (a ^ (d | ~b)) +  x[(short)10] + 0xffeff47d;
        c = ((tmp << 15) | (tmp >>> (32 - 15))) + d; /* 55 */
        tmp = b +  (d ^ (c | ~a)) +  x[(short)1] + 0x85845dd1;
        b = ((tmp << 21) | (tmp >>> (32 - 21))) + c; /* 56 */
        tmp = a +  (c ^ (b | ~d)) +  x[(short)8] + 0x6fa87e4f;
        a = ((tmp << 6) | (tmp >>> (32 - 6))) + b; /* 57 */
        tmp = d +  (b ^ (a | ~c)) +  x[(short)15] + 0xfe2ce6e0;
        d = ((tmp << 10) | (tmp >>> (32 - 10))) + a; /* 58 */
        tmp = c +  (a ^ (d | ~b)) +  x[(short)6] + 0xa3014314;
        c = ((tmp << 15) | (tmp >>> (32 - 15))) + d; /* 59 */
        tmp = b +  (d ^ (c | ~a)) +  x[(short)13] + 0x4e0811a1;
        b = ((tmp << 21) | (tmp >>> (32 - 21))) + c; /* 60 */
        tmp = a +  (c ^ (b | ~d)) +  x[(short)4] + 0xf7537e82;
        a = ((tmp << 6) | (tmp >>> (32 - 6))) + b; /* 61 */
        tmp = d +  (b ^ (a | ~c)) +  x[(short)11] + 0xbd3af235;
        d = ((tmp << 10) | (tmp >>> (32 - 10))) + a; /* 62 */
        tmp = c +  (a ^ (d | ~b)) +  x[(short)2] + 0x2ad7d2bb;
        c = ((tmp << 15) | (tmp >>> (32 - 15))) + d; /* 63 */
        tmp = b +  (d ^ (c | ~a)) +  x[(short)9] + 0xeb86d391;
        b = ((tmp << 21) | (tmp >>> (32 - 21))) + c; /* 64 */

        state[(short)0] += a;
        state[(short)1] += b;
        state[(short)2] += c;
        state[(short)3] += d;
        // return state;
        }
        //////// Decode/Transform/Encode : End Transform

        //////// Decode/Transform/Encode : Encode (int array result to byte array)
        // byte[] byteHash = Encode(Transform(buffer), 16);
        // private static byte[] Encode(int input[], int len)
        {
        len = 16;

        // byteHash = new byte[len];

        for (i = j = 0; j < len; i++, j += 4)
        {
            output[(short)(j)] = (byte) (state[i] & 0xff);
            output[(short)(j + 1)] = (byte) ((state[i] >>> 8) & 0xff);
            output[(short)(j + 2)] = (byte) ((state[i] >>> 16) & 0xff);
            output[(short)(j + 3)] = (byte) ((state[i] >>> 24) & 0xff);
        }

        // return byteHash;
        }
        //////// Decode/Transform/Encode : End Encode (int array result to byte array)

        Stopwatch.measure();
    }
}
    

