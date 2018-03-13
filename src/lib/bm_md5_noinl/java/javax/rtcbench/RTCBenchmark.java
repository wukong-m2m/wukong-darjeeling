package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class RTCBenchmark {
    public static String name = "MD5 NOINL";
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

    @Lightweight
    private static int rotate_left(int x, int n)
    {
        return (x << n) | (x >>> (32 - n));
    }

    public static void rtcbenchmark_measure_java_performance(byte input[], byte output[])
    {
        short i, j, len;
        short length = (short)input.length;
        byte[] buffer = new byte[64];
        int x[] = new int[16];
        int state[];

        for (byte k=0; k<10; k++) {
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
        
        a = rotate_left(a + ((b & c) | (~b & d)) + x[0] + 0xd76aa478, (short)7) + b; /* 1 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[1] + 0xe8c7b756, (short)12) + a; /* 2 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[2] + 0x242070db, (short)17) + d; /* 3 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[3] + 0xc1bdceee, (short)22) + c; /* 4 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[4] + 0xf57c0faf, (short)7) + b; /* 5 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[5] + 0x4787c62a, (short)12) + a; /* 6 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[6] + 0xa8304613, (short)17) + d; /* 7 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[7] + 0xfd469501, (short)22) + c; /* 8 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[8] + 0x698098d8, (short)7) + b; /* 9 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[9] + 0x8b44f7af, (short)12) + a; /* 10 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[10] + 0xffff5bb1, (short)17) + d; /* 11 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[11] + 0x895cd7be, (short)22) + c; /* 12 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[12] + 0x6b901122, (short)7) + b; /* 13 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[13] + 0xfd987193, (short)12) + a; /* 14 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[14] + 0xa679438e, (short)17) + d; /* 15 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[15] + 0x49b40821, (short)22) + c; /* 16 */

        /* Round 2 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[1] +  0xf61e2562, (short)5) +  b; /* 17 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[6] +  0xc040b340, (short)9) +  a; /* 18 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[11] +  0x265e5a51, (short)14) +  d; /* 19 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[0] +  0xe9b6c7aa, (short)20) +  c; /* 20 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[5] +  0xd62f105d, (short)5) +  b; /* 21 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[10] +  0x2441453, (short)9) +  a; /* 22 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[15] +  0xd8a1e681, (short)14) +  d; /* 23 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[4] +  0xe7d3fbc8, (short)20) +  c; /* 24 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[9] +  0x21e1cde6, (short)5) +  b; /* 25 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[14] +  0xc33707d6, (short)9) +  a; /* 26 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[3] +  0xf4d50d87, (short)14) +  d; /* 27 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[8] +  0x455a14ed, (short)20) +  c; /* 28 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[13] +  0xa9e3e905, (short)5) +  b; /* 29 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[2] +  0xfcefa3f8, (short)9) +  a; /* 30 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[7] +  0x676f02d9, (short)14) +  d; /* 31 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[12] +  0x8d2a4c8a, (short)20) +  c; /* 32 */

        /* Round 3 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[5] +  0xfffa3942, (short)4) + b; /* 33 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[8] +  0x8771f681, (short)11) + a; /* 34 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[11] +  0x6d9d6122, (short)16) + d; /* 35 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[14] +  0xfde5380c, (short)23) + c; /* 36 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[1] +  0xa4beea44, (short)4) + b; /* 37 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[4] +  0x4bdecfa9, (short)11) + a; /* 38 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[7] +  0xf6bb4b60, (short)16) + d; /* 39 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[10] +  0xbebfbc70, (short)23) + c; /* 40 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[13] +  0x289b7ec6, (short)4) + b; /* 41 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[0] +  0xeaa127fa, (short)11) + a; /* 42 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[3] +  0xd4ef3085, (short)16) + d; /* 43 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[6] +  0x4881d05, (short)23) + c; /* 44 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[9] +  0xd9d4d039, (short)4) + b; /* 45 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[12] +  0xe6db99e5, (short)11) + a; /* 46 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[15] +  0x1fa27cf8, (short)16) + d; /* 47 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[2] +  0xc4ac5665, (short)23) + c; /* 48 */

        /* Round 4 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[0] + 0xf4292244, (short)6) + b; /* 49 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[7] + 0x432aff97, (short)10) + a; /* 50 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[14] + 0xab9423a7, (short)15) + d; /* 51 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[5] + 0xfc93a039, (short)21) + c; /* 52 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[12] + 0x655b59c3, (short)6) + b; /* 53 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[3] + 0x8f0ccc92, (short)10) + a; /* 54 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[10] + 0xffeff47d, (short)15) + d; /* 55 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[1] + 0x85845dd1, (short)21) + c; /* 56 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[8] + 0x6fa87e4f, (short)6) + b; /* 57 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[15] + 0xfe2ce6e0, (short)10) + a; /* 58 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[6] + 0xa3014314, (short)15) + d; /* 59 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[13] + 0x4e0811a1, (short)21) + c; /* 60 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[4] + 0xf7537e82, (short)6) + b; /* 61 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[11] + 0xbd3af235, (short)10) + a; /* 62 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[2] + 0x2ad7d2bb, (short)15) + d; /* 63 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[9] + 0xeb86d391, (short)21) + c; /* 64 */

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

        }
    }
}
    

