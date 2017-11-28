#ifndef RTC_SAFETYCHECKS_OPCODES_H
#define RTC_SAFETYCHECKS_OPCODES_H

#include <stdint.h>

#define RTC_STACK_EFFECT_ENCODE_I_R(i,r) \
    (                         \
     (i==0 && r==0) ? 0  :    \
     (i==0 && r==1) ? 1  :    \
     (i==0 && r==2) ? 2  :    \
     (i==1 && r==0) ? 3  :    \
     (i==1 && r==1) ? 4  :    \
     (i==1 && r==2) ? 5  :    \
     (i==2 && r==0) ? 6  :    \
     (i==2 && r==1) ? 7  :    \
     (i==3 && r==0) ? 8  :    \
     (i==3 && r==1) ? 9  :    \
     (i==4 && r==0) ? 10 : 15 \
    )

#define RTC_STACK_EFFECT_ENCODED_SPECIAL_CASE ((14 << 4) + 14)

#define RTC_STACK_EFFECT_DECODE_I(encoded) \
    (                           \
     ((encoded)==0)  ? 0  :     \
     ((encoded)==1)  ? 0  :     \
     ((encoded)==2)  ? 0  :     \
     ((encoded)==3)  ? 1  :     \
     ((encoded)==4)  ? 1  :     \
     ((encoded)==5)  ? 1  :     \
     ((encoded)==6)  ? 2  :     \
     ((encoded)==7)  ? 2  :     \
     ((encoded)==8)  ? 3  :     \
     ((encoded)==9)  ? 3  :     \
     ((encoded)==10) ? 4  :     \
     ((encoded)==14) ? 14 : 15  \ // 14: SPECIAL, 15: ERROR
    )

#define RTC_STACK_EFFECT_DECODE_R(encoded) \
    (                           \
     ((encoded)==0)  ? 0  :     \
     ((encoded)==1)  ? 1  :     \
     ((encoded)==2)  ? 2  :     \
     ((encoded)==3)  ? 0  :     \
     ((encoded)==4)  ? 1  :     \
     ((encoded)==5)  ? 2  :     \
     ((encoded)==6)  ? 0  :     \
     ((encoded)==7)  ? 1  :     \
     ((encoded)==8)  ? 0  :     \
     ((encoded)==9)  ? 1  :     \
     ((encoded)==10) ? 4  :     \
     ((encoded)==14) ? 14 : 15  \ // 14: SPECIAL, 15: ERROR
    )

#define RTC_CONSUMES_I_R_AND_PRODUCES_I_R(cons_i, cons_r, prod_i, prod_r) ((RTC_STACK_EFFECT_ENCODE_I_R(cons_i, cons_r) << 4) + (RTC_STACK_EFFECT_ENCODE_I_R(prod_i, prod_r)))

uint8_t rtc_get_stack_effect_cons_i(uint8_t opcode);
uint8_t rtc_get_stack_effect_cons_r(uint8_t opcode);
uint8_t rtc_get_stack_effect_prod_i(uint8_t opcode);
uint8_t rtc_get_stack_effect_prod_r(uint8_t opcode);

#endif // RTC_SAFETYCHECKS_OPCODES_H
