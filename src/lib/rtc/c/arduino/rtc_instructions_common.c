#include <stdint.h>
#include "opcodes.h"
#include "execution.h"
#include "infusion.h"
#include "panic.h"
#include "program_mem.h"
#include "parse_infusion.h"
#include "asm.h"
#include "rtc.h"
#include "rtc_complex_instructions.h"
#include "rtc_instructions_common.h"
#include "rtc_safetychecks.h"

#ifdef AOT_STRATEGY_SIMPLESTACKCACHE
#include "rtc_simplestackcache.h"
#endif
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE
#include "rtc_poppedstackcache.h"
#endif
#ifdef AOT_STRATEGY_MARKLOOP
#include "rtc_markloop.h"
#endif

void rtc_common_translate_invoke(rtc_translationstate *ts, uint8_t opcode, uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1, uint8_t jvm_operand_byte2) {
#if defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
    emit_flush_to_flash(); // To make sure we won't accidentally optimise addresses or branch labels
#endif

    dj_local_id localId;
    localId.infusion_id = jvm_operand_byte0;
    localId.entity_id = jvm_operand_byte1;

    dj_global_id globalId = dj_global_id_resolve(ts->infusion,  localId);
    uint8_t rettype = JTID_VOID;

    emit_x_preinvoke();

    emit_LDI(R22, ((uint16_t)globalId.infusion) & 0xFF);
    emit_LDI(R23, (((uint16_t)globalId.infusion) >> 8) & 0xFF);
    emit_LDI(R24, globalId.entity_id);

    if (opcode == JVM_INVOKEVIRTUAL || opcode == JVM_INVOKEINTERFACE) {
        emit_LDI(R20, jvm_operand_byte2); // nr_ref_args
        emit_2_CALL((uint16_t)&RTC_INVOKEVIRTUAL_OR_INTERFACE);
    } else { // JVM_INVOKESPECIAL or JVM_INVOKESTATIC
        dj_di_pointer methodImpl = dj_global_id_getMethodImplementation(globalId);
        uint8_t flags = dj_di_methodImplementation_getFlags(methodImpl);

        emit_LDI(R20, ((uint16_t)methodImpl) & 0xFF);
        emit_LDI(R21, (((uint16_t)methodImpl) >> 8) & 0xFF);

        if ((flags & FLAGS_NATIVE) != 0) {
            emit_2_CALL((uint16_t)&RTC_INVOKESTATIC_FAST_NATIVE);
        } else {
            rettype = dj_di_methodImplementation_getReturnType(methodImpl); // By setting rettype here, the code past postinvoke will push any return value onto the stack (using stack caching if possible). This shouldn't be done for native methods, since they will push the return value onto the stack directly!!!!
            uint16_t frame_size = dj_frame_size(methodImpl);
            emit_LDI(R18, frame_size & 0xFF);
            emit_LDI(R19, (frame_size >> 8) & 0xFF);
            emit_LDI(R25, flags);
            emit_2_CALL((uint16_t)&RTC_INVOKESPECIAL_OR_STATIC_FAST_JAVA);
        }
    }

    emit_x_postinvoke();

    rtc_common_push_returnvalue_from_R22_if_necessary(rettype);
}

#ifndef NO_LIGHTWEIGHT_METHODS
void rtc_common_translate_invokelight(uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1) {
#if defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
    emit_flush_to_flash(); // To make sure we won't accidentally optimise addresses or branch labels
#endif

    dj_local_id localId;
    localId.infusion_id = jvm_operand_byte0;
    localId.entity_id = jvm_operand_byte1;

    if (localId.infusion_id != 0) {
        // We don't (yet) support calling lightweight methods in other infusions.
        // At least, R2 may have to be updated, and we'll have to save the used
        // call saved registers per method in flash.
        // But there may be other consequences, so have another good look at this
        // when we need it.
        dj_panic(DJ_PANIC_LIGHTWEIGHT_METHOD_MUST_BE_LOCAL);
    }

    rtc_current_method_set_uses_reg(R16);
    rtc_current_method_set_uses_reg_used_in_lightweight_invoke(localId.entity_id);

#if defined (AOT_STRATEGY_MARKLOOP)
    rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_ALL); // Maybe this could be more selective using the information from the compiled method. But I doubt that will make a big difference, and may not be worth the code space to implement.

    // If the invoke light happens in a MARKLOOP loop, the lightweight method might corrupt some pinned values, so we need to save them back to their variable slots first.
    // If we're not in a MARKLOOP loop, this will be a noop.
    rtc_markloop_emit_epilogue(true, localId.entity_id);
