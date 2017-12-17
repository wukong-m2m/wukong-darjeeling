#ifdef AOT_STRATEGY_MARKLOOP

#include "types.h"
#include "array.h"
#include "panic.h"
#include "opcodes.h"
#include "infusion.h"
#include "program_mem.h"
#include "execution.h"
#include "wkreprog.h"
#include "rtc.h"
#include "rtc_branches.h"
#include "rtc_complex_instructions.h"
#include "rtc_instructions_common.h"
#include "asm.h"
#include "rtc_markloop.h"
#include "rtc_safetychecks.h"

// NOTE: Function pointers are a "PC address", so already divided by 2 since the PC counts in words, not bytes.
// avr-libgcc functions used by translation
extern void __divmodhi4(void);
extern void __mulsi3(void);
extern void __mulhisi3(void);
extern void __divmodsi4(void);

void rtc_translate_single_instruction() {
    rtc_translationstate *ts = rtc_ts;
    dj_infusion *target_infusion;
    uint16_t offset;
    uint8_t m, n;
    int8_t i;

#ifdef AVRORA
#ifdef AOT_SAFETY_CHECKS
    avroraRTCTraceDarjeelingOpcodeInProgmemWithStackDepth(ts->jvm_code_start + ts->pc, rtc_ts->pre_instruction_int_stack, rtc_ts->pre_instruction_ref_stack);
#else
    avroraRTCTraceDarjeelingOpcodeInProgmem(ts->jvm_code_start + ts->pc);
#endif
#endif

    uint8_t opcode = dj_di_getU8(ts->jvm_code_start + ts->pc);
    DEBUG_LOG(DBG_RTCTRACE, "[rtc] JVM opcode %d (pc=%d, method length=%d)\n", opcode, ts->pc, ts->method_length);

    // Load possible operands. May waste some time if we don't need then, but saves some space.
#ifdef AOT_SAFETY_CHECKS
    uint8_t jvm_operand_byte0 = rtc_ts->jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
    uint8_t jvm_operand_byte1 = rtc_ts->jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ts->pc + 2);
    uint8_t jvm_operand_byte2 = rtc_ts->jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ts->pc + 3);
    uint8_t jvm_operand_byte3 = rtc_ts->jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
#else // AOT_SAFETY_CHECKS
    uint8_t jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
    uint8_t jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ts->pc + 2);
    uint8_t jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ts->pc + 3);
    uint8_t jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
#endif // AOT_SAFETY_CHECKS
    uint16_t jvm_operand_word0 = (jvm_operand_byte0 << 8) | jvm_operand_byte1;
    uint16_t jvm_operand_word1 = (jvm_operand_byte2 << 8) | jvm_operand_byte3;
    uint8_t operand_regs1[12];
    uint8_t *operand_regs2 = operand_regs1 + 4;
    uint8_t *operand_regs3 = operand_regs1 + 8;

#ifdef AOT_SAFETY_CHECKS
    rtc_ts->current_opcode = opcode;
#endif

    if (!rtc_poppedstackcache_can_I_skip_this()) {
    // rtc_poppedstackcache will check if the result of the current instruction
    // is already in a register. if so it will update the cache state to put this
    // value at the top of the stack, and return true.
    // if it returns false, we just need to generate this value as normal.
    // not that poppedstackcache uses a global reference to ts, which is a bit ugly
    switch (opcode) {
        case JVM_NOP:
        break;
        case JVM_SCONST_M1:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], 0xFF);
                emit_LDI(operand_regs1[1], 0xFF);
                rtc_stackcache_push_16bit(operand_regs1);                
            } else {
                emit_LDI(RZL, 0xFF);
                emit_LDI(RZH, 0xFF);
                emit_MOVW(operand_regs1[0], RZ);
                rtc_stackcache_push_16bit(operand_regs1);                
            }
        break;
        case JVM_SCONST_0:
            rtc_stackcache_getfree_16bit(operand_regs1);
            emit_CLR(operand_regs1[0]); // Operand is implicit in opcode
            emit_CLR(operand_regs1[1]);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SCONST_1:
        case JVM_SCONST_2:
        case JVM_SCONST_3:
        case JVM_SCONST_4:
        case JVM_SCONST_5:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], opcode - JVM_SCONST_0); // Operand is implicit in opcode
                emit_CLR(operand_regs1[1]);
            } else {
                emit_LDI(RZL, opcode - JVM_SCONST_0); // Operand is implicit in opcode
                emit_CLR(RZH);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_ICONST_M1:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], 0xFF);
                emit_LDI(operand_regs1[1], 0xFF);
            } else {
                emit_LDI(RZL, 0xFF);
                emit_LDI(RZH, 0xFF);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_getfree_16bit(operand_regs1+2);
            emit_MOVW(operand_regs1[2], operand_regs1[0]);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ICONST_0:
            rtc_stackcache_getfree_32bit(operand_regs1);
            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            emit_CLR(operand_regs1[2]);
            emit_CLR(operand_regs1[3]);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ICONST_1:
        case JVM_ICONST_2:
        case JVM_ICONST_3:
        case JVM_ICONST_4:
        case JVM_ICONST_5:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], opcode - JVM_ICONST_0); // Operand is implicit in opcode
                emit_CLR(operand_regs1[1]);
            } else {
                emit_LDI(RZL, opcode - JVM_ICONST_0); // Operand is implicit in opcode
                emit_CLR(RZH);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_getfree_16bit(operand_regs1+2);
            emit_CLR(operand_regs1[2]);
            emit_CLR(operand_regs1[3]);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ACONST_NULL:
            rtc_stackcache_getfree_ref(operand_regs1);
            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            rtc_stackcache_push_ref(operand_regs1);
        break;
        case JVM_BSPUSH:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], jvm_operand_byte0);
                if(jvm_operand_byte0 & 0x80) { // Sign extend
                    emit_LDI(operand_regs1[1], 0xFF);
                } else {
                    emit_LDI(operand_regs1[1], 0x00);
                }
            } else {
                emit_LDI(RZL, jvm_operand_byte0);
                if(jvm_operand_byte0 & 0x80) { // Sign extend
                    emit_LDI(RZH, 0xFF);
                } else {
                    emit_LDI(RZH, 0x00);
                }
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_BIPUSH:
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], jvm_operand_byte0);
                emit_CLR(operand_regs1[1]);
            } else {
                emit_LDI(RZL, jvm_operand_byte0);
                emit_CLR(RZH);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_getfree_16bit(operand_regs1+2);
            emit_CLR(operand_regs1[2]);
            emit_CLR(operand_regs1[3]);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_SSPUSH:
            // bytecode is big endian
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], jvm_operand_byte1);
                emit_LDI(operand_regs1[1], jvm_operand_byte0);
            } else {
                emit_LDI(RZL, jvm_operand_byte1);
                emit_LDI(RZH, jvm_operand_byte0);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SIPUSH:
            // bytecode is big endian
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], jvm_operand_byte1);
                emit_LDI(operand_regs1[1], jvm_operand_byte0);
            } else {
                emit_LDI(RZL, jvm_operand_byte1);
                emit_LDI(RZH, jvm_operand_byte0);
                emit_MOVW(operand_regs1[0], RZ);
            }
            rtc_stackcache_getfree_16bit(operand_regs1+2);
            emit_CLR(operand_regs1[2]);
            emit_CLR(operand_regs1[3]);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_IIPUSH:
            // bytecode is big endian

            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1)) {
                emit_LDI(operand_regs1[0], jvm_operand_byte3);
                emit_LDI(operand_regs1[1], jvm_operand_byte2);
            } else {
                emit_LDI(RZL, jvm_operand_byte3);
                emit_LDI(RZH, jvm_operand_byte2);
                emit_MOVW(operand_regs1[0], RZ);
            }
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs1+2)) {
                emit_LDI(operand_regs1[2], jvm_operand_byte1);
                emit_LDI(operand_regs1[3], jvm_operand_byte0);
            } else {
                emit_LDI(RZL, jvm_operand_byte1);
                emit_LDI(RZH, jvm_operand_byte0);
                emit_MOVW(operand_regs1[2], RZL);                
            }
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_LDS:
            rtc_flush_and_cleartags_ref(RTC_FILTER_CALLUSED_AND_REFERENCE, RTC_FILTER_CALLUSED_AND_REFERENCE);

            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id
            emit_x_CALL((uint16_t)&RTC_LDS);

            // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
            emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
            emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
            // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
            emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
            emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X

            // push the reference to the string onto the ref stack
            rtc_stackcache_push_ref_from_R24R25();
        break;
        case JVM_SLOAD:
        case JVM_SLOAD_0:
        case JVM_SLOAD_1:
        case JVM_SLOAD_2:
        case JVM_SLOAD_3:
            if (opcode != JVM_SLOAD)
                jvm_operand_byte0 = opcode - JVM_SLOAD_0;
            rtc_stackcache_getfree_16bit(operand_regs1);
            emit_load_local_16bit(operand_regs1, offset_for_intlocal_short(jvm_operand_byte0));
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_ILOAD:
        case JVM_ILOAD_0:
        case JVM_ILOAD_1:
        case JVM_ILOAD_2:
        case JVM_ILOAD_3:
            if (opcode != JVM_ILOAD)
                jvm_operand_byte0 = opcode - JVM_ILOAD_0;
            rtc_stackcache_getfree_32bit(operand_regs1);
            emit_load_local_32bit(operand_regs1, offset_for_intlocal_int(jvm_operand_byte0));
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ALOAD:
        case JVM_ALOAD_0:
        case JVM_ALOAD_1:
        case JVM_ALOAD_2:
        case JVM_ALOAD_3:
            if (opcode != JVM_ALOAD)
                jvm_operand_byte0 = opcode - JVM_ALOAD_0;
            rtc_stackcache_getfree_ref(operand_regs1);
            emit_load_local_ref(operand_regs1, offset_for_reflocal(jvm_operand_byte0));
            rtc_stackcache_push_ref(operand_regs1);
        break;
        case JVM_SSTORE:
        case JVM_SSTORE_0:
        case JVM_SSTORE_1:
        case JVM_SSTORE_2:
        case JVM_SSTORE_3:
            if (opcode != JVM_SSTORE)
                jvm_operand_byte0 = opcode - JVM_SSTORE_0;
            rtc_stackcache_pop_to_store_16bit(operand_regs1);
            emit_store_local_16bit(operand_regs1, offset_for_intlocal_short(jvm_operand_byte0));
        break;
        case JVM_ISTORE:
        case JVM_ISTORE_0:
        case JVM_ISTORE_1:
        case JVM_ISTORE_2:
        case JVM_ISTORE_3:
            if (opcode != JVM_ISTORE)
                jvm_operand_byte0 = opcode - JVM_ISTORE_0;
            rtc_stackcache_pop_to_store_32bit(operand_regs1);
            emit_store_local_32bit(operand_regs1, offset_for_intlocal_int(jvm_operand_byte0));
        break;
        case JVM_ASTORE:
        case JVM_ASTORE_0:
        case JVM_ASTORE_1:
        case JVM_ASTORE_2:
        case JVM_ASTORE_3:
            if (opcode != JVM_ASTORE)
                jvm_operand_byte0 = opcode - JVM_ASTORE_0;
            rtc_stackcache_pop_to_store_ref(operand_regs1);
            emit_store_local_ref(operand_regs1, offset_for_reflocal(jvm_operand_byte0));
        break;
        case JVM_BALOAD:
        case JVM_CALOAD:
        case JVM_SALOAD:
        case JVM_IALOAD:
        case JVM_AALOAD:
