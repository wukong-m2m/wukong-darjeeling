package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.ConstArray;
import javax.rtc.Lightweight;

public class RTCBenchmark {
    public static String name = "RC5 BASE";
    public static native void test_native();
    public static boolean test_java() {
        byte NUMNUMBERS = 8;
        final byte[] test_key = { (byte)0x91, (byte)0x5f, (byte)0x46, (byte)0x19, (byte)0xbe, (byte)0x41, (byte)0xb2, (byte)0x51, (byte)0x63, (byte)0x55, (byte)0xa5, (byte)0x01, (byte)0x10, (byte)0xa9, (byte)0xce, (byte)0x91 };
        final byte[] test_pt  = { (byte)0x21, (byte)0xa5, (byte)0xdb, (byte)0xee, (byte)0x15, (byte)0x4b, (byte)0x8f, (byte)0x6d };
        final byte[] test_ct  = { (byte)0xf7, (byte)0xc0, (byte)0x13, (byte)0xac, (byte)0x5b, (byte)0x2b, (byte)0x89, (byte)0x52 };

        byte[] tmp0 = new byte[NUMNUMBERS];
        // byte[] tmp1 = new byte[NUMNUMBERS];
        int x, y, err;
        // rc5_key key;
        int skey_rounds = 12;
        int[] skey_K = new int[50];

        /* setup key */
        if ((err = rc5_setup(test_key, 16, skey_rounds, skey_K)) != CRYPT_OK) {
            System.out.println("TEST SETUP FAILED");
            return false;
        }

        /* encrypt and decrypt */
        rc5_ecb_encrypt(test_pt, tmp0, skey_rounds, skey_K);
        // rc5_ecb_decrypt(tmp0, tmp1, skey_rounds, skey_K);

        System.out.print("Original:");
        for (byte k=0; k<NUMNUMBERS; k++) {
            System.out.print(" " + test_pt[k]);
        }
        System.out.println(" done.");
        System.out.print("Encrypted:");
        for (byte k=0; k<NUMNUMBERS; k++) {
            System.out.print(" " + tmp0[k]);
        }
        System.out.println(" done.");
        // System.out.print("Decrypted:");
        // for (byte k=0; k<NUMNUMBERS; k++) {
        //     System.out.print(" " + tmp1[k]);
        // }
        // System.out.println(" done.");

        for (byte k=0; k<NUMNUMBERS; k++) {
          if ((tmp0[k] != test_ct[k])) { // || (tmp1[k] != test_pt[k])) {
            return false;
          }
        }

        return true;
    }

    private final static int CRYPT_OK = 0;
    private final static int CRYPT_INVALID_ROUNDS = 1;
    private final static int CRYPT_INVALID_KEYSIZE = 2;
    private final static int CRYPT_FAIL_TESTVECTOR = 3;

    @ConstArray
    private static class stab {
        public final static int data[] = new int[] {
            0xb7e15163, 0x5618cb1c, 0xf45044d5, 0x9287be8e, 0x30bf3847, 0xcef6b200, 0x6d2e2bb9, 0x0b65a572,
            0xa99d1f2b, 0x47d498e4, 0xe60c129d, 0x84438c56, 0x227b060f, 0xc0b27fc8, 0x5ee9f981, 0xfd21733a,
            0x9b58ecf3, 0x399066ac, 0xd7c7e065, 0x75ff5a1e, 0x1436d3d7, 0xb26e4d90, 0x50a5c749, 0xeedd4102,
            0x8d14babb, 0x2b4c3474, 0xc983ae2d, 0x67bb27e6, 0x05f2a19f, 0xa42a1b58, 0x42619511, 0xe0990eca,
            0x7ed08883, 0x1d08023c, 0xbb3f7bf5, 0x5976f5ae, 0xf7ae6f67, 0x95e5e920, 0x341d62d9, 0xd254dc92,
            0x708c564b, 0x0ec3d004, 0xacfb49bd, 0x4b32c376, 0xe96a3d2f, 0x87a1b6e8, 0x25d930a1, 0xc410aa5a,
            0x62482413, 0x007f9dcc
        };
    }

    // #define BSWAP(x)  ( ((x>>24)&0x000000FFUL) | ((x<<24)&0xFF000000UL)  | \
    //                     ((x>>8)&0x0000FF00UL)  | ((x<<8)&0x00FF0000UL) )

