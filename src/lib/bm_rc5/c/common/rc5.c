/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 */

/**
   @file rc5.c
   LTC_RC5 code by Tom St Denis
*/

#include <stdint.h>
#include "config.h"
#include "darjeeling3.h"
#include "rc5.h"
#include <avr/pgmspace.h>
// #include "tomcrypt.h"

const uint32_t stab[50] PROGMEM = {
0xb7e15163UL, 0x5618cb1cUL, 0xf45044d5UL, 0x9287be8eUL, 0x30bf3847UL, 0xcef6b200UL, 0x6d2e2bb9UL, 0x0b65a572UL,
0xa99d1f2bUL, 0x47d498e4UL, 0xe60c129dUL, 0x84438c56UL, 0x227b060fUL, 0xc0b27fc8UL, 0x5ee9f981UL, 0xfd21733aUL,
0x9b58ecf3UL, 0x399066acUL, 0xd7c7e065UL, 0x75ff5a1eUL, 0x1436d3d7UL, 0xb26e4d90UL, 0x50a5c749UL, 0xeedd4102UL,
0x8d14babbUL, 0x2b4c3474UL, 0xc983ae2dUL, 0x67bb27e6UL, 0x05f2a19fUL, 0xa42a1b58UL, 0x42619511UL, 0xe0990ecaUL,
0x7ed08883UL, 0x1d08023cUL, 0xbb3f7bf5UL, 0x5976f5aeUL, 0xf7ae6f67UL, 0x95e5e920UL, 0x341d62d9UL, 0xd254dc92UL,
0x708c564bUL, 0x0ec3d004UL, 0xacfb49bdUL, 0x4b32c376UL, 0xe96a3d2fUL, 0x87a1b6e8UL, 0x25d930a1UL, 0xc410aa5aUL,
0x62482413UL, 0x007f9dccUL
};

 /**
    Initialize the LTC_RC5 block cipher
    @param key The symmetric key you wish to pass
    @param keylen The key length in bytes
    @param num_rounds The number of rounds desired (0 for default)
    @param skey The key in as scheduled by this function.
    @return CRYPT_OK if successful
 */
int rc5_setup(const unsigned char *key, int keylen, int num_rounds, rc5_key *skey)
{
    uint32_t L[64], *S, A, B, i, j, v, s, t, l;

    // LTC_ARGCHK(skey != NULL);
    // LTC_ARGCHK(key  != NULL);

    /* test parameters */
    if (num_rounds == 0) {
       num_rounds = 12;
    }

    if (num_rounds < 12 || num_rounds > 24) {
       return CRYPT_INVALID_ROUNDS;
    }

    /* key must be between 64 and 1024 bits */
    if (keylen < 8 || keylen > 128) {
       return CRYPT_INVALID_KEYSIZE;
    }

    skey->rounds = num_rounds;
    S = skey->K;

    /* copy the key into the L array */
    for (A = i = j = 0; i < (uint32_t)keylen; ) {
        A = (A << 8) | ((uint32_t)(key[i++] & 255));
        if ((i & 3) == 0) {
           L[j++] = BSWAP(A);
           A = 0;
        }
    }

    if ((keylen & 3) != 0) {
        A <<= (uint32_t)((8 * (4 - (keylen&3))));
        L[j++] = BSWAP(A);
    }

    /* setup the S array */
    t = (uint32_t)(2 * (num_rounds + 1));
    // XMEMCPY(S, stab, t * sizeof(*S));
    for (uint8_t k=0; k<t; k++) {
       S[k] = pgm_read_dword(stab + k);
    }

    /* mix buffer */
    s = 3 * MAX(t, j);
    l = j;
    for (A = B = i = j = v = 0; v < s; v++) {
        A = S[i] = ROLc(S[i] + A + B, 3);
        B = L[j] = ROL(L[j] + A + B, (A+B));
        if (++i == t) { i = 0; }
        if (++j == l) { j = 0; }
    }
    return CRYPT_OK;
}