#ifdef ARRAYINDEX_32BIT
            rtc_stackcache_pop_nondestructive_32bit(operand_regs2);
            emit_MOVW(RZ, operand_regs2[0]);
#else
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(RZ);
#endif
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);

            if (opcode==JVM_SALOAD || opcode==JVM_AALOAD) {
                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit_LSL(RZL);
                emit_ROL(RZH);
            } else if (opcode==JVM_IALOAD) {
                // Multiply the index by 4, since we're indexing 16 bit shorts.
                emit_LSL(RZL);
                emit_ROL(RZH);
                emit_LSL(RZL);
                emit_ROL(RZH);
            }

            // Add the array base address to the offset in Z
            emit_ADD(RZL, operand_regs1[0]);
            emit_ADC(RZH, operand_regs1[1]);

            if (opcode == JVM_AALOAD) {
                // Add 4 to skip 2 bytes for array length and 2 bytes for array type.
                emit_ADIW(RZ, 4); 
            } else { // all types of int array
                // Add 3 to skip 2 bytes for array length and 1 byte for array type.
                emit_ADIW(RZ, 3); 
            }

            // Now Z points to the target element
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            // We could load into operand_regs1, but this contains the array reference, which we may want to preserve.
            // Ask the stack cache for an available register pair, but it shouldn't spill to memory if nothing is available.
            rtc_stackcache_getfree_16bit_for_array_load(operand_regs1);
            switch (opcode) {
                case JVM_BALOAD:
                case JVM_CALOAD:
                    emit_LD_Z(operand_regs1[0]);
                    emit_CLR(operand_regs1[1]);
                    emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so operand_regs1[0] can stay 0 (skip next instruction)
                    emit_COM(operand_regs1[1]); // otherwise: flip operand_regs1[0] to 0xFF to extend the sign
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_SALOAD:
                    emit_LD_ZINC(operand_regs1[0]);
                    emit_LD_Z(operand_regs1[1]);
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_IALOAD:
                    emit_LD_ZINC(operand_regs1[0]);
                    emit_LD_ZINC(operand_regs1[1]);
                    // If we're using 16 bit index, we now need 2 more registers to store the result
                    rtc_stackcache_getfree_16bit(operand_regs1+2);
                    emit_LD_ZINC(operand_regs1[2]);
                    emit_LD_Z(operand_regs1[3]);
                    rtc_stackcache_push_32bit(operand_regs1);
                break;
                case JVM_AALOAD:
                    emit_LD_ZINC(operand_regs1[0]);
                    emit_LD_Z(operand_regs1[1]);
                    rtc_stackcache_push_ref(operand_regs1);
                break;
            }
        break;
        case JVM_BASTORE:
        case JVM_CASTORE:
        case JVM_SASTORE:
        case JVM_IASTORE:
        case JVM_AASTORE:
            // Pop the value we need to store in the array.
            switch (opcode) {
                case JVM_BASTORE:
                case JVM_CASTORE:
                case JVM_SASTORE:
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
                break;
                case JVM_IASTORE:
                    rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
                break;
                case JVM_AASTORE:
                    rtc_stackcache_pop_nondestructive_ref(operand_regs1);
                break;
            }

            rtc_stackcache_pop_nondestructive_ref(operand_regs2);
#ifdef ARRAYINDEX_32BIT
            rtc_stackcache_pop_nondestructive_32bit(operand_regs3);
            emit_MOVW(RZ, operand_regs3[0]);
#else
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(RZ);
#endif

            if (opcode==JVM_SASTORE || opcode==JVM_AASTORE) {
                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit_LSL(RZL);
                emit_ROL(RZH);
            } else if (opcode==JVM_IASTORE) {
                // Multiply the index by 4, since we're indexing 16 bit shorts.
                emit_LSL(RZL);
                emit_ROL(RZH);
                emit_LSL(RZL);
                emit_ROL(RZH);
            }

            // Add the array base address to the offset in Z
            emit_ADD(RZL, operand_regs2[0]);
            emit_ADC(RZH, operand_regs2[1]);

            if (opcode == JVM_AASTORE) {
                // Add 4 to skip 2 bytes for array length and 2 bytes for array type.
                emit_ADIW(RZ, 4); 
            } else { // all types of int array
                // Add 3 to skip 2 bytes for array length and 1 byte for array type.
                emit_ADIW(RZ, 3); 
            }