#elif defined (AOT_STRATEGY_POPPEDSTACKCACHE)
    rtc_stackcache_flush_all_regs();
    rtc_poppedstackcache_clear_all_valuetags();
#elif defined (AOT_STRATEGY_SIMPLESTACKCACHE)
    rtc_stackcache_flush_all_regs();
#elif defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
#else
    this should not happen
#endif

    dj_global_id globalId = dj_global_id_resolve(rtc_ts->infusion,  localId);
    native_method_function_t handler = rtc_ts->method_start_addresses[globalId.entity_id]; // Can't get the address from the infusion because the method addresses haven't been written to Flash yet
    if (handler == NULL) {
        dj_panic(DJ_PANIC_NO_ADDRESS_FOUND_FOR_LIGHTWEIGHT_METHOD);
    }

    dj_di_pointer callerMethodImpl = rtc_ts->methodimpl;
    dj_di_pointer calleeMethodImpl = dj_global_id_getMethodImplementation(globalId);

#ifdef AOT_SAFETY_CHECKS
    // A lightweight method will reuse the caller's stackframe. We need to check if enough space has been reserved in the
    // current method's frame to accomodate the callee's:
    //   - local variables
    //   - reference stack
    //   - integer stack
    // Note that the callee's max stack and total local variables will include space for any nested lightweight methods calls,
    // so these will have been checked in the callee's safety checks already.
    uint8_t calleeRefArgs = dj_di_methodImplementation_getReferenceArgumentCount(calleeMethodImpl);
    uint8_t calleeIntArgs = dj_di_methodImplementation_getIntegerArgumentCount(calleeMethodImpl);

    uint8_t spaceOnRefStack = dj_di_methodImplementation_getMaxRefStack(callerMethodImpl) - rtc_ts->pre_instruction_ref_stack + calleeRefArgs;
    uint8_t spaceOnIntStack = dj_di_methodImplementation_getMaxIntStack(callerMethodImpl) - rtc_ts->pre_instruction_int_stack + calleeIntArgs;
    uint8_t spaceForLocalVariables = dj_di_methodImplementation_getNumberOfTotalVariableSlots(callerMethodImpl) - dj_di_methodImplementation_getNumberOfOwnVariableSlots(callerMethodImpl);

    uint8_t calleeMaxRefStack = dj_di_methodImplementation_getMaxRefStack(calleeMethodImpl);
    uint8_t calleeMaxIntStack = dj_di_methodImplementation_getMaxIntStack(calleeMethodImpl);
    uint8_t calleeLocalVariables = dj_di_methodImplementation_getNumberOfTotalVariableSlots(calleeMethodImpl);
    uint8_t calleeReservedSpaceForReturnValue = (dj_di_methodImplementation_getFlags(calleeMethodImpl) & FLAGS_USES_SIMUL_INVOKESTATIC_MARKLOOP) ? 1 : 0;

    if ((calleeMaxRefStack > spaceOnRefStack)
            || (calleeMaxIntStack > spaceOnIntStack)
            || ((calleeLocalVariables + calleeReservedSpaceForReturnValue) > spaceForLocalVariables)) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_NOT_ENOUGH_SPACE_IN_FRAME_FOR_LW_CALL);
    }
