#include <stdint.h>
#include "opcodes.h"
#include "execution.h"
#include "program_mem.h"
#include "asm.h"
#include "rtc.h"
#include "rtc_complex_instructions.h"
#include "rtc_markloop.h"

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

    if        (opcode == JVM_INVOKEVIRTUAL
            || opcode == JVM_INVOKEINTERFACE) {
        emit_LDI(R20, jvm_operand_byte2); // nr_ref_args
        emit_2_CALL((uint16_t)&RTC_INVOKEVIRTUAL_OR_INTERFACE);
    } else if (opcode == JVM_INVOKESPECIAL) {
        emit_2_CALL((uint16_t)&RTC_INVOKESPECIAL);
    } else if (opcode == JVM_INVOKESTATIC) {
        dj_di_pointer methodImpl = dj_global_id_getMethodImplementation(globalId);
        uint8_t flags = dj_di_methodImplementation_getFlags(methodImpl);
        rettype = dj_di_methodImplementation_getReturnType(methodImpl);

        emit_LDI(R20, ((uint16_t)methodImpl) & 0xFF);
        emit_LDI(R21, (((uint16_t)methodImpl) >> 8) & 0xFF);

        if ((flags & FLAGS_NATIVE) != 0) {
            // void RTC_INVOKESTATIC_FAST_NATIVE(dj_global_id methodImplId, dj_di_pointer methodImpl);
            emit_2_CALL((uint16_t)&RTC_INVOKESTATIC_FAST_NATIVE);
        } else {
            // void RTC_INVOKESTATIC_FAST_JAVA(dj_global_id methodImplId, dj_di_pointer methodImpl, uint8_t flags);
            emit_LDI(R25, flags);
            emit_2_CALL((uint16_t)&RTC_INVOKESTATIC_FAST_JAVA);
        }
    }

    emit_x_postinvoke();

    // Will be VOID except for INVOKESTATIC calls that return something.
    switch (rettype) {
        case JTID_BOOLEAN:
        case JTID_CHAR:
        case JTID_BYTE:
        case JTID_SHORT:
            emit_MOVW(R24, R22); // Since we return everything as 32 bit, we need to move the lower 16 bits to R24 in order to use rtc_stackcache_push_16bit_from_R24R25. Optimise this later by pushing R22 directly. This is just a test now.
            rtc_stackcache_push_16bit_from_R24R25();
            break;
        case JTID_INT:
            rtc_stackcache_push_32bit_from_R22R25();
            break;
        case JTID_REF:
            emit_MOVW(R24, R22); // Since we return everything as 32 bit, we need to move the lower 16 bits to R24 in order to use rtc_stackcache_push_16bit_from_R24R25. Optimise this later by pushing R22 directly. This is just a test now.
            rtc_stackcache_push_ref_from_R24R25();
            break;
    }
}