#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif

            // Now Z points to the target element
            switch (opcode) {
                case JVM_BASTORE:
                case JVM_CASTORE:
                    emit_ST_Z(operand_regs1[0]);
                break;
                case JVM_SASTORE:
                case JVM_AASTORE:
                    emit_ST_ZINC(operand_regs1[0]);
                    emit_ST_Z(operand_regs1[1]);
                break;
                case JVM_IASTORE:
                    emit_ST_ZINC(operand_regs1[0]);
                    emit_ST_ZINC(operand_regs1[1]);
                    emit_ST_ZINC(operand_regs1[2]);
                    emit_ST_Z(operand_regs1[3]);
                break;
            }
        break;
        case JVM_IPOP:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
        break;
        case JVM_IPOP2:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
        break;
        case JVM_IDUP:
            // IDUP duplicates the top one SLOTS on the integer stack, not the top int. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_getfree_16bit(operand_regs2);
            emit_MOVW(operand_regs2[0], operand_regs1[0]);
            rtc_stackcache_push_16bit(operand_regs1);
            rtc_stackcache_push_16bit(operand_regs2);
            rtc_poppedstackcache_set_valuetag(operand_regs2, rtc_poppedstackcache_get_valuetag(operand_regs1)); // Copy the valuetag
        break;
        case JVM_IDUP2:
            // IDUP2 duplicates the top two SLOTS on the integer stack, not the top two ints. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_getfree_32bit(operand_regs2);
            emit_MOVW(operand_regs2[0], operand_regs1[0]);
            emit_MOVW(operand_regs2[2], operand_regs1[2]);                
            rtc_stackcache_push_32bit(operand_regs1);
            rtc_stackcache_push_32bit(operand_regs2);
            rtc_poppedstackcache_set_valuetag(operand_regs2, rtc_poppedstackcache_get_valuetag(operand_regs1));
            rtc_poppedstackcache_set_valuetag(operand_regs2+2, rtc_poppedstackcache_get_valuetag(operand_regs1+2)); // Copy the valuetags
        break;
        case JVM_IDUP_X:
            m = jvm_operand_byte0;
            n = m & 15;
            m >>= 4;
            // m: how many integer slots to duplicate
            // n: how deep to bury them in the stack. (see getIDupInstructions in the infuser)
            // not that the infuser always generated code with n>=m
            // Example for m=1 (copy 1), n=2 (bury 2 deep):
            // ..., v1, v2 -> ..., v2, v1, v2
            if (n == 0 || n > 4) {
                // n == 0 not supported, n>4 also not supported. Could be expanded using more registers, but for now it's not necessary
                dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
            } else {
                uint8_t j;

                // First pop n values
                for (i = 0; i < n; i++) {
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1 + (2*i));
                }

                // Then push the m values that need to be duplicated
                for (i = m-1, j = 0; i >= 0; i--, j++) { // loop from m-1 back to 0
                    // rtc_stackcache_IDUP_X_copy_and_push(operand_regs1[2*i]);
                    uint8_t idupx_copy_operands[2];
                    rtc_stackcache_getfree_16bit(idupx_copy_operands);
                    emit_MOVW(idupx_copy_operands[0], operand_regs1[2*i]);
                    rtc_stackcache_push_16bit(idupx_copy_operands);
                }

                // Finally push the original n values back on the stack
                for (i = n-1; i >= 0; i--) { // loop from n-1 back to 0
                    rtc_stackcache_push_16bit(operand_regs1 + (2*i));
                }
            }
        break;
        case JVM_ISWAP_X:
            m = jvm_operand_byte0;
            n = m & 15;
            m >>= 4;
            if ((n != 1 && n != 2) || (m != 1 && m != 2)) {
                dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
            }
            // example:
            // 0Short,0Int                                                 ; 0Int,0Short
            // m=2, n=1 (m=top, n=second)
            // pop top(m), pop second(n), push top(m), push second(n)
            if (m==1) { rtc_stackcache_pop_nondestructive_16bit(operand_regs1); } else { rtc_stackcache_pop_nondestructive_32bit(operand_regs1); }
            if (n==1) { rtc_stackcache_pop_nondestructive_16bit(operand_regs2); } else { rtc_stackcache_pop_nondestructive_32bit(operand_regs2); }
            if (m==1) {               rtc_stackcache_push_16bit(operand_regs1); } else {               rtc_stackcache_push_32bit(operand_regs1); }
            if (n==1) {               rtc_stackcache_push_16bit(operand_regs2); } else {               rtc_stackcache_push_32bit(operand_regs2); }
        break;
        case JVM_APOP:
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
        break;
        case JVM_ADUP:
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
            rtc_stackcache_getfree_ref(operand_regs2);
            emit_MOVW(operand_regs2[0], operand_regs1[0]);
            rtc_stackcache_push_ref(operand_regs1);
            rtc_stackcache_push_ref(operand_regs2);
            rtc_poppedstackcache_set_valuetag(operand_regs2, rtc_poppedstackcache_get_valuetag(operand_regs1)); // Copy the valuetag
        break;
        case JVM_GETFIELD_B:
        case JVM_GETFIELD_C:
            rtc_stackcache_getfree_16bit(operand_regs1);
            // POP the reference into Z
            rtc_stackcache_pop_destructive_ref_into_Z();

            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            // need to extend the sign to push it as a short
            emit_CLR(operand_regs1[1]);
            emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so the high byte can stay 0 (skip next instruction)
            emit_COM(operand_regs1[1]); // otherwise: flip the high byte to 0xFF to extend the sign

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_GETFIELD_S:
            rtc_stackcache_getfree_16bit(operand_regs1);
            // POP the reference into Z
            rtc_stackcache_pop_destructive_ref_into_Z();

            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            emit_LDD(operand_regs1[1], Z, jvm_operand_word0+1);

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_GETFIELD_I:
            rtc_stackcache_getfree_32bit(operand_regs1);
            // POP the reference into Z
            rtc_stackcache_pop_destructive_ref_into_Z();

            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            emit_LDD(operand_regs1[1], Z, jvm_operand_word0+1);
            emit_LDD(operand_regs1[2], Z, jvm_operand_word0+2);
            emit_LDD(operand_regs1[3], Z, jvm_operand_word0+3);

            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_GETFIELD_A:
            rtc_pop_flush_and_cleartags_ref(R24, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);
            rtc_stackcache_getfree_ref(operand_regs1);

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z
            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0*2);
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_LDD(operand_regs1[0], Z, (jvm_operand_word0)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_LDD(operand_regs1[1], Z, (jvm_operand_word0)+1);

            rtc_stackcache_push_ref(operand_regs1);
        break;
#ifndef NO_GETFIELD_A_FIXED
        case JVM_GETFIELD_A_FIXED: {
            uint16_t targetRefOffset = get_offset_for_FIELD_A_FIXED(jvm_operand_byte0, jvm_operand_byte1, jvm_operand_word1);

            rtc_stackcache_getfree_ref(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z(); // POP the reference into Z

            targetRefOffset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, targetRefOffset);
#ifdef AOT_SAFETY_CHECKS_READS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_LDD(operand_regs1[0], Z, targetRefOffset);
            emit_LDD(operand_regs1[1], Z, targetRefOffset+1);

            rtc_stackcache_push_ref(operand_regs1);
        }
        break;
#endif
        case JVM_PUTFIELD_B:
        case JVM_PUTFIELD_C:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_STD(operand_regs1[0], Z, jvm_operand_word0);
        break;
        case JVM_PUTFIELD_S:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_STD(operand_regs1[0], Z, jvm_operand_word0);
            emit_STD(operand_regs1[1], Z, jvm_operand_word0+1);
        break;
        case JVM_PUTFIELD_I:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0);