    // Initialize the LTC_RC5 block cipher
    // @param key The symmetric key you wish to pass
    // @param keylen The key length in bytes
    // @param num_rounds The number of rounds desired (0 for default)
    // @param skey The key in as scheduled by this function.
    // @return CRYPT_OK if successful
    private static int rc5_setup(final byte[] key, int keylen, int num_rounds, int[] skey_K)
    {
        int[] L = new int[64];
        int[] S;
        int A, B, i, j, v, s, t, l;

        /* test parameters */

        if (num_rounds < 12 || num_rounds > 24) {
           return CRYPT_INVALID_ROUNDS;
        }

        /* key must be between 64 and 1024 bits */
        if (keylen < 8 || keylen > 128) {
           return CRYPT_INVALID_KEYSIZE;
        }

        S = skey_K;

        /* copy the key into the L array */
        for (A = i = j = 0; i < keylen; ) {
            A = (A << 8) | ((key[i++] & 255));
            if ((i & 3) == 0) {
               // L[j++] = BSWAP(A);
               L[j++] = ( ((A>>>24)&0x000000FF) | ((A<<24)&0xFF000000) | ((A>>>8)&0x0000FF00)  | ((A<<8)&0x00FF0000) );
               A = 0;
            }
        }

        if ((keylen & 3) != 0) {
            A <<= ((8 * (4 - (keylen&3))));
            // L[j++] = BSWAP(A);
            L[j++] = ( ((A>>>24)&0x000000FF) | ((A<<24)&0xFF000000) | ((A>>>8)&0x0000FF00)  | ((A<<8)&0x00FF0000) );
        }

        /* setup the S array */
        t = (2 * (num_rounds + 1));
        // XMEMCPY(S, stab, t * sizeof(*S));
        for (byte k=0; k<t; k++) {
            S[k] = stab.data[k];
        }

        /* mix buffer */
        // s = 3 * ( ((t)>(j))?(t):(j) );
        if ((t)>(j)) {
            s = 3 * t;
        } else {
            s = 3 * j;
        }
        l = j;
        for (A = B = i = j = v = 0; v < s; v++) {
            // #define ROL(x, y)  ( (((uint32_t)(x)<<(uint32_t)((y)&31)) | (((uint32_t)(x)&0xFFFFFFFFUL)>>(uint32_t)(32-((y)&31)))) & 0xFFFFFFFFUL)
            // #define ROL(x, y)  ( (x<<(y&31)) | (x>>(32-(y&31))) )
            // A = S[i] = ROLc(S[i] + A + B, 3);
            // B = L[j] = ROL(L[j] + A + B, (A+B));
            int tmp = S[i] + A + B;
            A = S[i] = ( (tmp<<3) | (tmp>>>29) );
            tmp = L[j] + A + B;
            B = L[j] = ( (tmp<<((A+B)&31)) | (tmp>>>(32-((A+B)&31))) );

            if (++i == t) { i = 0; }
            if (++j == l) { j = 0; }
        }
        return CRYPT_OK;
    }

    @Lightweight
    private static int ROL(int x, int y) {
        return ( (x<<(y&31)) | (x>>>(32-(y&31))) );
    } 
    @Lightweight
    private static int LOAD32L(byte[] y, short offset) {
        return ((int)(y[(short)(offset+3)] & 255)<<24) | ((int)(y[(short)(offset+2)] & 255)<<16) | ((int)(y[(short)(offset+1)] & 255)<<8) | ((int)(y[(short)(offset+0)] & 255));
    }

    @Lightweight
    private static void STORE32L(int x, byte[] y, short offset) {
        y[offset++] = (byte)((x)&255);
        y[offset++] = (byte)(((x)>>>8)&255);
        y[offset++] = (byte)(((x)>>>16)&255);
        y[offset] = (byte)(((x)>>>24)&255);
    }

