#ifdef AOT_SAFETY_CHECKS

#include <stdint.h>
#include "program_mem.h"
#include "opcodes.h"
#include "parse_infusion.h"
#include "global_id.h"
#include "rtc.h"
#include "rtc_safetychecks_opcodes.h"

#define RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE ((14 << 4) + 14)

#define RTC_CONSUMES_I_R_AND_PRODUCES_I_R(cons_i, cons_r, prod_i, prod_r) ((RTC_STACK_EFFECT_ENCODE_I_R(cons_i, cons_r) << 4) + (RTC_STACK_EFFECT_ENCODE_I_R(prod_i, prod_r)))


const DJ_PROGMEM uint8_t rtc_safety_stack_effect_per_opcode[] = {

RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 0 nop                            : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 1 sconst_m1                      : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 2 sconst_0                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 3 sconst_1                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 4 sconst_2                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 5 sconst_3                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 6 sconst_4                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 7 sconst_5                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 8 iconst_m1                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 9 iconst_0                       : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 10 iconst_1                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 11 iconst_2                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 12 iconst_3                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 13 iconst_4                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 14 iconst_5                      : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 15 aconst_null                   : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 16 bspush                        : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 17 bipush                        : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 18 sspush                        : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 19 sipush                        : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 20 iipush                        : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 21 lds                           : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 22 sload                         : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 23 sload_0                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 24 sload_1                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 25 sload_2                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 26 sload_3                       : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 27 iload                         : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 28 iload_0                       : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 29 iload_1                       : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 30 iload_2                       : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 31 iload_3                       : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 32 aload                         : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 33 aload_0                       : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 34 aload_1                       : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 35 aload_2                       : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 36 aload_3                       : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 37 sstore                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 38 sstore_0                      : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 39 sstore_1                      : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 40 sstore_2                      : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 41 sstore_3                      : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 42 istore                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 43 istore_0                      : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 44 istore_1                      : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 45 istore_2                      : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 46 istore_3                      : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 47 astore                        : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 48 astore_0                      : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 49 astore_1                      : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 50 astore_2                      : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 51 astore_3                      : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 1 , 0 ), // opcode 52 baload                        : consumes( 1 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 1 , 0 ), // opcode 53 caload                        : consumes( 1 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 1 , 0 ), // opcode 54 saload                        : consumes( 1 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 2 , 0 ), // opcode 55 iaload                        : consumes( 1 , 1 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 0 , 1 ), // opcode 56 aaload                        : consumes( 1 , 1 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 1 , 0 , 0 ), // opcode 57 bastore                       : consumes( 2 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 1 , 0 , 0 ), // opcode 58 castore                       : consumes( 2 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 1 , 0 , 0 ), // opcode 59 sastore                       : consumes( 2 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 3 , 1 , 0 , 0 ), // opcode 60 iastore                       : consumes( 3 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 2 , 0 , 0 ), // opcode 61 aastore                       : consumes( 1 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 62 ipop                          : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 63 ipop2                         : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 2 , 0 ), // opcode 64 idup                          : consumes( 1 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 4 , 0 ), // opcode 65 idup2                         : consumes( 2 , 0 ) produces ( 4 , 0 )
                                                 0, // opcode 66 idup_x1                       : not implemented
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 67 iswap_x                       : SPECIAL
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 68 apop                          : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 2 , 0 , 0 ), // opcode 69 apop2                         : consumes( 0 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 2 ), // opcode 70 adup                          : consumes( 0 , 1 ) produces ( 0 , 2 )
                                                 0, // opcode 71 adup2                         : not implemented
                                                 0, // opcode 72 adup_x1                       : not implemented
                                                 0, // opcode 73 adup_x2                       : not implemented
                                                 0, // opcode 74 aswap                         : not implemented
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 1 , 0 ), // opcode 75 getfield_b                    : consumes( 0 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 1 , 0 ), // opcode 76 getfield_c                    : consumes( 0 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 1 , 0 ), // opcode 77 getfield_s                    : consumes( 0 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 2 , 0 ), // opcode 78 getfield_i                    : consumes( 0 , 1 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 1 ), // opcode 79 getfield_a                    : consumes( 0 , 1 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 0 , 0 ), // opcode 80 putfield_b                    : consumes( 1 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 0 , 0 ), // opcode 81 putfield_c                    : consumes( 1 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 1 , 0 , 0 ), // opcode 82 putfield_s                    : consumes( 1 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 1 , 0 , 0 ), // opcode 83 putfield_i                    : consumes( 2 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 2 , 0 , 0 ), // opcode 84 putfield_a                    : consumes( 0 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 85 getstatic_b                   : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 86 getstatic_c                   : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 87 getstatic_s                   : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 88 getstatic_i                   : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 89 getstatic_a                   : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 90 putstatic_b                   : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 91 putstatic_c                   : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 92 putstatic_s                   : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 93 putstatic_i                   : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 94 putstatic_a                   : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 95 sadd                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 96 ssub                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 97 smul                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 98 sdiv                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 99 srem                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 100 sneg                         : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 101 sshl                         : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 102 sshr                         : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 103 sushr                        : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 104 sand                         : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 105 sor                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 106 sxor                         : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 107 iadd                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 108 isub                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 109 imul                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 110 idiv                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 111 irem                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 2 , 0 ), // opcode 112 ineg                         : consumes( 2 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 3 , 0 , 2 , 0 ), // opcode 113 ishl                         : consumes( 3 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 3 , 0 , 2 , 0 ), // opcode 114 ishr                         : consumes( 3 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 3 , 0 , 2 , 0 ), // opcode 115 iushr                        : consumes( 3 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 116 iand                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 117 ior                          : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 2 , 0 ), // opcode 118 ixor                         : consumes( 4 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 119 binc                         : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 120 sinc                         : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 121 iinc                         : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 122 s2b                          : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 2 , 0 ), // opcode 123 s2i                          : consumes( 1 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 124 i2b                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 125 i2s                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 126 iifeq                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 127 iifne                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 128 iiflt                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 129 iifge                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 130 iifgt                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 131 iifle                        : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 132 ifnull                       : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 133 ifnonnull                    : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 134 if_scmpeq                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 135 if_scmpne                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 136 if_scmplt                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 137 if_scmpge                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 138 if_scmpgt                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 139 if_scmple                    : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 140 if_icmpeq                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 141 if_icmpne                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 142 if_icmplt                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 143 if_icmpge                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 144 if_icmpgt                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 4 , 0 , 0 , 0 ), // opcode 145 if_icmple                    : consumes( 4 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 2 , 0 , 0 ), // opcode 146 if_acmpeq                    : consumes( 0 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 2 , 0 , 0 ), // opcode 147 if_acmpne                    : consumes( 0 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 148 goto                         : consumes( 0 , 0 ) produces ( 0 , 0 )
                                                 0, // opcode 149 goto_w                       : not implemented
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 150 tableswitch                  : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 151 lookupswitch                 : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 152 sreturn                      : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 0 , 0 ), // opcode 153 ireturn                      : consumes( 2 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 154 areturn                      : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 155 return                       : consumes( 0 , 0 ) produces ( 0 , 0 )
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 156 invokevirtual                : SPECIAL
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 157 invokespecial                : SPECIAL
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 158 invokestatic                 : SPECIAL
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 159 invokeinterface              : SPECIAL
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 160 new                          : consumes( 0 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 1 ), // opcode 161 newarray                     : consumes( 1 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 1 ), // opcode 162 anewarray                    : consumes( 1 , 0 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 1 , 0 ), // opcode 163 arraylength                  : consumes( 0 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 164 athrow                       : not implemented
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 1 ), // opcode 165 checkcast                    : consumes( 0 , 1 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 1 , 0 ), // opcode 166 instanceof                   : consumes( 0 , 1 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 167 monitorenter                 : consumes( 0 , 1 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 0 ), // opcode 168 monitorexit                  : consumes( 0 , 1 ) produces ( 0 , 0 )
                                                 0, // opcode 169 idup_x2                      : not implemented
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 170 iinc_w                       : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 171 sinc_w                       : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 1 , 0 ), // opcode 172 i2c                          : consumes( 2 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 173 s2c                          : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 174 b2c                          : consumes( 1 , 0 ) produces ( 1 , 0 )
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 175 idup_x                       : SPECIAL
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 176 sifeq                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 177 sifne                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 178 siflt                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 179 sifge                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 180 sifgt                        : consumes( 1 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 0 , 0 ), // opcode 181 sifle                        : consumes( 1 , 0 ) produces ( 0 , 0 )
                                                 0, // opcode 182 lconst_0                     : not implemented
                                                 0, // opcode 183 lconst_1                     : not implemented
                                                 0, // opcode 184 lload                        : not implemented
                                                 0, // opcode 185 lload_0                      : not implemented
                                                 0, // opcode 186 lload_1                      : not implemented
                                                 0, // opcode 187 lload_2                      : not implemented
                                                 0, // opcode 188 lload_3                      : not implemented
                                                 0, // opcode 189 llpush                       : not implemented
                                                 0, // opcode 190 lstore                       : not implemented
                                                 0, // opcode 191 lstore_0                     : not implemented
                                                 0, // opcode 192 lstore_1                     : not implemented
                                                 0, // opcode 193 lstore_2                     : not implemented
                                                 0, // opcode 194 lstore_3                     : not implemented
                                                 0, // opcode 195 laload                       : not implemented
                                                 0, // opcode 196 lastore                      : not implemented
                                                 0, // opcode 197 getfield_l                   : not implemented
                                                 0, // opcode 198 putfield_l                   : not implemented
                                                 0, // opcode 199 getstatic_l                  : not implemented
                                                 0, // opcode 200 putstatic_l                  : not implemented
                                                 0, // opcode 201 ladd                         : not implemented
                                                 0, // opcode 202 lsub                         : not implemented
                                                 0, // opcode 203 lmul                         : not implemented
                                                 0, // opcode 204 ldiv                         : not implemented
                                                 0, // opcode 205 lrem                         : not implemented
                                                 0, // opcode 206 lneg                         : not implemented
                                                 0, // opcode 207 lshl                         : not implemented
                                                 0, // opcode 208 lshr                         : not implemented
                                                 0, // opcode 209 lushr                        : not implemented
                                                 0, // opcode 210 land                         : not implemented
                                                 0, // opcode 211 lor                          : not implemented
                                                 0, // opcode 212 lxor                         : not implemented
                                                 0, // opcode 213 lreturn                      : not implemented
                                                 0, // opcode 214 l2i                          : not implemented
                                                 0, // opcode 215 l2s                          : not implemented
                                                 0, // opcode 216 i2l                          : not implemented
                                                 0, // opcode 217 s2l                          : not implemented
                                                 0, // opcode 218 lcmp                         : not implemented
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 219 brtarget                     : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 220 markloop_start               : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 0 ), // opcode 221 markloop_end                 : consumes( 0 , 0 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 1 , 0 , 1 ), // opcode 222 getfield_a_fixed             : consumes( 0 , 1 ) produces ( 0 , 1 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 2 , 0 , 0 ), // opcode 223 putfield_a_fixed             : consumes( 0 , 2 ) produces ( 0 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 2 , 0 ), // opcode 224 simul                        : consumes( 2 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 225 lightweightmethodparameter_b : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 226 lightweightmethodparameter_c : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 1 , 0 ), // opcode 227 lightweightmethodparameter_s : consumes( 0 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 2 , 0 ), // opcode 228 lightweightmethodparameter_i : consumes( 0 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 0 , 0 , 0 , 1 ), // opcode 229 lightweightmethodparameter_a : consumes( 0 , 0 ) produces ( 0 , 1 )
             RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE, // opcode 230 invokelight                  : SPECIAL
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 231 sshl_const                   : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 232 sshr_const                   : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 1 , 0 , 1 , 0 ), // opcode 233 sushr_const                  : consumes( 1 , 0 ) produces ( 1 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 2 , 0 ), // opcode 234 ishl_const                   : consumes( 2 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 2 , 0 ), // opcode 235 ishr_const                   : consumes( 2 , 0 ) produces ( 2 , 0 )
RTC_CONSUMES_I_R_AND_PRODUCES_I_R( 2 , 0 , 2 , 0 ), // opcode 236 iushr_const                  : consumes( 2 , 0 ) produces ( 2 , 0 )
                                                 0, // opcode 237                              : not used
                                                 0, // opcode 238                              : not used
                                                 0, // opcode 239                              : not used
                                                 0, // opcode 240                              : not used
                                                 0, // opcode 241                              : not used
                                                 0, // opcode 242                              : not used
                                                 0, // opcode 243                              : not used
                                                 0, // opcode 244                              : not used
                                                 0, // opcode 245                              : not used
                                                 0, // opcode 246                              : not used
                                                 0, // opcode 247                              : not used
                                                 0, // opcode 248                              : not used
                                                 0, // opcode 249                              : not used
                                                 0, // opcode 250                              : not used
                                                 0, // opcode 251                              : not used
                                                 0, // opcode 252                              : not used
                                                 0, // opcode 253                              : not used
                                                 0, // opcode 254                              : not used
                                                 0, // opcode 255                              : not used
};