#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_STD(operand_regs1[0], Z, jvm_operand_word0);
            emit_STD(operand_regs1[1], Z, jvm_operand_word0+1);
            emit_STD(operand_regs1[2], Z, jvm_operand_word0+2);
            emit_STD(operand_regs1[3], Z, jvm_operand_word0+3);
        break;
        case JVM_PUTFIELD_A:
            rtc_stackcache_pop_nondestructive_ref(operand_regs1); // POP the value to store
            rtc_stackcache_pop_nondestructive_ref(operand_regs2); // POP the reference
            rtc_stackcache_push_ref(operand_regs1); // PUSH the value to store
            rtc_stackcache_push_ref(operand_regs2); // PUSH the reference (TODONR: this would be a lot easier if the operands where in the reversed order. let's fix that in the infuser later.)

            rtc_stackcache_mark_all_inused_available(); // After the previous some registers may be marked IN_USE, but we need them to be AVAILABLE
            rtc_pop_flush_and_cleartags_ref(R24, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z

            rtc_stackcache_pop_nondestructive_ref(operand_regs1); // POP the value to store again
            jvm_operand_word0 = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, jvm_operand_word0*2);
#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_STD(operand_regs1[0], Z, (jvm_operand_word0)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_STD(operand_regs1[1], Z, (jvm_operand_word0)+1);
        break;
#ifndef NO_GETFIELD_A_FIXED
        case JVM_PUTFIELD_A_FIXED: {
            uint16_t targetRefOffset = get_offset_for_FIELD_A_FIXED(jvm_operand_byte0, jvm_operand_byte1, jvm_operand_word1);

            rtc_stackcache_pop_nondestructive_ref(operand_regs1); // POP the value to store
            rtc_stackcache_pop_destructive_ref_into_Z(); // POP the reference into Z

            targetRefOffset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, targetRefOffset);

#ifdef AOT_SAFETY_CHECKS
            emit_2_CALL((uint16_t)&rtc_safety_mem_check);
#endif            
            emit_STD(operand_regs1[0], Z, targetRefOffset); // targetRefOffset is an index in the (16 bit) array, so multiply by 2
            emit_STD(operand_regs1[1], Z, targetRefOffset+1);
        }
        break;
#endif
        case JVM_GETSTATIC_B:
        case JVM_GETSTATIC_C:
        case JVM_GETSTATIC_S:
        case JVM_GETSTATIC_I:
        case JVM_GETSTATIC_A:
        case JVM_PUTSTATIC_B:
        case JVM_PUTSTATIC_C:
        case JVM_PUTSTATIC_S:
        case JVM_PUTSTATIC_I:
        case JVM_PUTSTATIC_A:
            // jvm_operand_byte0: the infusion.
            // jvm_operand_byte1: Get the field.
            emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

            if (jvm_operand_byte0 == 0) {
                target_infusion = ts->infusion;
                offset = 0; // We will _add_ the real offset to this below. For statics in a different infusion, offset will be initialised to sizeof(dj_infusion).
            } else {
                // We need to read from another infusion. Get that infusion's address first.
                // Load the address of the referenced infusion into operand_regs2[0]:operand_regs2[1]
                rtc_stackcache_getfree_16bit(operand_regs2);
                offset = rtc_offset_for_referenced_infusion(ts->infusion, jvm_operand_byte0);
                offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                emit_LDD(operand_regs2[0], Z, offset);
                emit_LDD(operand_regs2[1], Z, offset+1);
                // Then move operand_regs2[0]:operand_regs2[1] to Z
                emit_MOVW(RZ, operand_regs2[0]);
                // Find the target infusion to calculate the right offset in the next step
                target_infusion = dj_infusion_resolve(ts->infusion, jvm_operand_byte0);
                offset = sizeof(dj_infusion); // Initialise offset to sizeof(dj_infusion) because Z now points to the target infusion, but below we will calculate the offset relative to the start of the static variables, which follow the dj_infusion struct in memory.
            }
            switch (opcode) {
                case JVM_GETSTATIC_B:
                case JVM_GETSTATIC_C:
                    rtc_stackcache_getfree_16bit(operand_regs1);
                    offset += rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_LDD(operand_regs1[0], Z, offset);
                    // need to extend the sign to push the byte as a short
                    emit_CLR(operand_regs1[1]);
                    emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so operand_regs1[0] can stay 0 (skip next instruction)
                    emit_COM(operand_regs1[1]); // otherwise: flip operand_regs1[0] to 0xFF to extend the sign
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_GETSTATIC_S:
                    rtc_stackcache_getfree_16bit(operand_regs1);
                    offset += rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_GETSTATIC_I:
                    rtc_stackcache_getfree_32bit(operand_regs1);
                    offset += rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    emit_LDD(operand_regs1[2], Z, offset+2);
                    emit_LDD(operand_regs1[3], Z, offset+3);
                    rtc_stackcache_push_32bit(operand_regs1);
                break;
                case JVM_GETSTATIC_A:
                    rtc_stackcache_getfree_ref(operand_regs1);
                    offset += rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    rtc_stackcache_push_ref(operand_regs1);
                break;
                case JVM_PUTSTATIC_B:
                case JVM_PUTSTATIC_C:
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
                    offset += rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_STD(operand_regs1[0], Z, offset);
                break;
                case JVM_PUTSTATIC_S:
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
                    offset += rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                break;
                case JVM_PUTSTATIC_I:
                    rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
                    offset += rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                    emit_STD(operand_regs1[2], Z, offset+2);
                    emit_STD(operand_regs1[3], Z, offset+3);
                break;
                case JVM_PUTSTATIC_A:
                    rtc_stackcache_pop_nondestructive_ref(operand_regs1);
                    offset += rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    offset = emit_ADIW_if_necessary_to_bring_offset_in_range(RZ, offset);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                break;
            }
        break;
        case JVM_SSUB:
            rtc_stackcache_set_may_use_RZ();
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_clear_may_use_RZ();
            rtc_stackcache_pop_destructive_16bit(operand_regs2);
            emit_SUB(operand_regs2[0], operand_regs1[0]);
            emit_SBC(operand_regs2[1], operand_regs1[1]);
            rtc_stackcache_mark_available_16bit(operand_regs1);
            rtc_stackcache_push_16bit(operand_regs2);
        break;
        case JVM_SMUL:
            if (rtc_stackcache_stack_top_is_pinned()) {
                rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
                rtc_stackcache_pop_destructive_16bit(operand_regs1);
            } else {
                rtc_stackcache_pop_destructive_16bit(operand_regs1);
                rtc_stackcache_set_may_use_RZ();
                rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
            }
            rtc_stackcache_getfree_16bit(operand_regs3);

            // Code generated by avr-gcc -mmcu=atmega2560 -O3
            // mul r24,r22
            // movw r18,r0
            // mul r24,r23
            // add r19,r0
            // mul r25,r22
            // add r19,r0
            // clr r1
            // movw r24,r18
            // ret

            emit_MUL(operand_regs1[0], operand_regs2[0]);
            emit_MOVW(operand_regs3[0], R0);
            emit_MUL(operand_regs1[0], operand_regs2[1]);
            emit_ADD(operand_regs3[1], R0);
            emit_MUL(operand_regs1[1], operand_regs2[0]);
            emit_ADD(operand_regs3[1], R0);
            emit_CLR(R1);

            rtc_stackcache_mark_available_16bit(operand_regs1);
            rtc_stackcache_mark_available_16bit(operand_regs2);
            rtc_stackcache_push_16bit(operand_regs3);
        break;
        case JVM_SDIV:
        case JVM_SREM:
            rtc_pop_flush_and_cleartags_int16(R22, R24, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);

            emit_x_CALL((uint16_t)&__divmodhi4);
            if (opcode == JVM_SDIV) {
                operand_regs1[0] = R22;
                operand_regs1[1] = R23;
                rtc_stackcache_push_16bit(operand_regs1);
            } else { // JVM_SREM
                rtc_stackcache_push_16bit_from_R24R25();
            }
        break;
        case JVM_SNEG:
            rtc_stackcache_getfree_16bit(operand_regs1);
            rtc_stackcache_set_may_use_RZ();
            rtc_stackcache_pop_nondestructive_16bit(operand_regs2);

            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            emit_SUB(operand_regs1[0], operand_regs2[0]);
            emit_SBC(operand_regs1[1], operand_regs2[1]);

            rtc_stackcache_mark_available_16bit(operand_regs2);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SSHL:
        case JVM_SSHR:
        case JVM_SUSHR:
#ifndef NO_CONSTSHIFT 
        case JVM_SSHL_CONST:
        case JVM_SSHR_CONST:
        case JVM_SUSHR_CONST:
            {
                bool emit_loop = opcode == JVM_SSHL || opcode == JVM_SSHR || opcode == JVM_SUSHR;
                uint8_t bits_to_shift = emit_loop ? 1 : jvm_operand_byte0;
#else
            {
                 // If we turn off this optimisation, just set these to fixed values and let the compiler take care of removing unnecessary code. (tested. it does.)
                bool emit_loop = true;
                uint8_t bits_to_shift = 1;
#endif
                if (emit_loop) {
                    rtc_stackcache_set_may_use_RZ();
                    rtc_stackcache_pop_destructive_16bit(operand_regs1);
                    rtc_stackcache_clear_may_use_RZ();
                }

                rtc_stackcache_pop_destructive_16bit(operand_regs2); // pop the operand

                // Emit code. Here we need emit_loop and bits_to_shift to be set.
                // If emit_loop is true, bits_to_shift should be 1, and the loop reg should be in operand1[0]
                if (emit_loop) {
                    emit_RJMP(4);
                }

                while (bits_to_shift >= 8) {
                    if (opcode == JVM_SSHL_CONST) {                
                        emit_MOV(operand_regs2[1], operand_regs2[0]);
                        emit_CLR(operand_regs2[0]);
                    } else if (opcode == JVM_SSHR_CONST) {
                        emit_CLR(RZL);
                        emit_SBRC(operand_regs2[1], 7);
                        emit_COM(RZL);
                        emit_MOV(operand_regs2[0], operand_regs2[1]);
                        emit_MOV(operand_regs2[1], RZL);
                    } else if (opcode == JVM_SUSHR_CONST) {
                        emit_MOV(operand_regs2[0], operand_regs2[1]);
                        emit_CLR(operand_regs2[1]);
                    }                    
                    bits_to_shift -= 8;
                }

                while (bits_to_shift > 0) {
                    if (opcode == JVM_SSHL || opcode == JVM_SSHL_CONST) {
                        emit_LSL(operand_regs2[0]);
                        emit_ROL(operand_regs2[1]);
                    } else if (opcode == JVM_SSHR || opcode == JVM_SSHR_CONST) {
                        emit_ASR(operand_regs2[1]);
                        emit_ROR(operand_regs2[0]);
                    } else if (opcode == JVM_SUSHR || opcode == JVM_SUSHR_CONST) {
                        emit_LSR(operand_regs2[1]);
                        emit_ROR(operand_regs2[0]);
                    }
                    bits_to_shift--;
                }

                if (emit_loop) {
                    emit_DEC(operand_regs1[0]);
                    emit_BRPL(-8);
                    rtc_stackcache_mark_available_16bit(operand_regs1);
                }

                rtc_stackcache_push_16bit(operand_regs2);
            }
        break;
        case JVM_SADD:
        case JVM_SAND:
        case JVM_SOR:
        case JVM_SXOR:
            if (rtc_stackcache_stack_top_is_pinned()) {
                rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
                rtc_stackcache_pop_destructive_16bit(operand_regs1);
            } else {
                rtc_stackcache_pop_destructive_16bit(operand_regs1);
                rtc_stackcache_set_may_use_RZ();
                rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
            }

            if (opcode == JVM_SAND) {
                emit_AND(operand_regs1[0], operand_regs2[0]);
                emit_AND(operand_regs1[1], operand_regs2[1]);
            } else if (opcode == JVM_SOR) {
                emit_OR(operand_regs1[0], operand_regs2[0]);
                emit_OR(operand_regs1[1], operand_regs2[1]);
            } else if (opcode == JVM_SXOR) {
                emit_EOR(operand_regs1[0], operand_regs2[0]);
                emit_EOR(operand_regs1[1], operand_regs2[1]);
            } else if (opcode == JVM_SADD) {
                emit_ADD(operand_regs1[0], operand_regs2[0]);
                emit_ADC(operand_regs1[1], operand_regs2[1]);
            }

            rtc_stackcache_mark_available_16bit(operand_regs2);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_ISUB:
            rtc_stackcache_set_may_use_RZ();
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_clear_may_use_RZ();
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_SUB(operand_regs2[0], operand_regs1[0]);
            emit_SBC(operand_regs2[1], operand_regs1[1]);
            emit_SBC(operand_regs2[2], operand_regs1[2]);
            emit_SBC(operand_regs2[3], operand_regs1[3]);

            rtc_stackcache_mark_available_32bit(operand_regs2);
            rtc_stackcache_push_32bit(operand_regs2);
        break;
#ifndef NO_SIMUL
        case JVM_SIMUL:
            // Note that __mulhisi3 needs one of the operands in RX (R26:R27)

            rtc_pop_flush_and_cleartags_int16(R18, R20, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED); // First pop the operand in R20 because this may change RX
            emit_MOVW(RZ, RX); // Now save RX in RZ
            emit_MOVW(RX, R20); // And then move the operand into RX

            // ;;; R25:R22 = (signed long) R27:R26 * (signed long) R19:R18
            // ;;; C3:C0   = (signed long) A1:A0   * (signed long) B1:B0
            // ;;; Clobbers: __tmp_reg__
            // DEFUN __mulhisi3            

            emit_x_CALL_without_saving_RX((uint16_t)&__mulhisi3);
            rtc_stackcache_push_32bit_from_R22R25();

            emit_MOVW(RX, RZ);
        break;
#endif
        case JVM_IMUL: // to read later: https://mekonik.wordpress.com/2009/03/18/arduino-avr-gcc-multiplication/
            rtc_pop_flush_and_cleartags_double_int32(R18, R22, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);

            emit_x_CALL((uint16_t)&__mulsi3);
            rtc_stackcache_push_32bit_from_R22R25();
        break;
        case JVM_IDIV:
        case JVM_IREM:
            rtc_pop_flush_and_cleartags_double_int32(R18, R22, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);

            emit_x_CALL((uint16_t)&__divmodsi4);
            if (opcode == JVM_IDIV) {
                operand_regs1[0] = R18;
                operand_regs1[1] = R19;
                operand_regs1[2] = R20;
                operand_regs1[3] = R21;
                rtc_stackcache_push_32bit(operand_regs1);
            } else { // JVM_IREM
                rtc_stackcache_push_32bit_from_R22R25();
            }
        break;
        case JVM_INEG:
            rtc_stackcache_getfree_32bit(operand_regs1);
            rtc_stackcache_set_may_use_RZ();
            rtc_stackcache_pop_nondestructive_32bit(operand_regs2);

            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            emit_MOVW(operand_regs1[2], operand_regs1[0]);
            emit_SUB(operand_regs1[0], operand_regs2[0]);
            emit_SBC(operand_regs1[1], operand_regs2[1]);
            emit_SBC(operand_regs1[2], operand_regs2[2]);
            emit_SBC(operand_regs1[3], operand_regs2[3]);

            rtc_stackcache_mark_available_32bit(operand_regs2);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ISHL:
        case JVM_ISHR:
        case JVM_IUSHR:
#ifndef NO_CONSTSHIFT 
        case JVM_ISHL_CONST:
        case JVM_ISHR_CONST:
        case JVM_IUSHR_CONST:
            {
                bool emit_loop = opcode == JVM_ISHL || opcode == JVM_ISHR || opcode == JVM_IUSHR;
                uint8_t bits_to_shift = emit_loop ? 1 : jvm_operand_byte0;
#else
            {
                 // If we turn off this optimisation, just set these to fixed values and let the compiler take care of removing unnecessary code. (tested. it does.)
                bool emit_loop = true;
                uint8_t bits_to_shift = 1;
#endif
                if(emit_loop) {
                    rtc_stackcache_set_may_use_RZ();
                    rtc_stackcache_pop_destructive_16bit(operand_regs1);
                    rtc_stackcache_clear_may_use_RZ();
                }

                rtc_stackcache_pop_destructive_32bit(operand_regs2); // pop the operand

                // Emit code. Here we need emit_loop and bits_to_shift to be set.
                // If emit_loop is true, bits_to_shift should be 1, and the loop reg should be in operand1[0]
                if (emit_loop) {
                    emit_RJMP(8);
                }

                if (bits_to_shift >= 8) {
                    uint8_t bytes_to_shift = bits_to_shift / 8;
                    bits_to_shift = bits_to_shift % 8;
                    uint8_t extension_reg = R1;
                    if (opcode == JVM_ISHR_CONST) {
                        emit_CLR(RZL);
                        emit_SBRC(operand_regs2[3], 7);
                        emit_COM(RZL);
                        extension_reg = RZL;
                    }

                    uint8_t *target_reg;
                    uint8_t *source_reg;
                    int8_t step;
                    if (opcode == JVM_ISHL_CONST) {
                        target_reg = &(operand_regs2[3]);
                        source_reg = target_reg - bytes_to_shift;
                        step = -1;
                    } else { // JVM_I[U]SHR
                        target_reg = &(operand_regs2[0]);
                        source_reg = target_reg + bytes_to_shift;
                        step = 1;
                    }

                    uint8_t bytes_to_clear = bytes_to_shift;
                    uint8_t bytes_to_move = 4 - bytes_to_shift;
                    while (bytes_to_move-- > 0) {
                        emit_MOV(*target_reg, *source_reg);
                        target_reg += step;
                        source_reg += step;
                    }
                    while (bytes_to_clear-- > 0) {
                        emit_MOV(*target_reg, extension_reg);
                        target_reg += step;
                    }
                }

                while (bits_to_shift > 0) {
                    if (opcode == JVM_ISHL || opcode == JVM_ISHL_CONST) {                
                        emit_LSL(operand_regs2[0]);
                        emit_ROL(operand_regs2[1]);
                        emit_ROL(operand_regs2[2]);
                        emit_ROL(operand_regs2[3]);
                    } else if (opcode == JVM_ISHR || opcode == JVM_ISHR_CONST) {
                        emit_ASR(operand_regs2[3]);
                        emit_ROR(operand_regs2[2]);
                        emit_ROR(operand_regs2[1]);
                        emit_ROR(operand_regs2[0]);
                    } else if (opcode == JVM_IUSHR || opcode == JVM_IUSHR_CONST) {
                        emit_LSR(operand_regs2[3]);
                        emit_ROR(operand_regs2[2]);
                        emit_ROR(operand_regs2[1]);
                        emit_ROR(operand_regs2[0]);
                    }
                    bits_to_shift--;
                }

                if (emit_loop) {
                    emit_DEC(operand_regs1[0]);
                    emit_BRPL(-12);
                    rtc_stackcache_mark_available_16bit(operand_regs1);
                }

                rtc_stackcache_push_32bit(operand_regs2);
            }
        break;
        case JVM_IADD:
        case JVM_IAND:
        case JVM_IOR:
        case JVM_IXOR:
            if (rtc_stackcache_stack_top_is_pinned()) {
                rtc_stackcache_pop_nondestructive_32bit(operand_regs2);
                rtc_stackcache_pop_destructive_32bit(operand_regs1);
            } else {
                rtc_stackcache_pop_destructive_32bit(operand_regs1);
                rtc_stackcache_set_may_use_RZ();
                rtc_stackcache_pop_nondestructive_32bit(operand_regs2);
            }

            if (opcode == JVM_IAND) {
                emit_AND(operand_regs1[0], operand_regs2[0]);
                emit_AND(operand_regs1[1], operand_regs2[1]);
                emit_AND(operand_regs1[2], operand_regs2[2]);
                emit_AND(operand_regs1[3], operand_regs2[3]);
            } else if (opcode == JVM_IOR) {
                emit_OR(operand_regs1[0], operand_regs2[0]);
                emit_OR(operand_regs1[1], operand_regs2[1]);
                emit_OR(operand_regs1[2], operand_regs2[2]);
                emit_OR(operand_regs1[3], operand_regs2[3]);
            } else if (opcode == JVM_IXOR) {
                emit_EOR(operand_regs1[0], operand_regs2[0]);
                emit_EOR(operand_regs1[1], operand_regs2[1]);
                emit_EOR(operand_regs1[2], operand_regs2[2]);
                emit_EOR(operand_regs1[3], operand_regs2[3]);
            } else if (opcode == JVM_IADD) {
                emit_ADD(operand_regs1[0], operand_regs2[0]);
                emit_ADC(operand_regs1[1], operand_regs2[1]);
                emit_ADC(operand_regs1[2], operand_regs2[2]);
                emit_ADC(operand_regs1[3], operand_regs2[3]);
            } 

            rtc_stackcache_mark_available_32bit(operand_regs2);
            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_SINC:
        case JVM_SINC_W:
            rtc_common_translate_inc(opcode, jvm_operand_byte0, jvm_operand_byte1, jvm_operand_byte2);

            rtc_poppedstackcache_clear_all_except_pinned_with_valuetag(ts->current_instruction_valuetag); // Any cached value for this variable is now outdated.
        break;
        case JVM_IINC:
        case JVM_IINC_W:
            rtc_common_translate_inc(opcode, jvm_operand_byte0, jvm_operand_byte1, jvm_operand_byte2);

            rtc_poppedstackcache_clear_all_except_pinned_with_valuetag(ts->current_instruction_valuetag); // Any cached value for this variable is now outdated.
            rtc_poppedstackcache_clear_all_except_pinned_with_valuetag(RTC_VALUETAG_TO_INT_L(ts->current_instruction_valuetag)); // Also for the 2nd word
        break;
        case JVM_S2B:
        case JVM_S2C:
            rtc_stackcache_pop_destructive_16bit(operand_regs1);

            // need to extend the sign
            emit_CLR(operand_regs1[1]);
            emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so the high byte can stay 0 (skip next instruction)
            emit_COM(operand_regs1[1]); // otherwise: flip the high byte to 0xFF to extend the sign

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_S2I:
            // 32 bit: pop 16 off the stack, the reserve 16 more free bits
            // needs to be in this order to get a little endian 32 bit integer
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_getfree_16bit(operand_regs1+2);

            // need to extend the sign
            emit_CLR(operand_regs1[2]);
            emit_SBRC(operand_regs1[1], 7); // highest bit of MSB operand_regs1[1] cleared -> S value is positive, so operand_regs1[2] can stay 0 (skip next instruction)
            emit_COM(operand_regs1[2]); // otherwise: flip operand_regs1[2] to 0xFF to extend the sign
            emit_MOV(operand_regs1[3], operand_regs1[2]);

            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_I2S:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SRETURN:
            // NOTE THAT THIS IS NOT STANDARD avr-gcc ABI, WHICH EXPECTS 16 bit VALUES IN R24:R25, BUT THIS ALLOWS FOR MORE EFFICIENT HANDLING IN CALLMETHOD.
            rtc_pop_flush_and_cleartags_int16(R22, 0, RTC_FILTER_NONE, RTC_FILTER_NONE);
            emit_x_branchtag(OPCODE_RJMP, ts->methodimpl_header.nr_branch_targets); // We add a final branchtag at the end of the method as the exit point.
        break;
        case JVM_IRETURN:
            rtc_pop_flush_and_cleartags_single_int32(R22, RTC_FILTER_NONE, RTC_FILTER_NONE);
            emit_x_branchtag(OPCODE_RJMP, ts->methodimpl_header.nr_branch_targets); // We add a final branchtag at the end of the method as the exit point.
        break;
        case JVM_ARETURN:
            // NOTE THAT THIS IS NOT STANDARD avr-gcc ABI, WHICH EXPECTS 16 bit VALUES IN R24:R25, BUT THIS ALLOWS FOR MORE EFFICIENT HANDLING IN CALLMETHOD.
            rtc_pop_flush_and_cleartags_ref(R22, RTC_FILTER_NONE, RTC_FILTER_NONE);
            emit_x_branchtag(OPCODE_RJMP, ts->methodimpl_header.nr_branch_targets); // We add a final branchtag at the end of the method as the exit point.
        break;
        case JVM_RETURN:
            emit_x_branchtag(OPCODE_RJMP, ts->methodimpl_header.nr_branch_targets); // We add a final branchtag at the end of the method as the exit point.
        break;
        case JVM_INVOKEVIRTUAL:
        case JVM_INVOKESPECIAL:
        case JVM_INVOKESTATIC:
        case JVM_INVOKEINTERFACE:
            // clear the stack cache, so all stack elements are in memory, not in registers
            // We can't just flush the call used registers here because the method operands need to be in memory

            // clear the all valuetags for all call-used registers, since the value may be gone after the function call returns,
            // and all references in call-saved registers since they may not be accurate if the garbage collector runs.
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_CALLUSED_AND_REFERENCE);

            rtc_common_translate_invoke(ts, opcode, jvm_operand_byte0, jvm_operand_byte1);
        break;
#ifndef NO_LIGHTWEIGHT_METHODS
        case JVM_INVOKELIGHT:
            rtc_common_translate_invokelight(jvm_operand_byte0, jvm_operand_byte1);
        break;
#endif
        case JVM_NEW:
            rtc_flush_and_cleartags_ref(RTC_FILTER_CALLUSED_AND_REFERENCE, RTC_FILTER_CALLUSED_AND_REFERENCE);

            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id
            emit_x_CALL((uint16_t)&RTC_NEW);

            // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
            emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
            emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
            // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
            emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
            emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


            // push the reference to the new object onto the ref stack
            rtc_stackcache_push_ref_from_R24R25();
        break;
        case JVM_NEWARRAY:
            rtc_flush_and_cleartags_ref(RTC_FILTER_CALLUSED_AND_REFERENCE, RTC_FILTER_CALLUSED_AND_REFERENCE);

            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(R22); // size
            emit_LDI(R24, jvm_operand_byte0); // (int) element type
            emit_x_CALL((uint16_t)&dj_int_array_create);


            // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
            emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
            emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
            // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
            emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
            emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


            // push the reference to the new array onto the ref stack
            rtc_stackcache_push_ref_from_R24R25();
        break;
        case JVM_ANEWARRAY:
            rtc_flush_and_cleartags_ref(RTC_FILTER_CALLUSED_AND_REFERENCE, RTC_FILTER_CALLUSED_AND_REFERENCE);

            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(R22); // size
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id
            emit_x_CALL((uint16_t)&RTC_ANEWARRAY);

            // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
            emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
            emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
            // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
            emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
            emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


            // push the reference to the new object onto the ref stack
            rtc_stackcache_push_ref_from_R24R25();
        break;
        case JVM_ARRAYLENGTH: // The length of an array is stored as 16 bit at the start of the array
            rtc_stackcache_pop_destructive_ref_into_Z(RZ); // POP the reference into Z
            rtc_stackcache_getfree_16bit(operand_regs1);
            emit_LD_ZINC(operand_regs1[0]);
            emit_LD_Z(operand_regs1[1]);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_ATHROW:
            // TODO: fix exceptions
        break;
        case JVM_CHECKCAST:
            // ADUP first, CHECKCAST should only peek
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
            rtc_stackcache_getfree_ref(operand_regs2);
            emit_MOVW(operand_regs2[0], operand_regs1[0]);
            rtc_stackcache_push_ref(operand_regs1);
            rtc_stackcache_push_ref(operand_regs2);
            rtc_poppedstackcache_set_valuetag(operand_regs2, rtc_poppedstackcache_get_valuetag(operand_regs1)); // Copy the valuetag

            rtc_stackcache_mark_all_inused_available(); // After the previous some registers may be marked IN_USE, but we need them to be AVAILABLE

            rtc_pop_flush_and_cleartags_ref(R22, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);

            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id

            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY
            emit_x_CALL((uint16_t)&RTC_CHECKCAST);
        break;
        case JVM_INSTANCEOF:
            rtc_pop_flush_and_cleartags_ref(R22, RTC_FILTER_CALLUSED, RTC_FILTER_CALLUSED);
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id

            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY
            emit_x_CALL((uint16_t)&RTC_INSTANCEOF);

            // push the result onto the stack
            rtc_stackcache_push_16bit_from_R24R25();
        break;
        case JVM_MONITORENTER:
        case JVM_MONITOREXIT:
            // Since we don't support threads, there's no point in implementing these. But we still need to pop the reference.
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
        break;
        // BRANCHES
        case JVM_SIFEQ:
        case JVM_SIFNE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_destructive_16bit(operand_regs1);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_SIFEQ) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_SIFNE) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            }
        break;
        case JVM_SIFLT:
        case JVM_SIFGE:
        case JVM_SIFGT:
        case JVM_SIFLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_SIFLT) {
                emit_CP(operand_regs1[1], ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_SIFGE) {
                emit_CP(operand_regs1[1], ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_SIFGT) {
                emit_CP(ZERO_REG, operand_regs1[0]);
                emit_CPC(ZERO_REG, operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_SIFLE) {
                emit_CP(ZERO_REG, operand_regs1[0]);
                emit_CPC(ZERO_REG, operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IIFEQ:
        case JVM_IIFNE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_destructive_32bit(operand_regs1);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_IIFEQ) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_OR(operand_regs1[0], operand_regs1[2]);
                emit_OR(operand_regs1[0], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IIFNE) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_OR(operand_regs1[0], operand_regs1[2]);
                emit_OR(operand_regs1[0], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            }
        break;
        case JVM_IIFLT:
        case JVM_IIFGE:
        case JVM_IIFGT:
        case JVM_IIFLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_IIFLT) {
                emit_CP(operand_regs1[3], ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IIFGE) {
                emit_CP(operand_regs1[3], ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IIFGT) {
                emit_CP(ZERO_REG, operand_regs1[0]);
                emit_CPC(ZERO_REG, operand_regs1[1]);
                emit_CPC(ZERO_REG, operand_regs1[2]);
                emit_CPC(ZERO_REG, operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IIFLE) {
                emit_CP(ZERO_REG, operand_regs1[0]);
                emit_CPC(ZERO_REG, operand_regs1[1]);
                emit_CPC(ZERO_REG, operand_regs1[2]);
                emit_CPC(ZERO_REG, operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IFNULL:
        case JVM_IFNONNULL:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_destructive_ref(operand_regs1);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_IFNULL) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IFNONNULL) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            }
        break;
        case JVM_IF_SCMPEQ:
        case JVM_IF_SCMPNE:
        case JVM_IF_SCMPLT:
        case JVM_IF_SCMPGE:
        case JVM_IF_SCMPGT:
        case JVM_IF_SCMPLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
            if (opcode == JVM_IF_SCMPEQ) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPNE) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPLT) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPGE) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPGT) {
                emit_CP(operand_regs1[0], operand_regs2[0]);
                emit_CPC(operand_regs1[1], operand_regs2[1]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPLE) {
                emit_CP(operand_regs1[0], operand_regs2[0]);
                emit_CPC(operand_regs1[1], operand_regs2[1]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IF_ICMPEQ:
        case JVM_IF_ICMPNE:
        case JVM_IF_ICMPLT:
        case JVM_IF_ICMPGE:
        case JVM_IF_ICMPGT:
        case JVM_IF_ICMPLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_32bit(operand_regs2);
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_IF_ICMPEQ) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_CPC(operand_regs2[2], operand_regs1[2]);
                emit_CPC(operand_regs2[3], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPNE) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_CPC(operand_regs2[2], operand_regs1[2]);
                emit_CPC(operand_regs2[3], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPLT) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_CPC(operand_regs2[2], operand_regs1[2]);
                emit_CPC(operand_regs2[3], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPGE) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_CPC(operand_regs2[2], operand_regs1[2]);
                emit_CPC(operand_regs2[3], operand_regs1[3]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPGT) {
                emit_CP(operand_regs1[0], operand_regs2[0]);
                emit_CPC(operand_regs1[1], operand_regs2[1]);
                emit_CPC(operand_regs1[2], operand_regs2[2]);
                emit_CPC(operand_regs1[3], operand_regs2[3]);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPLE) {
                emit_CP(operand_regs1[0], operand_regs2[0]);
                emit_CPC(operand_regs1[1], operand_regs2[1]);
                emit_CPC(operand_regs1[2], operand_regs2[2]);
                emit_CPC(operand_regs1[3], operand_regs2[3]);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IF_ACMPEQ:
        case JVM_IF_ACMPNE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
            rtc_stackcache_pop_nondestructive_ref(operand_regs2);                
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_IF_ACMPEQ) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_ACMPNE) {
                emit_CP(operand_regs2[0], operand_regs1[0]);
                emit_CPC(operand_regs2[1], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            }
        break;
        case JVM_GOTO:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            emit_x_branchtag(OPCODE_RJMP, jvm_operand_word1);
        break;
        case JVM_TABLESWITCH: {
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;

            // Pop the key value, and reserve some registers
            rtc_stackcache_pop_destructive_32bit(operand_regs1);

            // Load the lower bound
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));

            // Substract lower bound from the key to find the index
            if (jvm_operand_byte0 != 0 || jvm_operand_byte1 != 0 || jvm_operand_byte2 != 0 || jvm_operand_byte3 != 0) {
                emit_LDI(RZL, jvm_operand_byte3);
                emit_SUB(operand_regs1[0], RZL);
                emit_LDI(RZL, jvm_operand_byte2);
                emit_SBC(operand_regs1[1], RZL);
                emit_LDI(RZL, jvm_operand_byte1);
                emit_SBC(operand_regs1[2], RZL);
                emit_LDI(RZL, jvm_operand_byte0);
                emit_SBC(operand_regs1[3], RZL);
            }

            // Load the range (=upperbound-lowerbound)
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            int32_t range = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));

            // Do an unsigned (BRLO) compare to see if the index is negative or larger than the upper bound
            emit_LDI(RZL, jvm_operand_byte3); // Bytecode is big endian
            emit_CP (RZL, operand_regs1[0]);
            emit_LDI(RZL, jvm_operand_byte2);
            emit_CPC(RZL, operand_regs1[1]);
            emit_LDI(RZL, jvm_operand_byte1);
            emit_CPC(RZL, operand_regs1[2]);
            emit_LDI(RZL, jvm_operand_byte0);
            emit_CPC(RZL, operand_regs1[3]);
            emit_x_branchtag(OPCODE_BRLO, jvm_operand_word1);

            // lower bound <= index <= upper bound, so we need to jump to a case: label.
            // operand_regs1[0]:operand_regs1[1] now contains the index (it can't be > 16 bits since that doesn't fit in flash)
            // The branch targets may not have consecutive numbers, for example if there are branches within a switch case
            // We'll do a double jump instead, first IJMPing to a table of RJMPs to the branch target table
            // So a total of 3 jmps instead of 2 for a normal branch. This could be optimised a bit by making sure the branch targets
            // are consecutive, which we could enforce in the infuser, but that would only save a few cycles and given the
            // amount of work we're already doing here, it won't speed things up by much, so I can't be bothered.

            emit_RCALL(0); // RCALL to offset 0 does nothing, except get the PC on the stack, which we need here

            emit_POP(RZH); // POP PC into Z
            emit_POP(RZL);
            emit_ADIW(RZ, 6); // Will need to compensate here for the instructions inbetween RCALL(0) and the table. Now Z will point to the start of the RJMP table.
                              // Note that this should be 7 for cpus with >128K flash since they have a 3 byte PC.
            emit_ADD(RZL, operand_regs1[0]); // Add the index to get the target address in the RJMP table
            emit_ADC(RZH, operand_regs1[1]);

            emit_IJMP(); // All this fuss because there's no relative indirect jump...

            // Now emit the RJMP table itself
            for (int i=0; i<(range+1); i++) { // +1 since both bounds are inclusive
                jvm_operand_word0 = (dj_di_getU8(ts->jvm_code_start + ts->pc + 3) << 8) | dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
                ts->pc += 4;
                emit_x_branchtag(OPCODE_RJMP, jvm_operand_word0);
            }
        }
        break;
        case JVM_LOOKUPSWITCH: {
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            uint16_t default_branch_target = jvm_operand_word1;
            ts->pc += 4;

            // Pop the key value, and reserve some registers
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);

            uint16_t number_of_cases = (dj_di_getU8(ts->jvm_code_start + ts->pc + 1) << 8) | dj_di_getU8(ts->jvm_code_start + ts->pc + 2);
            ts->pc += 2;
            for (int i=0; i<number_of_cases; i++) {
                // Get the case label
                jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
                jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
                jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
                jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
                // Get the branch target (and skip the branch address used by the interpreter)
                jvm_operand_word0 = (dj_di_getU8(ts->jvm_code_start + ts->pc + 3) << 8) | dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
                ts->pc += 4;
                emit_LDI(RZL, jvm_operand_byte3); // Bytecode is big endian
                emit_CP(RZL, operand_regs1[0]);
                emit_LDI(RZL, jvm_operand_byte2);
                emit_CPC(RZL, operand_regs1[1]);
                emit_LDI(RZL, jvm_operand_byte1);
                emit_CPC(RZL, operand_regs1[2]);
                emit_LDI(RZL, jvm_operand_byte0);
                emit_CPC(RZL, operand_regs1[3]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word0);
            }

            emit_x_branchtag(OPCODE_RJMP, default_branch_target);
        }
        break;
        case JVM_BRTARGET:
            rtc_flush_and_cleartags_ref(RTC_FILTER_ALL, RTC_FILTER_NONE); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            rtc_poppedstackcache_clear_all_except_pinned_valuetags();

            rtc_mark_branchtarget();
        break;
        case JVM_MARKLOOP_START:
            rtc_markloop_emit_prologue(false, 0);
            ts->pc += (2*jvm_operand_byte0)+1;
        break;
        case JVM_MARKLOOP_END:
            rtc_markloop_emit_epilogue(false, 0);
        break;

        // Not implemented
        default:
            DEBUG_LOG(DBG_RTC, "Unimplemented Java opcode %d at pc=%d\n", opcode, ts->pc);
            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
        break;
    }
    }

#ifdef AOT_SAFETY_CHECKS
    rtc_safety_check_opcode(opcode);
#endif

    rtc_stackcache_next_instruction();
    ts->pc += rtc_number_of_operandbytes_for_opcode(opcode);
    ts->pc++;
}

#endif // AOT_STRATEGY_MARKLOOP