    // Encrypts a block of text with LTC_RC5
    // @param pt The input plaintext (8 bytes)
    // @param ct The output ciphertext (8 bytes)
    // @param skey The key as scheduled
    // @return CRYPT_OK if successful
    private static int rc5_ecb_encrypt(final byte[] pt, byte[] ct, int skey_rounds, int[] skey_K)
    {
        return rtcbenchmark_measure_java_performance(pt, ct, skey_rounds, skey_K);
    }
    public static int rtcbenchmark_measure_java_performance(final byte[] pt, byte[] ct, int skey_rounds, int[] skey_K)
    {
        // uint32_t A, B, *K;
        // K used to be a pointer to somewhere in skey->K.
        // But since Java doesn't allow the same pointer trick, I'll use K as an offset to add when indexing skey->K instead.
        int A, B;
        int r;
        short K;

        for (byte i=0; i<100; i++) {
        A = LOAD32L(pt, (short)0);
        B = LOAD32L(pt, (short)4);

        A += skey_K[(short)0];
        B += skey_K[(short)1];
        // K  = skey_K + 2;
        K  = 2;

        if ((skey_rounds & 1) == 0) {
            for (r = 0; r < skey_rounds; r += 2) {
                A = ROL(A ^ B, B) + skey_K[(short)(K+0)];
                B = ROL(B ^ A, A) + skey_K[(short)(K+1)];
                A = ROL(A ^ B, B) + skey_K[(short)(K+2)];
                B = ROL(B ^ A, A) + skey_K[(short)(K+3)];
                // A = ( ((A ^ B)<<(B&31)) | ((A ^ B)>>>(32-(B&31))) ) + skey_K[(short)(K+0)];
                // B = ( ((B ^ A)<<(A&31)) | ((B ^ A)>>>(32-(A&31))) ) + skey_K[(short)(K+1)];
                // A = ( ((A ^ B)<<(B&31)) | ((A ^ B)>>>(32-(B&31))) ) + skey_K[(short)(K+2)];
                // B = ( ((B ^ A)<<(A&31)) | ((B ^ A)>>>(32-(A&31))) ) + skey_K[(short)(K+3)];
                K += 4;
            }
            K++; // This doesn't do anything but is just here to make sure the point of exit for the loop is here, so we can optimise it with a MARKLOOP.
            // Without this statement, the branch that terminates by branching over the else clause, which is more efficient, but we can't handle that case yet.
            // We should improve the infuser to automatically transform the code so that there's a single point of exit, but for now this will do as well.
        } else {
            for (r = 0; r < skey_rounds; r++) {
                A = ROL(A ^ B, B) + skey_K[(short)(K+0)];;
                B = ROL(B ^ A, A) + skey_K[(short)(K+1)];;
                // A = ( ((A ^ B)<<(B&31)) | ((A ^ B)>>>(32-(B&31))) ) + skey_K[(short)(K+0)];
                // B = ( ((B ^ A)<<(A&31)) | ((B ^ A)>>>(32-(A&31))) ) + skey_K[(short)(K+1)];
                K += 2;
            }
        }
        STORE32L(A, ct, (short)0);
        STORE32L(B, ct, (short)4);
        }

        return CRYPT_OK;
    }

    // // Decrypts a block of text with LTC_RC5
    // // @param ct The input ciphertext (8 bytes)
    // // @param pt The output plaintext (8 bytes)
    // // @param skey The key as scheduled
    // // @return CRYPT_OK if successful
    // private static int rc5_ecb_decrypt(final byte[] ct, byte[] pt, int skey_rounds, int[] skey_K)
    // {
    //     int A, B;
    //     int r, K;
    //     // LTC_ARGCHK(skey != NULL);
    //     // LTC_ARGCHK(pt   != NULL);
    //     // LTC_ARGCHK(ct   != NULL);

    //     // LOAD32L(A, &ct[0]);
    //     // LOAD32L(B, &ct[4]);
    //     A = ((int)(ct[3] & 255)<<24) | ((int)(ct[2] & 255)<<16) | ((int)(ct[1] & 255)<<8) | ((int)(ct[0] & 255));
    //     B = ((int)(ct[4+3] & 255)<<24) | ((int)(ct[4+2] & 255)<<16) | ((int)(ct[4+1] & 255)<<8) | ((int)(ct[4+0] & 255));


    //     // K = skey_K + (skey_rounds << 1);
    //     K = skey_rounds << 1;

    //     if ((skey_rounds & 1) == 0) {
    //         K -= 2;
    //         for (r = skey_rounds - 1; r >= 0; r -= 2) {
    //             // #define ROR(x, y)  ( (x>>(y&31)) | (x<<(32-(y&31))) )
    //             B = ( ((B - skey_K[K+3])>>>(A&31)) | ((B - skey_K[K+3])<<(32-(A&31))) ) ^ A;
    //             A = ( ((A - skey_K[K+2])>>>(B&31)) | ((A - skey_K[K+2])<<(32-(B&31))) ) ^ B;
    //             B = ( ((B - skey_K[K+1])>>>(A&31)) | ((B - skey_K[K+1])<<(32-(A&31))) ) ^ A;
    //             A = ( ((A - skey_K[K+0])>>>(B&31)) | ((A - skey_K[K+0])<<(32-(B&31))) ) ^ B;
    //             K -= 4;
    //         }
    //     } else {
    //         for (r = skey_rounds - 1; r >= 0; r--) {
    //             B = ( ((B - skey_K[K+1])>>>(A&31)) | ((B - skey_K[K+1])<<(32-(A&31))) ) ^ A;
    //             A = ( ((A - skey_K[K+0])>>>(B&31)) | ((A - skey_K[K+0])<<(32-(B&31))) ) ^ B;
    //             K -= 2;
    //         }
    //     }
    //     A -= skey_K[0];
    //     B -= skey_K[1];

    //     // STORE32L(A, &pt[0]);
    //     // STORE32L(B, &pt[4]);
    //     pt[3] = (byte)(((A)>>>24)&255); pt[2] = (byte)(((A)>>>16)&255); pt[1] = (byte)(((A)>>>8)&255); pt[0] = (byte)((A)&255);
    //     pt[4+3] = (byte)(((B)>>>24)&255); pt[4+2] = (byte)(((B)>>>16)&255); pt[4+1] = (byte)(((B)>>>8)&255); pt[4+0] = (byte)((B)&255);

    //     return CRYPT_OK;
    // }
}