#endif // AOT_SAFETY_CHECKS

    uint8_t rettype = dj_di_methodImplementation_getReturnType(calleeMethodImpl);

    bool lightweightMethodUsesLocalVariables = (dj_di_methodImplementation_getNumberOfTotalVariableSlots(calleeMethodImpl) > 0);
    uint16_t bytesForCurrentMethodsOwnLocals = 2*(dj_di_methodImplementation_getNumberOfOwnVariableSlots(callerMethodImpl));
    if (lightweightMethodUsesLocalVariables) {
        // Target lightweight method uses local variables. It will use the extra space
        // reserved in the current method's frame for such lightweight methods.
        // We need to move the Y pointer, which points to the current method's locals
        // to point to the reserved space, so the lightweight method can index locals
        // as usual. (see notes.txt)
        uint16_t bytesForCurrentMethodsOwnLocalsCopy = bytesForCurrentMethodsOwnLocals; // Make a copy bef
        bytesForCurrentMethodsOwnLocalsCopy = emit_ADIW_if_necessary_to_bring_offset_in_range(RY, bytesForCurrentMethodsOwnLocalsCopy);
        emit_ADIW(RY, bytesForCurrentMethodsOwnLocalsCopy);
    }

    emit_2_CALL((uint16_t)handler);

    if (lightweightMethodUsesLocalVariables) {
        // If we moved Y forward before, move it back now. (we can't push/pop here, since there may be operands on the stack)
        bytesForCurrentMethodsOwnLocals = emit_SBIW_if_necessary_to_bring_offset_in_range(RY, bytesForCurrentMethodsOwnLocals);
        emit_SBIW(RY, bytesForCurrentMethodsOwnLocals);
    }

    rtc_common_push_returnvalue_from_R22_if_necessary(rettype);

#if defined (AOT_STRATEGY_MARKLOOP)
    // If the invoke light happens in a MARKLOOP loop, the lightweight method might corrupt some pinned values, so we need to save them back to their variable slots first.
    // If we're not in a MARKLOOP loop, this will be a noop.
    rtc_markloop_emit_prologue(true, localId.entity_id);    
#endif
}
#endif

