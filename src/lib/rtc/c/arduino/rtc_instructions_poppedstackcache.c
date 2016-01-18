#ifdef AOT_STRATEGY_POPPEDSTACKCACHE

#include "types.h"
#include "array.h"
#include "panic.h"
#include "opcodes.h"
#include "infusion.h"
#include "program_mem.h"
#include "wkreprog.h"
#include "rtc.h"
#include "rtc_branches.h"
#include "rtc_complex_instructions.h"
#include "asm.h"
#include "rtc_poppedstackcache.h"

// NOTE: Function pointers are a "PC address", so already divided by 2 since the PC counts in words, not bytes.
// avr-libgcc functions used by translation
extern void __divmodhi4(void);
extern void __mulsi3(void);
extern void __divmodsi4(void);
// the stack pointers used by execution.c
extern int16_t *intStack;
extern ref_t *refStack;
extern ref_t *localReferenceVariables;

void rtc_translate_single_instruction(rtc_translationstate *ts) {
    dj_infusion *target_infusion;
    dj_di_pointer tmp_current_position;
    uint8_t offset;
    uint8_t m, n;
    int8_t i;

#ifdef AVRORA
    avroraRTCTraceDarjeelingOpcodeInProgmem(ts->jvm_code_start + ts->pc);
#endif

    uint8_t opcode = dj_di_getU8(ts->jvm_code_start + ts->pc);
    DEBUG_LOG(DBG_RTCTRACE, "[rtc] JVM opcode %d (pc=%d, method length=%d)\n", opcode, ts->pc, ts->method_length);

    // Load possible operands. May waste some time if we don't need then, but saves some space.
    uint8_t jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
    uint8_t jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ts->pc + 2);
    uint8_t jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ts->pc + 3);
    uint8_t jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
    uint16_t jvm_operand_word0 = (jvm_operand_byte0 << 8) | jvm_operand_byte1;
    uint16_t jvm_operand_word1 = (jvm_operand_byte2 << 8) | jvm_operand_byte3;
    int16_t jvm_operand_signed_word;
    uint8_t operand_regs1[12];
    uint8_t *operand_regs2 = operand_regs1 + 4;
    uint8_t *operand_regs3 = operand_regs1 + 8;

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
                emit_CLR(operand_regs1[1]);
            } else {
                emit_LDI(RZL, jvm_operand_byte0);
                emit_CLR(RZH);
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

            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack

            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
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
            emit_LDD(operand_regs1[0], Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(operand_regs1[1], Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0)+1);
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
            emit_LDD(operand_regs1[0], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(operand_regs1[1], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+1);
            emit_LDD(operand_regs1[2], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+2);
            emit_LDD(operand_regs1[3], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+3);
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
            emit_LDD(operand_regs1[0], Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(operand_regs1[1], Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0)+1);
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
            emit_STD(operand_regs1[0], Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0));
            emit_STD(operand_regs1[1], Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0)+1);
        break;
        case JVM_ISTORE:
        case JVM_ISTORE_0:
        case JVM_ISTORE_1:
        case JVM_ISTORE_2:
        case JVM_ISTORE_3:
            if (opcode != JVM_ISTORE)
                jvm_operand_byte0 = opcode - JVM_ISTORE_0;
            rtc_stackcache_pop_to_store_32bit(operand_regs1);
            emit_STD(operand_regs1[0], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0));
            emit_STD(operand_regs1[1], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+1);
            emit_STD(operand_regs1[2], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+2);
            emit_STD(operand_regs1[3], Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+3);
        break;
        case JVM_ASTORE:
        case JVM_ASTORE_0:
        case JVM_ASTORE_1:
        case JVM_ASTORE_2:
        case JVM_ASTORE_3:
            if (opcode != JVM_ASTORE)
                jvm_operand_byte0 = opcode - JVM_ASTORE_0;
            rtc_stackcache_pop_to_store_ref(operand_regs1);
            emit_STD(operand_regs1[0], Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0));
            emit_STD(operand_regs1[1], Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0)+1);
        break;
        case JVM_BALOAD:
        case JVM_CALOAD:
        case JVM_SALOAD:
        case JVM_IALOAD:
        case JVM_AALOAD:
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(RZ);
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
            // We could load into operand_regs1, but this contains the array reference, which we may want to preserve.
            // Ask the stack cache for an available register pair, but it shouldn't spill to memory if nothing is available.
            rtc_stackcache_getfree_16bit_but_only_if_we_wont_spill_otherwise_clear_valuetag(operand_regs1);
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
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(RZ);

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
                uint8_t idupx_copy_operands[8];
                uint8_t j;

                // First pop n values
                for (i = 0; i < n; i++) {
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1 + (2*i));
                }

                // Then push the m values that need to be duplicated
                for (i = 0; i < m; i++) {
                    // Allocate space first
                    rtc_stackcache_getfree_16bit(idupx_copy_operands + (2*i));
                }
                for (i = m-1, j = 0; i >= 0; i--, j++) { // loop from m-1 back to 0
                    // Then copy the values and push them
                    emit_MOVW(idupx_copy_operands[2*j], operand_regs1[2*i]);
                    rtc_stackcache_push_16bit(idupx_copy_operands + 2*j);
                }

                // Finally push the original n values back on the stack
                for (i = n-1; i >= 0; i--) { // loop from n-1 back to 0
                    rtc_stackcache_push_16bit(operand_regs1 + (2*i));
                }
            }
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

            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            // need to extend the sign to push it as a short
            emit_CLR(operand_regs1[1]);
            emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
            emit_COM(operand_regs1[1]); // otherwise: flip R24 to 0xFF to extend the sign

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_GETFIELD_S:
            rtc_stackcache_getfree_16bit(operand_regs1);
            // POP the reference into Z
            rtc_stackcache_pop_destructive_ref_into_Z();

            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            emit_LDD(operand_regs1[1], Z, jvm_operand_word0+1);

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_GETFIELD_I:
            rtc_stackcache_getfree_32bit(operand_regs1);
            // POP the reference into Z
            rtc_stackcache_pop_destructive_ref_into_Z();

            emit_LDD(operand_regs1[0], Z, jvm_operand_word0);
            emit_LDD(operand_regs1[1], Z, jvm_operand_word0+1);
            emit_LDD(operand_regs1[2], Z, jvm_operand_word0+2);
            emit_LDD(operand_regs1[3], Z, jvm_operand_word0+3);

            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_GETFIELD_A:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_ref_into_fixed_reg(R24); // POP the reference

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z
            emit_LDD(R22, Z, (jvm_operand_word0*2)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_LDD(R23, Z, (jvm_operand_word0*2)+1);
            operand_regs1[0] = R22;
            operand_regs1[1] = R23;
            rtc_stackcache_push_ref(operand_regs1);
        break;
        case JVM_PUTFIELD_B:
        case JVM_PUTFIELD_C:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
            emit_STD(operand_regs1[0], Z, jvm_operand_word0);
        break;
        case JVM_PUTFIELD_S:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
            emit_STD(operand_regs1[0], Z, jvm_operand_word0);
            emit_STD(operand_regs1[1], Z, jvm_operand_word0+1);
        break;
        case JVM_PUTFIELD_I:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_ref_into_Z();
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
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_ref_into_fixed_reg(R24); // POP the reference again into R24

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z

            rtc_stackcache_pop_nondestructive_ref(operand_regs1); // POP the value to store again
            emit_STD(operand_regs1[0], Z, (jvm_operand_word0*2)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_STD(operand_regs1[1], Z, (jvm_operand_word0*2)+1);
        break;
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
            } else {
                // We need to read from another infusion. Get that infusion's address first.
                // Load the address of the referenced infusion into operand_regs2[0]:operand_regs2[1]
                rtc_stackcache_getfree_16bit(operand_regs2);
                offset = rtc_offset_for_referenced_infusion(ts->infusion, jvm_operand_byte0);
                emit_LDD(operand_regs2[0], Z, offset);
                emit_LDD(operand_regs2[1], Z, offset+1);
                // Then move operand_regs2[0]:operand_regs2[1] to Z
                emit_MOVW(RZ, operand_regs2[0]);
                // Z now points to the target infusion, but it should point to the start of the static variables
                emit_ADIW(RZ, sizeof(dj_infusion));
                // Find the target infusion to calculate the right offset in the next step
                target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
            }
            switch (opcode) {
                case JVM_GETSTATIC_B:
                case JVM_GETSTATIC_C:
                    rtc_stackcache_getfree_16bit(operand_regs1);
                    emit_LDD(operand_regs1[0], Z, rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1));
                    // need to extend the sign to push the byte as a short
                    emit_CLR(operand_regs1[1]);
                    emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so operand_regs1[0] can stay 0 (skip next instruction)
                    emit_COM(operand_regs1[1]); // otherwise: flip operand_regs1[0] to 0xFF to extend the sign
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_GETSTATIC_S:
                    rtc_stackcache_getfree_16bit(operand_regs1);
                    offset = rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    rtc_stackcache_push_16bit(operand_regs1);
                break;
                case JVM_GETSTATIC_I:
                    rtc_stackcache_getfree_32bit(operand_regs1);
                    offset = rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    emit_LDD(operand_regs1[2], Z, offset+2);
                    emit_LDD(operand_regs1[3], Z, offset+3);
                    rtc_stackcache_push_32bit(operand_regs1);
                break;
                case JVM_GETSTATIC_A:
                    rtc_stackcache_getfree_ref(operand_regs1);
                    offset = rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    emit_LDD(operand_regs1[0], Z, offset);
                    emit_LDD(operand_regs1[1], Z, offset+1);
                    rtc_stackcache_push_ref(operand_regs1);
                break;
                case JVM_PUTSTATIC_B:
                case JVM_PUTSTATIC_C:
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
                    emit_STD(operand_regs1[0], Z, rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1));
                break;
                case JVM_PUTSTATIC_S:
                    rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
                    offset = rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                break;
                case JVM_PUTSTATIC_I:
                    rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
                    offset = rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                    emit_STD(operand_regs1[2], Z, offset+2);
                    emit_STD(operand_regs1[3], Z, offset+3);
                break;
                case JVM_PUTSTATIC_A:
                    rtc_stackcache_pop_nondestructive_ref(operand_regs1);
                    offset = rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    emit_STD(operand_regs1[0], Z, offset);
                    emit_STD(operand_regs1[1], Z, offset+1);
                break;
            }
        break;
        case JVM_SADD:
            rtc_stackcache_pop_destructive_16bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
            emit_ADD(operand_regs1[0], operand_regs2[0]);
            emit_ADC(operand_regs1[1], operand_regs2[1]);
            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SSUB:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_16bit(operand_regs2);
            emit_SUB(operand_regs2[0], operand_regs1[0]);
            emit_SBC(operand_regs2[1], operand_regs1[1]);
            rtc_stackcache_push_16bit(operand_regs2);
        break;
        case JVM_SMUL:
            rtc_stackcache_pop_destructive_16bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_16bit(operand_regs2);
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

            rtc_stackcache_push_16bit(operand_regs3);
        break;
        case JVM_SDIV:
        case JVM_SREM:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(R22);
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(R24);

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
            rtc_stackcache_pop_nondestructive_16bit(operand_regs2);

            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            emit_SUB(operand_regs1[0], operand_regs2[0]);
            emit_SBC(operand_regs1[1], operand_regs2[1]);

            rtc_stackcache_push_16bit(operand_regs1);
        break;
        case JVM_SSHL:
        case JVM_SSHR:
        case JVM_SUSHR:
            #ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    rtc_stackcache_pop_destructive_16bit(operand_regs1);
                }
                rtc_stackcache_pop_destructive_16bit(operand_regs2); // operand
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    emit_RJMP(4);
                }

                if (opcode == JVM_SSHL) {
                    emit_LSL(operand_regs2[0]);
                    emit_ROL(operand_regs2[1]);
                } else if (opcode == JVM_SSHR) {
                    emit_ASR(operand_regs2[1]);
                    emit_ROR(operand_regs2[0]);
                } else if (opcode == JVM_SUSHR) {
                    emit_LSR(operand_regs2[1]);
                    emit_ROR(operand_regs2[0]);
                }
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    emit_DEC(operand_regs1[0]);
                    emit_BRPL(-8);
                } else {
                    // special case for shifting by 1 bit. -> optimise I/SCONST_1 followed by a shift, to a single shift.
                    ts->do_CONST1_SHIFT_optimisation = false;
                }

                rtc_stackcache_push_16bit(operand_regs2);
            #else
            rtc_stackcache_pop_destructive_16bit(operand_regs1); // number of bits to shift
            rtc_stackcache_pop_destructive_16bit(operand_regs2); // operand

            emit_RJMP(4);
            if (opcode == JVM_SSHL) {
                emit_LSL(operand_regs2[0]);
                emit_ROL(operand_regs2[1]);
            } else if (opcode == JVM_SSHR) {
                emit_ASR(operand_regs2[1]);
                emit_ROR(operand_regs2[0]);
            } else if (opcode == JVM_SUSHR) {
                emit_LSR(operand_regs2[1]);
                emit_ROR(operand_regs2[0]);
            }
            emit_DEC(operand_regs1[0]);
            emit_BRPL(-8);
            
            rtc_stackcache_push_16bit(operand_regs2);
            #endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        break;
        case JVM_SAND:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_16bit(operand_regs2);

            emit_AND(operand_regs2[0], operand_regs1[0]);
            emit_AND(operand_regs2[1], operand_regs1[1]);

            rtc_stackcache_push_16bit(operand_regs2);
        break;
        case JVM_SOR:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_16bit(operand_regs2);

            emit_OR(operand_regs2[0], operand_regs1[0]);
            emit_OR(operand_regs2[1], operand_regs1[1]);

            rtc_stackcache_push_16bit(operand_regs2);
        break;
        case JVM_SXOR:
            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_pop_destructive_16bit(operand_regs2);

            emit_EOR(operand_regs2[0], operand_regs1[0]);
            emit_EOR(operand_regs2[1], operand_regs1[1]);

            rtc_stackcache_push_16bit(operand_regs2);
        break;
        case JVM_IADD:
            rtc_stackcache_pop_destructive_32bit(operand_regs1);
            rtc_stackcache_pop_nondestructive_32bit(operand_regs2);

            emit_ADD(operand_regs1[0], operand_regs2[0]);
            emit_ADC(operand_regs1[1], operand_regs2[1]);
            emit_ADC(operand_regs1[2], operand_regs2[2]);
            emit_ADC(operand_regs1[3], operand_regs2[3]);

            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ISUB:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_SUB(operand_regs2[0], operand_regs1[0]);
            emit_SBC(operand_regs2[1], operand_regs1[1]);
            emit_SBC(operand_regs2[2], operand_regs1[2]);
            emit_SBC(operand_regs2[3], operand_regs1[3]);

            rtc_stackcache_push_32bit(operand_regs2);
        break;
        case JVM_IMUL: // to read later: https://mekonik.wordpress.com/2009/03/18/arduino-avr-gcc-multiplication/
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_32bit_into_fixed_reg(R22);
            rtc_stackcache_pop_destructive_32bit_into_fixed_reg(R18);

            emit_x_CALL((uint16_t)&__mulsi3);
            rtc_stackcache_push_32bit_from_R22R25();
        break;
        case JVM_IDIV:
        case JVM_IREM:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_32bit_into_fixed_reg(R18);
            rtc_stackcache_pop_destructive_32bit_into_fixed_reg(R22);

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
            rtc_stackcache_pop_nondestructive_32bit(operand_regs2);

            emit_CLR(operand_regs1[0]);
            emit_CLR(operand_regs1[1]);
            emit_MOVW(operand_regs1[2], operand_regs1[0]);
            emit_SUB(operand_regs1[0], operand_regs2[0]);
            emit_SBC(operand_regs1[1], operand_regs2[1]);
            emit_SBC(operand_regs1[2], operand_regs2[2]);
            emit_SBC(operand_regs1[3], operand_regs2[3]);

            rtc_stackcache_push_32bit(operand_regs1);
        break;
        case JVM_ISHL:
        case JVM_ISHR:
        case JVM_IUSHR:
            #ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    rtc_stackcache_pop_destructive_16bit(operand_regs1);
                }
                rtc_stackcache_pop_destructive_32bit(operand_regs2); // operand
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    emit_RJMP(8);
                }

                if (opcode == JVM_ISHL) {                
                    emit_LSL(operand_regs2[0]);
                    emit_ROL(operand_regs2[1]);
                    emit_ROL(operand_regs2[2]);
                    emit_ROL(operand_regs2[3]);
                } else if (opcode == JVM_ISHR) {
                    emit_ASR(operand_regs2[3]);
                    emit_ROR(operand_regs2[2]);
                    emit_ROR(operand_regs2[1]);
                    emit_ROR(operand_regs2[0]);
                } else if (opcode == JVM_IUSHR) {
                    emit_LSR(operand_regs2[3]);
                    emit_ROR(operand_regs2[2]);
                    emit_ROR(operand_regs2[1]);
                    emit_ROR(operand_regs2[0]);
                }
                if (!(ts->do_CONST1_SHIFT_optimisation)) {
                    emit_DEC(operand_regs1[0]);
                    emit_BRPL(-12);
                } else  {
                    // special case for shifting by 1 bit. -> optimise I/SCONST_1 followed by a shift, to a single shift.
                    ts->do_CONST1_SHIFT_optimisation = false;
                }

                rtc_stackcache_push_32bit(operand_regs2);
            #else
            if (opcode == JVM_IUSHR) {
                rtc_stackcache_pop_destructive_16bit(operand_regs1);
            } else { // JVM_ISHL or JVM_ISHR
                rtc_stackcache_pop_destructive_32bit(operand_regs1);
            }
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_RJMP(8);
            if (opcode == JVM_ISHL) {                
                emit_LSL(operand_regs2[0]);
                emit_ROL(operand_regs2[1]);
                emit_ROL(operand_regs2[2]);
                emit_ROL(operand_regs2[3]);
            } else if (opcode == JVM_ISHR) {
                emit_ASR(operand_regs2[3]);
                emit_ROR(operand_regs2[2]);
                emit_ROR(operand_regs2[1]);
                emit_ROR(operand_regs2[0]);
            } else if (opcode == JVM_IUSHR) {
                emit_LSR(operand_regs2[3]);
                emit_ROR(operand_regs2[2]);
                emit_ROR(operand_regs2[1]);
                emit_ROR(operand_regs2[0]);
            }
            emit_DEC(operand_regs1[0]);
            emit_BRPL(-12);

            rtc_stackcache_push_32bit(operand_regs2);
            #endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        break;
        case JVM_IAND:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_AND(operand_regs2[0], operand_regs1[0]);
            emit_AND(operand_regs2[1], operand_regs1[1]);
            emit_AND(operand_regs2[2], operand_regs1[2]);
            emit_AND(operand_regs2[3], operand_regs1[3]);

            rtc_stackcache_push_32bit(operand_regs2);
        break;
        case JVM_IOR:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_OR(operand_regs2[0], operand_regs1[0]);
            emit_OR(operand_regs2[1], operand_regs1[1]);
            emit_OR(operand_regs2[2], operand_regs1[2]);
            emit_OR(operand_regs2[3], operand_regs1[3]);

            rtc_stackcache_push_32bit(operand_regs2);
        break;
        case JVM_IXOR:
            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_pop_destructive_32bit(operand_regs2);

            emit_EOR(operand_regs2[0], operand_regs1[0]);
            emit_EOR(operand_regs2[1], operand_regs1[1]);
            emit_EOR(operand_regs2[2], operand_regs1[2]);
            emit_EOR(operand_regs2[3], operand_regs1[3]);

            rtc_stackcache_push_32bit(operand_regs2);
        break;
        case JVM_SINC:
        case JVM_SINC_W:
            // -129 -> JVM_SINC_W
            // -128 -> JVM_SINC
            // +127 -> JVM_SINC
            // +128 -> JVM_SINC_W
            // jvm_operand_byte0: index of int local
            if (opcode == JVM_SINC) {
                jvm_operand_signed_word = (int8_t)jvm_operand_byte1;
            } else {
                jvm_operand_signed_word = (int16_t)(((uint16_t)jvm_operand_byte1 << 8) + jvm_operand_byte2);
            }
            offset = offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0);
            if (jvm_operand_signed_word == 1) {
                // Special case
                emit_LDD(RZL, Y, offset);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset);
                emit_BRNE(6);
                emit_LDD(RZL, Y, offset+1);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset+1);
            } else {
                uint8_t c0, c1;
                if (jvm_operand_signed_word > 0) {
                    // Positive operand
                    c0 = -(jvm_operand_signed_word & 0xFF);
                    c1 = -((jvm_operand_signed_word >> 8) & 0xFF)-1;
                } else {
                    // Negative operand
                    c0 = (-jvm_operand_signed_word) & 0xFF;
                    c1 = ((-jvm_operand_signed_word) >> 8) & 0xFF;
                }

                emit_LDD(RZL, Y, offset);
                emit_SUBI(RZL, c0);
                emit_STD(RZL, Y, offset);

                emit_LDD(RZL, Y, offset+1);
                emit_SBCI(RZL, c1);
                emit_STD(RZL, Y, offset+1);
            }
            rtc_poppedstackcache_clear_all_with_valuetag(ts->current_instruction_valuetag); // Any cached value for this variable is now outdated.
        break;
        case JVM_IINC:
        case JVM_IINC_W:
            // -129 -> JVM_IINC_W
            // -128 -> JVM_IINC
            // +127 -> JVM_IINC
            // +128 -> JVM_IINC_W
            // jvm_operand_byte0: index of int local
            if (opcode == JVM_IINC) {
                jvm_operand_signed_word = (int8_t)jvm_operand_byte1;
            } else {
                jvm_operand_signed_word = (int16_t)(((uint16_t)jvm_operand_byte1 << 8) + jvm_operand_byte2);
            }
            offset = offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0);
            if (jvm_operand_signed_word == 1) {
                // Special case
                emit_LDD(RZL, Y, offset);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset);
                emit_BRNE(22);
                emit_LDD(RZL, Y, offset+1);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset+1);
                emit_BRNE(14);
                emit_LDD(RZL, Y, offset+2);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset+2);
                emit_BRNE(6);
                emit_LDD(RZL, Y, offset+3);
                emit_INC(RZL);
                emit_STD(RZL, Y, offset+3);
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

                emit_LDD(RZL, Y, offset);
                emit_SUBI(RZL, c0);
                emit_STD(RZL, Y, offset);

                emit_LDD(RZL, Y, offset+1);
                emit_SBCI(RZL, c1);
                emit_STD(RZL, Y, offset+1);

                emit_LDD(RZL, Y, offset+2);
                emit_SBCI(RZL, c2);
                emit_STD(RZL, Y, offset+2);

                emit_LDD(RZL, Y, offset+3);
                emit_SBCI(RZL, c3);
                emit_STD(RZL, Y, offset+3);
            }
            rtc_poppedstackcache_clear_all_with_valuetag(ts->current_instruction_valuetag); // Any cached value for this variable is now outdated.
            rtc_poppedstackcache_clear_all_with_valuetag(RTC_VALUETAG_TO_INT_L(ts->current_instruction_valuetag)); // Also for the 2nd word
        break;
        case JVM_S2B:
        case JVM_S2C:
            rtc_stackcache_pop_destructive_16bit(operand_regs1);

            // need to extend the sign
            emit_CLR(operand_regs1[1]);
            emit_SBRC(operand_regs1[0], 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
            emit_COM(operand_regs1[1]); // otherwise: flip R24 to 0xFF to extend the sign

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
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags(); // To make sure the return registers are available
            rtc_stackcache_pop_destructive_16bit_into_fixed_reg(R24);
            emit_x_epilogue();
        break;
        case JVM_IRETURN:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags(); // To make sure the return registers are available
            rtc_stackcache_pop_destructive_32bit_into_fixed_reg(R22);
            emit_x_epilogue();
        break;
        case JVM_ARETURN:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags(); // To make sure the return registers are available
            rtc_stackcache_pop_destructive_ref_into_fixed_reg(R24);
            emit_x_epilogue();
        break;
        case JVM_RETURN:
            emit_x_epilogue();
        break;
        case JVM_INVOKEVIRTUAL:
        case JVM_INVOKESPECIAL:
        case JVM_INVOKESTATIC:
        case JVM_INVOKEINTERFACE:

            // clear the stack cache, so all stack elements are in memory, not in registers
            rtc_stackcache_flush_all_regs();
            // clear the valuetags for all call-used registers, since the value may be gone after the function call returns
            rtc_poppedstackcache_clear_all_callused_valuetags();

            // set intStack to SP
            emit_PUSH(ZERO_REG); // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT. 
                                        // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                        // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.
            emit_2_LDS(RZL, SPaddress_L); // Load SP into RZ
            emit_2_LDS(RZH, SPaddress_H); // Load SP into RZ
            emit_2_STS((uint16_t)&(intStack), RZL); // Store SP into intStack
            emit_2_STS((uint16_t)&(intStack)+1, RZH); // Store SP into intStack

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

            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id
            if        (opcode == JVM_INVOKEVIRTUAL
                    || opcode == JVM_INVOKEINTERFACE) {
                emit_LDI(R22, jvm_operand_byte2); // nr_ref_args
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
            emit_2_LDS(RZL, (uint16_t)&(intStack)); // Load intStack into RZ
            emit_2_LDS(RZH, (uint16_t)&(intStack)+1); // Load intStack into RZ
            emit_2_STS(SPaddress_L, RZL); // Store RZ into SP
            emit_2_STS(SPaddress_H, RZH); // Store RZ into SP
            emit_POP(RZH); // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE FIRST LINE FOR INVOKESTATIC. SEE COMMENT ABOVE.
        break;
        case JVM_NEW:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();

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
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();

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
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();

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
        case JVM_CHECKCAST:
            // TODO: optimise this. CHECKCAST should only peek.
            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
            rtc_stackcache_push_ref(operand_regs1);
            rtc_stackcache_push_ref(operand_regs1);

            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_ref_into_fixed_reg(R22); // reference to the object
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id


            emit_x_CALL((uint16_t)&RTC_CHECKCAST);
        break;
        case JVM_INSTANCEOF:
            rtc_stackcache_flush_call_used_regs_and_clear_valuetags();
            rtc_stackcache_pop_destructive_ref_into_fixed_reg(R22); // reference to the object
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id

            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY
            emit_x_CALL((uint16_t)&RTC_INSTANCEOF);

            // push the result onto the stack
            rtc_stackcache_push_16bit_from_R24R25();
        break;
        // BRANCHES
        case JVM_SIFEQ:
        case JVM_SIFNE:
        case JVM_SIFLT:
        case JVM_SIFGE:
        case JVM_SIFGT:
        case JVM_SIFLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_nondestructive_16bit(operand_regs1);
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            if (opcode == JVM_SIFEQ) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_SIFNE) {
                emit_OR(operand_regs1[0], operand_regs1[1]);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_SIFLT) {
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
        case JVM_IIFLT:
        case JVM_IIFGE:
        case JVM_IIFGT:
        case JVM_IIFLE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)

            rtc_stackcache_pop_nondestructive_32bit(operand_regs1);
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
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
            } else if (opcode == JVM_IIFLT) {
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

            rtc_stackcache_pop_nondestructive_ref(operand_regs1);
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
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
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
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
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
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
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
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

            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            emit_x_branchtag(OPCODE_RJMP, jvm_operand_word1);
        break;
        case JVM_TABLESWITCH: {
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;

            // Pop the key value, and reserve some registers
            rtc_stackcache_pop_destructive_32bit(operand_regs1);

            // Load the upper bound
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            int32_t upperbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
            emit_LDI(RZL, jvm_operand_byte0); // Bytecode is big endian
            emit_CP (RZL, operand_regs1[0]);
            emit_LDI(RZL, jvm_operand_byte1);
            emit_CPC(RZL, operand_regs1[1]);
            emit_LDI(RZL, jvm_operand_byte2);
            emit_CPC(RZL, operand_regs1[2]);
            emit_LDI(RZL, jvm_operand_byte3);
            emit_CPC(RZL, operand_regs1[3]);
            emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);

            // Lower than or equal to the upper bound: load the lower bound
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            int32_t lowerbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
            emit_LDI(RZL, jvm_operand_byte0); // Bytecode is big endian
            emit_CP(operand_regs1[0], RZL);
            emit_LDI(RZL, jvm_operand_byte1);
            emit_CPC(operand_regs1[1], RZL);
            emit_LDI(RZL, jvm_operand_byte2);
            emit_CPC(operand_regs1[2], RZL);
            emit_LDI(RZL, jvm_operand_byte3);
            emit_CPC(operand_regs1[3], RZL);
            emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);

            // Also higher than or equal to the lower bound: branch through the switch table
            // Substract lower bound from the key to find the index (assume 16b will be enough)
            if (rtc_stackcache_getfree_16bit_prefer_ge_R16(operand_regs2)) {
                emit_LDI(operand_regs2[0], jvm_operand_byte3);
                emit_LDI(operand_regs2[1], jvm_operand_byte2);
            } else {
                emit_LDI(RZL, jvm_operand_byte3);
                emit_LDI(RZH, jvm_operand_byte2);
                emit_MOVW(operand_regs2[0], RZ);                
            }
            emit_SUB(operand_regs1[0], operand_regs2[0]);
            emit_SBC(operand_regs1[1], operand_regs2[1]);

            // operand_regs1[0]:operand_regs1[1] now contains the index
            // The branch targets may not have consecutive numbers, for example if there are branches within a switch case
            // We'll do a double jump instead, first IJMPing to a table of RJMPs to the branch target table
            // So a total of 3 jmps instead of 2 for a normal branch. This could be optimised a bit by making sure the branch targets
            // are consecutive, which we could enforce in the infuser, but that would only save a few cycles and given the
            // amount of work we're already doing here, it won't speed things up by much, so I can't be bothered.

            emit_RCALL(0); // RCALL to offset 0 does nothing, except get the PC on the stack, which we need here

            emit_POP(RZH); // POP PC into Z (ignore the highest (>128K) byte for now)
            emit_POP(RZH);
            emit_POP(RZL);
            emit_ADIW(RZ, 7); // Will need to compensate here for the instructions inbetween RCALL(0) and the table. Now Z will point to the start of the RJMP table.
            emit_ADD(RZL, operand_regs1[0]); // Add the index to get the target address in the RJMP table
            emit_ADC(RZH, operand_regs1[1]);
            emit_IJMP(); // All this fuss because there's no relative indirect jump...

            // Now emit the RJMP table itself
            for (int i=0; i<(upperbound-lowerbound+1); i++) { // +1 since both bounds are inclusive
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
            rtc_stackcache_getfree_32bit(operand_regs2);

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
                emit_LDI(RZL, jvm_operand_byte0); // Bytecode is big endian
                emit_CP(RZL, operand_regs1[0]);
                emit_LDI(RZL, jvm_operand_byte1);
                emit_CPC(RZL, operand_regs1[1]);
                emit_LDI(RZL, jvm_operand_byte2);
                emit_CPC(RZL, operand_regs1[2]);
                emit_LDI(RZL, jvm_operand_byte3);
                emit_CPC(RZL, operand_regs1[3]);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word0);
            }

            emit_x_branchtag(OPCODE_RJMP, default_branch_target);
        }
        break;
        case JVM_BRTARGET:
            rtc_stackcache_flush_all_regs(); // Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.
            rtc_poppedstackcache_clear_all_valuetags();

            // This is a noop, but we need to record the offset of the next
            // instruction in the branch target table at the start of the method.
            // Record the offset IN WORDS from the method start to make sure it works
            // for addresses > 128K as well. (but this won't work for methods > 128K)
            emit_flush_to_flash(); // Finish writing, and also make sure we won't optimise across basic block boundaries.
            tmp_current_position = wkreprog_get_raw_position();
            wkreprog_close();
            wkreprog_open_raw(rtc_branch_target_table_address(ts->branch_target_table_start_ptr, ts->branch_target_count), ts->end_of_safe_region);
            emit_raw_word(tmp_current_position/2 - ts->branch_target_table_start_ptr/2);
            emit_flush_to_flash();
            wkreprog_close();
            wkreprog_open_raw(tmp_current_position, ts->end_of_safe_region);
            ts->branch_target_count++;
        break;
        case JVM_MARKLOOP_START:
            ts->pc += (2*jvm_operand_byte0)+1;
        break;
        case JVM_MARKLOOP_END:
        break;

        // Not implemented
        default:
            DEBUG_LOG(DBG_RTC, "Unimplemented Java opcode %d at pc=%d\n", opcode, ts->pc);
            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
        break;
    }
    }

    rtc_stackcache_next_instruction();
    ts->pc += rtc_number_of_operandbytes_for_opcode(opcode);
    ts->pc++;
}

#endif // AOT_STRATEGY_POPPEDSTACKCACHE
