#include <stdint.h>
#include "opcodes.h"
#include "execution.h"
#include "program_mem.h"
#include "asm.h"
#include "rtc.h"
#include "rtc_complex_instructions.h"

void rtc_common_translate_invoke(rtc_translationstate *ts, uint8_t opcode, uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1, uint8_t jvm_operand_byte2) {
    dj_local_id localId;
    localId.infusion_id = jvm_operand_byte0;
    localId.entity_id = jvm_operand_byte1;

    dj_global_id globalId = dj_global_id_resolve(ts->infusion,  localId);

#ifdef OPTIMISE_Os_INVOKE
    emit_x_preinvoke();

    emit_LDI(R22, ((uint16_t)globalId.infusion) & 0xFF);
    emit_LDI(R23, (((uint16_t)globalId.infusion) >> 8) & 0xFF);
    emit_LDI(R24, globalId.entity_id);

    if        (opcode == JVM_INVOKEVIRTUAL
            || opcode == JVM_INVOKEINTERFACE) {
        emit_LDI(R20, jvm_operand_byte2); // nr_ref_args
        emit_x_CALL((uint16_t)&RTC_INVOKEVIRTUAL_OR_INTERFACE);
    } else if (opcode == JVM_INVOKESPECIAL) {
        emit_x_CALL((uint16_t)&RTC_INVOKESPECIAL);
    } else if (opcode == JVM_INVOKESTATIC) {
        emit_x_CALL((uint16_t)&RTC_INVOKESTATIC);
    }

    emit_x_postinvoke();
#else // OPTIMISE_Os_INVOKE
    // set intStack to SP
    emit_PUSH(ZERO_REG); // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT. 
                                // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.
    emit_2_LDS(R22, SPaddress_L); // Load SP into R22:R23
    emit_2_LDS(R23, SPaddress_H); // Load SP into R22:R23
    emit_2_STS((uint16_t)&(intStack), R22); // Store SP into intStack
    emit_2_STS((uint16_t)&(intStack)+1, R23); // Store SP into intStack

    // Reserve 8 bytes of space on the stack, in case the returned int is large than passed ints
    // TODO: make this more efficient by looking up the method, and seeing if the return type is int,
    //       and if so, if the size of the return type is larger than the integers passed. Then only
    //       reserve the space that's needed.
    //       This is for the worst case, where no ints are passed, so there's no space reserved, and
    //       a 64 bit long is returned.
    emit_RCALL(0); // RCALL to offset 0 does nothing, except reserving 2 bytes on the stack. cheaper than two useless pushes.
    emit_RCALL(0);
    emit_RCALL(0);
    emit_RCALL(0);

    // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
    emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
    emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

    emit_LDI(R22, ((uint16_t)globalId.infusion) & 0xFF);
    emit_LDI(R23, (((uint16_t)globalId.infusion) >> 8) & 0xFF);
    emit_LDI(R24, globalId.entity_id);
    if        (opcode == JVM_INVOKEVIRTUAL
            || opcode == JVM_INVOKEINTERFACE) {
        emit_LDI(R20, jvm_operand_byte2); // nr_ref_args
        emit_x_CALL((uint16_t)&RTC_INVOKEVIRTUAL_OR_INTERFACE);
    } else if (opcode == JVM_INVOKESPECIAL) {
        emit_x_CALL((uint16_t)&RTC_INVOKESPECIAL);
    } else if (opcode == JVM_INVOKESTATIC) {
        emit_x_CALL((uint16_t)&RTC_INVOKESTATIC);
    }

    // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
    emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
    emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
    // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
    emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
    emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


    // get SP from intStack
    emit_2_LDS(R22, (uint16_t)&(intStack)); // Load intStack into R22:R23
    emit_2_LDS(R23, (uint16_t)&(intStack)+1); // Load intStack into R22:R23
    emit_2_STS(SPaddress_L, R22); // Store R22:25 into SP
    emit_2_STS(SPaddress_H, R23); // Store R22:25 into SP
    emit_POP(R23); // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE FIRST LINE FOR INVOKESTATIC. SEE COMMENT ABOVE.
#endif // OPTIMISE_Os_INVOKE
}