void rtc_common_translate_inc(uint8_t opcode, uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1, uint8_t jvm_operand_byte2) {
    // -129 -> JVM_S/IINC_W
    // -128 -> JVM_S/IINC
    // +127 -> JVM_S/IINC
    // +128 -> JVM_S/IINC_W
    // jvm_operand_byte0: index of int local
    int16_t jvm_operand_signed_word;
    uint8_t operand_reg_load_into;
    uint8_t operand_reg_pointer_to_locals;
    uint8_t offset;
    if (opcode == JVM_SINC || opcode == JVM_IINC) {
        jvm_operand_signed_word = (int8_t)jvm_operand_byte1;
    } else {
        jvm_operand_signed_word = (int16_t)(((uint16_t)jvm_operand_byte1 << 8) + jvm_operand_byte2);
    }
    bool is_iinc;
    if (opcode == JVM_IINC || opcode == JVM_IINC_W) {
        is_iinc = true;
        offset = offset_for_intlocal_int(rtc_ts->methodimpl, jvm_operand_byte0);
#ifdef AOT_SAFETY_CHECKS
        rtc_safety_check_offset_valid_for_local_variable(offset + 3); // +3 because we will write four bytes at this offset and both need to fit in the space reserved for local variables.
#endif //AOT_SAFETY_CHECKS
    } else {
        is_iinc = false;
        offset = offset_for_intlocal_short(rtc_ts->methodimpl, jvm_operand_byte0);
#ifdef AOT_SAFETY_CHECKS
        rtc_safety_check_offset_valid_for_local_variable(offset + 1); // +1 because we will write four bytes at this offset and both need to fit in the space reserved for local variables.
#endif //AOT_SAFETY_CHECKS
    }

    if (asm_needs_ADIW_to_bring_offset_in_range(offset)) {
        // Offset too large: copy Z to Y and ADIW it until we can reach the desired offset
        emit_MOVW(RZ, RY);
        offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
        operand_reg_pointer_to_locals = Z;
        if (jvm_operand_signed_word == 1) {
            operand_reg_load_into = R0; // The register to load the local into, one byte at a time.
        } else {
            // We will use SUBI/SBCI instead of INC to increment by more than 1, but we can't use
            // R0 for that since these instructions need a register >= R16. Temporarily save R16 on
            // the stack and use that instead.
            emit_PUSH(R16);
            operand_reg_load_into = R16; // The register to load the local into, one byte at a time.
        }
    } else {
        // Offset in range: just use Y directly
        operand_reg_pointer_to_locals = Y;
        operand_reg_load_into = RZL; // The register to load the local into, one byte at a time.
    }
    if (jvm_operand_signed_word == 1) {
        // Special case
        emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset);
        emit_INC(operand_reg_load_into);
        emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset);
        emit_BRNE(is_iinc ? 22 : 6); // For short inc we just need to jump 6, but for int we will generate more code below, so we need to jump farther
        emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+1);
        emit_INC(operand_reg_load_into);
        emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+1);
        if (is_iinc) {
            emit_BRNE(14);
            emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+2);
            emit_INC(operand_reg_load_into);
            emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+2);
            emit_BRNE(6);
            emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+3);
            emit_INC(operand_reg_load_into);
            emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+3);
        }
    } else {
        uint8_t c0, c1, c2, c3;
        if (jvm_operand_signed_word > 0) {
            // Positive operand
            c0 = -(jvm_operand_signed_word & 0xFF);
            c1 = -((jvm_operand_signed_word >> 8) & 0xFF)-1;
            c2 = -1;
            c3 = -1;
        } else {
            // Negative operand
            c0 = (-jvm_operand_signed_word) & 0xFF;
            c1 = ((-jvm_operand_signed_word) >> 8) & 0xFF;
            c2 = 0;
            c3 = 0;
        }

        emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset);
        emit_SUBI(operand_reg_load_into, c0);
        emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset);

        emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+1);
        emit_SBCI(operand_reg_load_into, c1);
        emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+1);

        if (is_iinc) {
            emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+2);
            emit_SBCI(operand_reg_load_into, c2);
            emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+2);

            emit_LDD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+3);
            emit_SBCI(operand_reg_load_into, c3);
            emit_STD(operand_reg_load_into, operand_reg_pointer_to_locals, offset+3);
        }
    }
    if (operand_reg_load_into == R16) {
        emit_POP(R16);
    }
}

void rtc_common_push_returnvalue_from_R22_if_necessary(uint8_t rettype) {
    // Will be VOID except for INVOKESTATIC calls that return something.
#if defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
    switch (rettype) {
        case JTID_BOOLEAN:
        case JTID_CHAR:
        case JTID_BYTE:
        case JTID_SHORT:
            emit_x_PUSH_16bit(R22);
            break;
        case JTID_INT:
            emit_x_PUSH_32bit(R22);
            break;
        case JTID_REF:
            emit_x_PUSH_REF(R22);
            break;
    }
#else // simple, popped, or markloop
    switch (rettype) {
        case JTID_BOOLEAN:
        case JTID_CHAR:
        case JTID_BYTE:
        case JTID_SHORT:
            rtc_stackcache_push_16bit_from_R22R23();
            break;
        case JTID_INT:
            rtc_stackcache_push_32bit_from_R22R25();
            break;
        case JTID_REF:
            rtc_stackcache_push_ref_from_R22R23();
            break;
    }
#endif    
}

uint16_t get_offset_for_FIELD_A_FIXED(uint8_t infusion_id, uint8_t entity_id, uint16_t ref_index) {
    dj_local_id local_id;
    local_id.infusion_id = infusion_id;
    local_id.entity_id = entity_id;
    dj_global_id global_id = dj_global_id_resolve(rtc_ts->infusion, local_id);
    dj_di_pointer classDef = dj_infusion_getClassDefinition(global_id.infusion, global_id.entity_id);
    uint16_t baseRefOffset = dj_di_classDefinition_getOffsetOfFirstReference(classDef);
    uint16_t targetRefOffset = baseRefOffset + ref_index*2;
    return targetRefOffset;
}
