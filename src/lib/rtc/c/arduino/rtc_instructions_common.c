#include <stdint.h>
#include "opcodes.h"
#include "execution.h"
#include "infusion.h"
#include "panic.h"
#include "program_mem.h"
#include "asm.h"
#include "rtc.h"
#include "rtc_complex_instructions.h"
#include "rtc_instructions_common.h"

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
void rtc_common_translate_invokelight(uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1) {
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
    dj_di_pointer methodImpl = dj_global_id_getMethodImplementation(globalId);
    uint8_t rettype = dj_di_methodImplementation_getReturnType(methodImpl);

    emit_2_CALL((uint16_t)handler);
    rtc_common_push_returnvalue_from_R22_if_necessary(rettype);

#if defined (AOT_STRATEGY_MARKLOOP)
    // If the invoke light happens in a MARKLOOP loop, the lightweight method might corrupt some pinned values, so we need to save them back to their variable slots first.
    // If we're not in a MARKLOOP loop, this will be a noop.
    rtc_markloop_emit_prologue(true, localId.entity_id);    
#endif
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