/**
  Encrypts a block of text with LTC_RC5
  @param pt The input plaintext (8 bytes)
  @param ct The output ciphertext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
int __attribute__((noinline)) rtcbenchmark_measure_native_performance(const unsigned char *pt, unsigned char *ct, rc5_key *skey)
{
    javax_darjeeling_Stopwatch_void_resetAndStart();

    uint32_t A, B, *K;
    int r;
    // LTC_ARGCHK(skey != NULL);
    // LTC_ARGCHK(pt   != NULL);
    // LTC_ARGCHK(ct   != NULL);

    for (uint8_t i=0; i<100; i++) {
    LOAD32L(A, &pt[0]);
    LOAD32L(B, &pt[4]);
    A += skey->K[0];
    B += skey->K[1];
    K  = skey->K + 2;

    if ((skey->rounds & 1) == 0) {
      for (r = 0; r < skey->rounds; r += 2) {
          A = ROL(A ^ B, B) + K[0];
          B = ROL(B ^ A, A) + K[1];
          A = ROL(A ^ B, B) + K[2];
          B = ROL(B ^ A, A) + K[3];
          K += 4;
      }
    } else {
      for (r = 0; r < skey->rounds; r++) {
          A = ROL(A ^ B, B) + K[0];
          B = ROL(B ^ A, A) + K[1];
          K += 2;
      }
    }
    STORE32L(A, &ct[0]);
    STORE32L(B, &ct[4]);
    }

    javax_darjeeling_Stopwatch_void_measure();
    return CRYPT_OK;
}
int rc5_ecb_encrypt(const unsigned char *pt, unsigned char *ct, rc5_key *skey)
{
  return rtcbenchmark_measure_native_performance(pt, ct, skey);
}

// /**
//   Decrypts a block of text with LTC_RC5
//   @param ct The input ciphertext (8 bytes)
//   @param pt The output plaintext (8 bytes)
//   @param skey The key as scheduled
//   @return CRYPT_OK if successful
// */
// int rc5_ecb_decrypt(const unsigned char *ct, unsigned char *pt, rc5_key *skey)
// {
//     javax_darjeeling_Stopwatch_void_resetAndStart();

//     uint32_t A, B, *K;
//     int r;
//     // LTC_ARGCHK(skey != NULL);
//     // LTC_ARGCHK(pt   != NULL);
//     // LTC_ARGCHK(ct   != NULL);

//     LOAD32L(A, &ct[0]);
//     LOAD32L(B, &ct[4]);
//     K = skey->K + (skey->rounds << 1);

//     if ((skey->rounds & 1) == 0) {
//        K -= 2;
//        for (r = skey->rounds - 1; r >= 0; r -= 2) {
//           B = ROR(B - K[3], A) ^ A;
//           A = ROR(A - K[2], B) ^ B;
//           B = ROR(B - K[1], A) ^ A;
//           A = ROR(A - K[0], B) ^ B;
//           K -= 4;
//         }
//     } else {
//       for (r = skey->rounds - 1; r >= 0; r--) {
//           B = ROR(B - K[1], A) ^ A;
//           A = ROR(A - K[0], B) ^ B;
//           K -= 2;
//       }
//     }
//     A -= skey->K[0];
//     B -= skey->K[1];
//     STORE32L(A, &pt[0]);
//     STORE32L(B, &pt[4]);

//     javax_darjeeling_Stopwatch_void_measure();
//     return CRYPT_OK;
// }

void javax_rtcbench_RTCBenchmark_void_test_native(void)
{
    uint8_t NUMNUMBERS = 8;
    unsigned char test_key[] = { 0x91, 0x5f, 0x46, 0x19, 0xbe, 0x41, 0xb2, 0x51, 0x63, 0x55, 0xa5, 0x01, 0x10, 0xa9, 0xce, 0x91 };
    unsigned char test_pt[]  = { 0x21, 0xa5, 0xdb, 0xee, 0x15, 0x4b, 0x8f, 0x6d };
    unsigned char test_ct[]  = { 0xf7, 0xc0, 0x13, 0xac, 0x5b, 0x2b, 0x89, 0x52 };

    unsigned char tmp0[NUMNUMBERS];
    // unsigned char tmp1[NUMNUMBERS];

    int err;
    rc5_key key;
    
    /* setup key */
    if ((err = rc5_setup(test_key, 16, 12, &key)) != CRYPT_OK) {
        avroraPrintStr("TEST SETUP FAILED");
        return;
    }

    /* encrypt and decrypt */
    rc5_ecb_encrypt(test_pt, tmp0, &key);
    // rc5_ecb_decrypt(tmp0, tmp1, &key);

    avroraPrintStr("Original:");
    for (uint8_t k=0; k<NUMNUMBERS; k++) {
        avroraPrintInt8(test_pt[k]);
    }
    avroraPrintStr("done.");
    avroraPrintStr("Encrypted:");
    for (uint8_t k=0; k<NUMNUMBERS; k++) {
        avroraPrintInt8(tmp0[k]);
    }
    avroraPrintStr("done.");
    // avroraPrintStr("Decrypted:");
    // for (uint8_t k=0; k<NUMNUMBERS; k++) {
    //     avroraPrintInt8(tmp1[k]);
    // }
    // avroraPrintStr("done.");

    for (uint8_t k=0; k<NUMNUMBERS; k++) {
      if ((tmp0[k] != test_ct[k])) { // || (tmp1[k] != test_pt[k])) {
        avroraPrintStr("TEST FAILED");
        return;
      }
    }
    avroraPrintStr("TEST OK");
}

