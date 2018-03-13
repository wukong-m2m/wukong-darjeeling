#include <stdint.h>
#include "darjeeling3.h"
#include "debug.h"
#include "rtc_measure.h"

#define HASH_LENGTH_BYTES 16

#define rotate_left(x,n) (((x) << (n)) | ((x) >> (32 - (n))))

// Split into separate function to avoid the compiler just optimising away the whole test.
void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint8_t input[], uint8_t inputLength, uint8_t output[]) {
	rtc_startBenchmarkMeasurement_Native();
        uint8_t i, j, len;
        uint8_t buffer[64];
        uint32_t x[16];

        for (uint8_t k=0; k<10; k++) {
        // Init
        if (inputLength > 56) {
            return;
        }

        for (i=0; i<64; i++) {
            buffer[i]=0;
        }

        // Copy the data
        for (i=0; i<inputLength; i++) {
            buffer[i] = input[i];
        }
        // Append a 1 bit
        buffer[inputLength] = 0x80;

        // Fill with 0 until inputLength = 448 bits
        // NOP

        // Append number of bits in message in 64 bit LE. (since we only accept 56 byte message, we only need to fill the first two bytes)
        buffer[56] = ((inputLength << 3) & 0xff);
        buffer[57] = (((inputLength << 3) >> 8) & 0xff);

        //////// Decode/Transform/Encode : Decode (byte array to int array)
        // x = Decode(buffer, 64, 0);
        // private static int[] Decode(byte buffer[], int len, int shift)
        {
        len = 64;

        for (i = j = 0; j < len; i++, j += 4)
        {
            x[i] = ((buffer[j] & 0xff))
                    | ((uint32_t)((buffer[j + 1] & 0xff)) << 8)
                    | ((uint32_t)((buffer[j + 2] & 0xff)) << 16)
                    | ((uint32_t)((buffer[j + 3] & 0xff)) << 24);

        }
        // return out;
        }
        //////// Decode/Transform/Encode : End Decode (byte array to int array)

        //////// Decode/Transform/Encode : Transform
        // private static int[] Transform(byte buffer[])
        uint32_t state[] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
        {
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

        /* Round 1 */
        
        a = rotate_left(a + ((b & c) | (~b & d)) + x[0] + 0xd76aa478, 7) + b; /* 1 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[1] + 0xe8c7b756, 12) + a; /* 2 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[2] + 0x242070db, 17) + d; /* 3 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[3] + 0xc1bdceee, 22) + c; /* 4 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[4] + 0xf57c0faf, 7) + b; /* 5 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[5] + 0x4787c62a, 12) + a; /* 6 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[6] + 0xa8304613, 17) + d; /* 7 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[7] + 0xfd469501, 22) + c; /* 8 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[8] + 0x698098d8, 7) + b; /* 9 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[9] + 0x8b44f7af, 12) + a; /* 10 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[10] + 0xffff5bb1, 17) + d; /* 11 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[11] + 0x895cd7be, 22) + c; /* 12 */
        a = rotate_left(a + ((b & c) | (~b & d)) + x[12] + 0x6b901122, 7) + b; /* 13 */
        d = rotate_left(d + ((a & b) | (~a & c)) + x[13] + 0xfd987193, 12) + a; /* 14 */
        c = rotate_left(c + ((d & a) | (~d & b)) + x[14] + 0xa679438e, 17) + d; /* 15 */
        b = rotate_left(b + ((c & d) | (~c & a)) + x[15] + 0x49b40821, 22) + c; /* 16 */

        /* Round 2 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[1] +  0xf61e2562, 5) +  b; /* 17 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[6] +  0xc040b340, 9) +  a; /* 18 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[11] +  0x265e5a51, 14) +  d; /* 19 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[0] +  0xe9b6c7aa, 20) +  c; /* 20 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[5] +  0xd62f105d, 5) +  b; /* 21 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[10] +  0x2441453, 9) +  a; /* 22 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[15] +  0xd8a1e681, 14) +  d; /* 23 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[4] +  0xe7d3fbc8, 20) +  c; /* 24 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[9] +  0x21e1cde6, 5) +  b; /* 25 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[14] +  0xc33707d6, 9) +  a; /* 26 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[3] +  0xf4d50d87, 14) +  d; /* 27 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[8] +  0x455a14ed, 20) +  c; /* 28 */
        a = rotate_left(a +  ((b & d) | (c & ~d)) +  x[13] +  0xa9e3e905, 5) +  b; /* 29 */
        d = rotate_left(d +  ((a & c) | (b & ~c)) +  x[2] +  0xfcefa3f8, 9) +  a; /* 30 */
        c = rotate_left(c +  ((d & b) | (a & ~b)) +  x[7] +  0x676f02d9, 14) +  d; /* 31 */
        b = rotate_left(b +  ((c & a) | (d & ~a)) +  x[12] +  0x8d2a4c8a, 20) +  c; /* 32 */

        /* Round 3 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[5] +  0xfffa3942, 4) + b; /* 33 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[8] +  0x8771f681, 11) + a; /* 34 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[11] +  0x6d9d6122, 16) + d; /* 35 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[14] +  0xfde5380c, 23) + c; /* 36 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[1] +  0xa4beea44, 4) + b; /* 37 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[4] +  0x4bdecfa9, 11) + a; /* 38 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[7] +  0xf6bb4b60, 16) + d; /* 39 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[10] +  0xbebfbc70, 23) + c; /* 40 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[13] +  0x289b7ec6, 4) + b; /* 41 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[0] +  0xeaa127fa, 11) + a; /* 42 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[3] +  0xd4ef3085, 16) + d; /* 43 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[6] +  0x4881d05, 23) + c; /* 44 */
        a = rotate_left(a +  (b ^ c ^ d) +  x[9] +  0xd9d4d039, 4) + b; /* 45 */
        d = rotate_left(d +  (a ^ b ^ c) +  x[12] +  0xe6db99e5, 11) + a; /* 46 */
        c = rotate_left(c +  (d ^ a ^ b) +  x[15] +  0x1fa27cf8, 16) + d; /* 47 */
        b = rotate_left(b +  (c ^ d ^ a) +  x[2] +  0xc4ac5665, 23) + c; /* 48 */

        /* Round 4 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[0] + 0xf4292244, 6) + b; /* 49 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[7] + 0x432aff97, 10) + a; /* 50 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[14] + 0xab9423a7, 15) + d; /* 51 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[5] + 0xfc93a039, 21) + c; /* 52 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[12] + 0x655b59c3, 6) + b; /* 53 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[3] + 0x8f0ccc92, 10) + a; /* 54 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[10] + 0xffeff47d, 15) + d; /* 55 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[1] + 0x85845dd1, 21) + c; /* 56 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[8] + 0x6fa87e4f, 6) + b; /* 57 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[15] + 0xfe2ce6e0, 10) + a; /* 58 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[6] + 0xa3014314, 15) + d; /* 59 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[13] + 0x4e0811a1, 21) + c; /* 60 */
        a = rotate_left(a +  (c ^ (b | ~d)) +  x[4] + 0xf7537e82, 6) + b; /* 61 */
        d = rotate_left(d +  (b ^ (a | ~c)) +  x[11] + 0xbd3af235, 10) + a; /* 62 */
        c = rotate_left(c +  (a ^ (d | ~b)) +  x[2] + 0x2ad7d2bb, 15) + d; /* 63 */
        b = rotate_left(b +  (d ^ (c | ~a)) +  x[9] + 0xeb86d391, 21) + c; /* 64 */

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
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
            output[j] = (state[i] & 0xff);
            output[j + 1] = ((state[i] >> 8) & 0xff);
            output[j + 2] = ((state[i] >> 16) & 0xff);
            output[j + 3] = ((state[i] >> 24) & 0xff);
        }

        // return byteHash;
        }
        //////// Decode/Transform/Encode : End Encode (int array result to byte array)

        }

	rtc_stopBenchmarkMeasurement();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint8_t input[] = { 'm', 'e', 's', 's', 'a', 'g', 'e', ' ', 'd', 'i', 'g', 'e', 's', 't' };
	uint8_t inputLength = 14;
    uint8_t desiredOutput[] = { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 };

    uint8_t hash[HASH_LENGTH_BYTES];

    rtcbenchmark_measure_native_performance(input, inputLength, hash);
    
    for (int i=0; i<HASH_LENGTH_BYTES; i++) {
        if (desiredOutput[i]!=hash[i]) {
            	avroraPrintStr("INCORRECT HASH");
            return;
        }
    }
    avroraPrintStr("HASH OK.");
}