const DJ_PROGMEM uint8_t rtc_safety_stack_decode_i[] = {
	0,
	0,
	0,
	1,
	1,
	1,
	2,
	2,
	3,
	3,
	4,
	14,
};

const DJ_PROGMEM uint8_t rtc_safety_stack_decode_r[] = {
	0,
	1,
	2,
	0,
	1,
	2,
	0,
	1,
	0,
	1,
	0,
	14,
};

// This could be smaller, but for now I'd prefer to keep the encoding/decoding tables as #defines
uint8_t rtc_safety_get_stack_effect(uint8_t opcode, uint8_t what) {
    if (opcode == JVM_ISWAP_X) {
        switch (what) {
            case RTC_STACK_CONS_REF:
            case RTC_STACK_PROD_REF:
                return 0;
            case RTC_STACK_CONS_INT:
            case RTC_STACK_PROD_INT:
                return (rtc_ts->jvm_operand_byte0 & 15) + (rtc_ts->jvm_operand_byte0 >> 4); // n and m stored in high and low nibbles, but only values 1 and 2 are supported
        }
    } else if (opcode == JVM_IDUP_X) {
        switch (what) {
            case RTC_STACK_CONS_REF:
            case RTC_STACK_PROD_REF:
                return 0;
            case RTC_STACK_CONS_INT:
                return (rtc_ts->jvm_operand_byte0 & 15);
            case RTC_STACK_PROD_INT:
                return (rtc_ts->jvm_operand_byte0 & 15) + (rtc_ts->jvm_operand_byte0 >> 4);
        }
    } else if (opcode == JVM_INVOKEVIRTUAL || opcode == JVM_INVOKEINTERFACE || opcode == JVM_INVOKESTATIC || opcode == JVM_INVOKESPECIAL || opcode == JVM_INVOKELIGHT) {
        dj_local_id localId;
        localId.infusion_id = rtc_ts->jvm_operand_byte0;
        localId.entity_id = rtc_ts->jvm_operand_byte1;

        dj_global_id globalId = dj_global_id_resolve(rtc_ts->infusion,  localId);
        if (opcode == JVM_INVOKEVIRTUAL || opcode == JVM_INVOKEINTERFACE) {
            // The ID in the opcode is the id of a method DEFINITION. Replace it with any IMPLEMENTATION we can find.
            // At runtime, we will check each invoke resolves to an implementation with the same signature.
            globalId = dj_global_id_lookupAnyVirtualMethod(globalId);
        }

        dj_di_pointer methodImpl = dj_global_id_getMethodImplementation(globalId);

        switch (what) {
            case RTC_STACK_CONS_INT:
                return dj_di_methodImplementation_getIntegerArgumentCount(methodImpl);
            case RTC_STACK_CONS_REF:
                return dj_di_methodImplementation_getReferenceArgumentCount(methodImpl)
                        + (opcode == JVM_INVOKESPECIAL || opcode == JVM_INVOKEINTERFACE || opcode == JVM_INVOKEVIRTUAL ? 1 : 0); // Add +1 for instance reference
            case RTC_STACK_PROD_INT:
                switch (dj_di_methodImplementation_getReturnType(methodImpl)) {
                    case JTID_BOOLEAN:
                    case JTID_CHAR:
                    case JTID_BYTE:
                    case JTID_SHORT:
                        return 1;
                    case JTID_INT:
                        return 2;
                    default:
                        return 0;
                }            
            case RTC_STACK_PROD_REF:
                return dj_di_methodImplementation_getReturnType(methodImpl) == JTID_REF ? 1 : 0;
        }
    } else {
        uint8_t encoded_cons = rtc_safety_stack_effect_per_opcode[opcode] >> 4;
        uint8_t encoded_prod = rtc_safety_stack_effect_per_opcode[opcode] & 0x0F;

        #define RTC_STACK_EFFECT_REF_MASK  1
        uint8_t encoded = (what & RTC_STACK_EFFECT_PROD_MASK) 
        				? encoded_prod
        				: encoded_cons;

        if (what & RTC_STACK_EFFECT_REF_MASK) {
            return rtc_safety_stack_decode_r[encoded];
        } else {
            return rtc_safety_stack_decode_i[encoded];
        }
    }

    return 0;
}

#endif // AOT_SAFETY_CHECKS
