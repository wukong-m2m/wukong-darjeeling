#include <stdint.h>
#include "opcodes.h"
#include "execution.h"
#include "program_mem.h"
#include "asm.h"
#include "rtc.h"
#include "rtc_complex_instructions.h"

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
