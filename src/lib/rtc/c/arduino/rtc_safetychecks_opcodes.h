#ifndef RTC_SAFETYCHECKS_OPCODES_H
#define RTC_SAFETYCHECKS_OPCODES_H

#include <stdint.h>
#include "opcodes.h"

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

#define RTC_STACK_CONS_INT 0
#define RTC_STACK_CONS_REF 1
#define RTC_STACK_PROD_INT 2
#define RTC_STACK_PROD_REF 3

#define RTC_STACK_EFFECT_REF_MASK  1
#define RTC_STACK_EFFECT_PROD_MASK 2

uint8_t rtc_safety_get_stack_effect(uint8_t opcode, uint8_t what);

#define RTC_OPCODE_IS_RETURN(opcode) ((opcode) == JVM_SRETURN || (opcode) == JVM_IRETURN || (opcode) == JVM_ARETURN || (opcode) == JVM_RETURN)
#define RTC_OPCODE_IS_BRANCH(opcode) ((opcode) == JVM_IIFEQ || (opcode) == JVM_IIFNE || (opcode) == JVM_IIFLT || (opcode) == JVM_IIFGE || (opcode) == JVM_IIFGT || (opcode) == JVM_IIFLE || (opcode) == JVM_IFNULL || (opcode) == JVM_IFNONNULL || (opcode) == JVM_IF_SCMPEQ || (opcode) == JVM_IF_SCMPNE || (opcode) == JVM_IF_SCMPLT || (opcode) == JVM_IF_SCMPGE || (opcode) == JVM_IF_SCMPGT || (opcode) == JVM_IF_SCMPLE || (opcode) == JVM_IF_ICMPEQ || (opcode) == JVM_IF_ICMPNE || (opcode) == JVM_IF_ICMPLT || (opcode) == JVM_IF_ICMPGE || (opcode) == JVM_IF_ICMPGT || (opcode) == JVM_IF_ICMPLE || (opcode) == JVM_IF_ACMPEQ || (opcode) == JVM_IF_ACMPNE || (opcode) == JVM_GOTO || (opcode) == JVM_GOTO_W || (opcode) == JVM_TABLESWITCH || (opcode) == JVM_LOOKUPSWITCH || (opcode) == JVM_SIFEQ || (opcode) == JVM_SIFNE || (opcode) == JVM_SIFLT || (opcode) == JVM_SIFGE || (opcode) == JVM_SIFGT || (opcode) == JVM_SIFLE)
#define RTC_OPCODE_IS_BRTARGET(opcode) ((opcode) == JVM_BRTARGET)

#endif // RTC_SAFETYCHECKS_OPCODES_H
