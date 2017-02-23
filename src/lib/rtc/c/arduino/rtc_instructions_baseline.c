#if defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)

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

// NOTE: Function pointers are a "PC address", so already divided by 2 since the PC counts in words, not bytes.
// avr-libgcc functions used by translation
extern void __divmodhi4(void);
extern void __mulsi3(void);
extern void __divmodsi4(void);

void rtc_translate_single_instruction() {
    rtc_translationstate *ts = rtc_ts;
    dj_infusion *target_infusion;
    uint_farptr_t tmp_current_position;
    uint8_t offset;
    uint8_t m, n;
    int8_t i;

#ifdef AVRORA
    avroraRTCTraceDarjeelingOpcodeInProgmem(ts->jvm_code_start + ts->pc);
#endif

    uint8_t opcode = dj_di_getU8(ts->jvm_code_start + ts->pc);
#ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
    uint8_t next_opcode = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS
    DEBUG_LOG(DBG_RTCTRACE, "[rtc] JVM opcode %d (pc=%d, method length=%d)\n", opcode, ts->pc, ts->method_length);

    // Load possible operands. May waste some time if we don't need then, but saves some space.
    uint8_t jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
    uint8_t jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ts->pc + 2);
    uint8_t jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ts->pc + 3);
    uint8_t jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ts->pc + 4);
    uint16_t jvm_operand_word0 = (jvm_operand_byte0 << 8) | jvm_operand_byte1;
    uint16_t jvm_operand_word1 = (jvm_operand_byte2 << 8) | jvm_operand_byte3;
    int16_t jvm_operand_signed_word;

    switch (opcode) {
        case JVM_NOP:
        break;
        case JVM_SCONST_M1:
            emit_LDI(R24, 0xFF);
            emit_LDI(R25, 0xFF);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SCONST_1:
#ifdef AOT_OPTIMISE_CONSTANT_SHIFTS_BY1
            if (next_opcode == JVM_SSHL
                || next_opcode == JVM_SSHR
                || next_opcode == JVM_SUSHR
                || next_opcode == JVM_ISHL
                || next_opcode == JVM_ISHR
                || next_opcode == JVM_IUSHR) { // Somehow IUSHR has 16 bit operand but ISHR and ISHL have 32 bit.
                ts->do_CONST_SHIFT_optimisation = 1;
                break;
            }
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS_BY1
        case JVM_SCONST_2:
        case JVM_SCONST_3:
        case JVM_SCONST_4:
        case JVM_SCONST_5:
#if defined(AOT_OPTIMISE_CONSTANT_SHIFTS) && !defined(AOT_OPTIMISE_CONSTANT_SHIFTS_BY1)
            if (next_opcode == JVM_SSHL
                || next_opcode == JVM_SSHR
                || next_opcode == JVM_SUSHR
                || next_opcode == JVM_ISHL
                || next_opcode == JVM_ISHR
                || next_opcode == JVM_IUSHR) {
                ts->do_CONST_SHIFT_optimisation = opcode - JVM_SCONST_0;
                break;
            }
#endif 
        case JVM_SCONST_0:
            emit_LDI(R24, opcode - JVM_SCONST_0); // Operand is implicit in opcode
            emit_LDI(R25, 0);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_ICONST_M1:
            emit_LDI(R22, 0xFF);
            emit_LDI(R23, 0xFF);
            emit_MOVW(R24, R22);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_ICONST_0:
        case JVM_ICONST_1:
        case JVM_ICONST_2:
        case JVM_ICONST_3:
        case JVM_ICONST_4:
        case JVM_ICONST_5:
            emit_LDI(R22, opcode - JVM_ICONST_0); // Operand is implicit in opcode
            emit_LDI(R23, 0);
            emit_LDI(R24, 0);
            emit_LDI(R25, 0);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_ACONST_NULL:
            emit_LDI(R24, 0);
            emit_LDI(R25, 0);
            emit_x_PUSH_REF(R24);
        break;
        case JVM_BSPUSH:
            ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
#if defined(AOT_OPTIMISE_CONSTANT_SHIFTS) && !defined(AOT_OPTIMISE_CONSTANT_SHIFTS_BY1)
            next_opcode = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
            if (next_opcode == JVM_SSHL
                || next_opcode == JVM_SSHR
                || next_opcode == JVM_SUSHR
                || next_opcode == JVM_ISHL
                || next_opcode == JVM_ISHR
                || next_opcode == JVM_IUSHR) {
                ts->do_CONST_SHIFT_optimisation = jvm_operand_byte0;
                break;
            }
#endif
            emit_LDI(R24, jvm_operand_byte0);
            emit_LDI(R25, 0);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_BIPUSH:
            ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            emit_LDI(R22, jvm_operand_byte0);
            emit_LDI(R23, 0);
            emit_LDI(R24, 0);
            emit_LDI(R25, 0);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_SSPUSH:
            // bytecode is big endian
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_LDI(R24, jvm_operand_byte1);
            emit_LDI(R25, jvm_operand_byte0);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SIPUSH:
            // bytecode is big endian
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_LDI(R22, jvm_operand_byte1);
            emit_LDI(R23, jvm_operand_byte0);
            emit_LDI(R24, 0);
            emit_LDI(R25, 0);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_IIPUSH:
            // bytecode is big endian
            ts->pc += 4; // Skip operand (already read into jvm_operand_byte0)
            emit_LDI(R22, jvm_operand_byte3);
            emit_LDI(R23, jvm_operand_byte2);
            emit_LDI(R24, jvm_operand_byte1);
            emit_LDI(R25, jvm_operand_byte0);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_LDS:
            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
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
            emit_x_PUSH_REF(R24);
        break;
        case JVM_SLOAD:
        case JVM_SLOAD_0:
        case JVM_SLOAD_1:
        case JVM_SLOAD_2:
        case JVM_SLOAD_3:
            if (opcode == JVM_SLOAD)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_SLOAD_0;
            emit_LDD(R24, Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(R25, Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0)+1);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_ILOAD:
        case JVM_ILOAD_0:
        case JVM_ILOAD_1:
        case JVM_ILOAD_2:
        case JVM_ILOAD_3:
            if (opcode == JVM_ILOAD)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_ILOAD_0;
            emit_LDD(R22, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(R23, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+1);
            emit_LDD(R24, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+2);
            emit_LDD(R25, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+3);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_ALOAD:
        case JVM_ALOAD_0:
        case JVM_ALOAD_1:
        case JVM_ALOAD_2:
        case JVM_ALOAD_3:
            if (opcode == JVM_ALOAD)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_ALOAD_0;
            emit_LDD(R24, Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0));
            emit_LDD(R25, Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0)+1);
            emit_x_PUSH_REF(R24);
        break;
        case JVM_SSTORE:
        case JVM_SSTORE_0:
        case JVM_SSTORE_1:
        case JVM_SSTORE_2:
        case JVM_SSTORE_3:
            if (opcode == JVM_SSTORE)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_SSTORE_0;
            emit_x_POP_16bit(R24);
            emit_STD(R24, Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0));
            emit_STD(R25, Y, offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0)+1);
        break;
        case JVM_ISTORE:
        case JVM_ISTORE_0:
        case JVM_ISTORE_1:
        case JVM_ISTORE_2:
        case JVM_ISTORE_3:
            if (opcode == JVM_ISTORE)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_ISTORE_0;
            emit_x_POP_32bit(R22);
            emit_STD(R22, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0));
            emit_STD(R23, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+1);
            emit_STD(R24, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+2);
            emit_STD(R25, Y, offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0)+3);
        break;
        case JVM_ASTORE:
        case JVM_ASTORE_0:
        case JVM_ASTORE_1:
        case JVM_ASTORE_2:
        case JVM_ASTORE_3:
            if (opcode == JVM_ASTORE)
                ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            else
                jvm_operand_byte0 = opcode - JVM_ASTORE_0;
            emit_x_POP_REF(R24);
            emit_STD(R24, Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0));
            emit_STD(R25, Y, offset_for_reflocal(ts->methodimpl, jvm_operand_byte0)+1);
        break;
        case JVM_BALOAD:
        case JVM_CALOAD:
        case JVM_SALOAD:
        case JVM_IALOAD:
        case JVM_AALOAD:
#ifdef ARRAYINDEX_32BIT
            emit_x_POP_32bit(R22);

            // POP the array reference into Z.
            emit_x_POP_REF(RZ); // Z now pointer to the base of the array object.

            if (opcode==JVM_SALOAD || opcode==JVM_AALOAD) {
                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit_LSL(R22);
                emit_ROL(R23);
            } else if (opcode==JVM_IALOAD) {
                // Multiply the index by 4, since we're indexing 16 bit shorts.
                emit_LSL(R22);
                emit_ROL(R23);
                emit_LSL(R22);
                emit_ROL(R23);
            }

            // Add (1/2/4)*the index to Z
            emit_ADD(RZL, R22);
            emit_ADC(RZH, R23);
#else
            emit_x_POP_16bit(R24);

            // POP the array reference into Z.
            emit_x_POP_REF(RZ); // Z now pointer to the base of the array object.

            if (opcode==JVM_SALOAD || opcode==JVM_AALOAD) {
                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit_LSL(R24);
                emit_ROL(R25);
            } else if (opcode==JVM_IALOAD) {
                // Multiply the index by 4, since we're indexing 16 bit shorts.
                emit_LSL(R24);
                emit_ROL(R25);
                emit_LSL(R24);
                emit_ROL(R25);
            }

            // Add (1/2/4)*the index to Z
            emit_ADD(RZL, R24);
            emit_ADC(RZH, R25);
#endif

            if (opcode == JVM_AALOAD) {
                // Add 4 to skip 2 bytes for array length and 2 bytes for array type.
                emit_ADIW(RZ, 4); 
            } else { // all types of int array
                // Add 3 to skip 2 bytes for array length and 1 byte for array type.
                emit_ADIW(RZ, 3); 
            }

            // Now Z points to the target element
            switch (opcode) {
                case JVM_BALOAD:
                case JVM_CALOAD:
                    emit_LD_Z(R24);
                    emit_CLR(R25);
                    emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                    emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign
                    emit_x_PUSH_16bit(R24);
                break;
                case JVM_SALOAD:
                    emit_LD_ZINC(R24);
                    emit_LD_Z(R25);
                    emit_x_PUSH_16bit(R24);
                break;
                case JVM_IALOAD:
                    emit_LD_ZINC(R22);
                    emit_LD_ZINC(R23);
                    emit_LD_ZINC(R24);
                    emit_LD_Z(R25);
                    emit_x_PUSH_32bit(R22);
                break;
                case JVM_AALOAD:
                    emit_LD_ZINC(R24);
                    emit_LD_Z(R25);
                    emit_x_PUSH_REF(R24);
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
                    emit_x_POP_16bit(R24);
                break;
                case JVM_IASTORE:
                    emit_x_POP_32bit(R22);
                break;
                case JVM_AASTORE:
                    emit_x_POP_REF(R24);
                break;
            }

#ifdef ARRAYINDEX_32BIT
            emit_x_POP_32bit(R18);
#else
            emit_x_POP_16bit(R18);
#endif

            // POP the array reference into Z.
            emit_x_POP_REF(RZ); // Z now pointer to the base of the array object.

            if (opcode==JVM_SASTORE || opcode==JVM_AASTORE) {
                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit_LSL(R18);
                emit_ROL(R19);
            } else if (opcode==JVM_IASTORE) {
                // Multiply the index by 4, since we're indexing 16 bit shorts.
                emit_LSL(R18);
                emit_ROL(R19);
                emit_LSL(R18);
                emit_ROL(R19);
            }

            // Add (1/2/4)*the index to Z
            emit_ADD(RZL, R18);
            emit_ADC(RZH, R19);

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
                    emit_ST_Z(R24);
                break;
                case JVM_SASTORE:
                case JVM_AASTORE:
                    emit_ST_ZINC(R24);
                    emit_ST_Z(R25);
                break;
                case JVM_IASTORE:
                    emit_ST_ZINC(R22);
                    emit_ST_ZINC(R23);
                    emit_ST_ZINC(R24);
                    emit_ST_Z(R25);
                break;
            }
        break;
        case JVM_IPOP:
            emit_x_POP_16bit(R24);
        break;
        case JVM_IPOP2:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R24);
        break;
        case JVM_IDUP:
            // IDUP duplicates the top one SLOTS on the integer stack, not the top int. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
            emit_x_POP_16bit(R24);
            emit_x_PUSH_16bit(R24);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_IDUP2:
            // IDUP2 duplicates the top two SLOTS on the integer stack, not the top two ints. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
            emit_x_POP_32bit(R22);
            emit_x_PUSH_32bit(R22);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_IDUP_X:
            m = dj_di_getU8(ts->jvm_code_start + ++(ts->pc)); // 
            n = m & 15;
            m >>= 4;
            // m: how many integer slots to duplicate
            // n: how deep to bury them in the stack. (see getIDupInstructions in the infuser)
            // not that the infuser always generated code with n>=m
            // Example for m=1, n=2:
            // ..., v1, v2 -> ..., v2, v1, v2
            if (n == 0 || n > 4) {
                // n == 0 not supported, n>4 also not supported. Could be expanded using more registers, but for now it's not necessary
                dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
            } else {
                for (i = 0; i < n; i++) {
                    // First pop n values
                    emit_x_POP_16bit(R18+(2*i));
                }
                for (i = m-1; i >= 0; i--) { // loop from m-1 back to 0
                    // Then push the m values that need to be duplicated
                    emit_x_PUSH_16bit(R18+(2*i));
                }
                for (i = n-1; i >= 0; i--) { // loop from n-1 back to 0
                    // Finally push the original n values back on the stack
                    emit_x_PUSH_16bit(R18+(2*i));
                }
            }
        break;
        case JVM_APOP:
            emit_x_POP_REF(R24);                
        break;
        case JVM_ADUP:
            emit_x_POP_REF(R24);
            emit_x_PUSH_REF(R24);
            emit_x_PUSH_REF(R24);
        break;
        case JVM_GETFIELD_B:
        case JVM_GETFIELD_C:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_LDD(R24, Z, jvm_operand_word0);

            // need to extend the sign to push it as a short
            emit_CLR(R25);
            emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
            emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign

            emit_x_PUSH_16bit(R24);
        break;
        case JVM_GETFIELD_S:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_LDD(R24, Z, jvm_operand_word0);
            emit_LDD(R25, Z, jvm_operand_word0+1);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_GETFIELD_I:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_LDD(R22, Z, jvm_operand_word0);
            emit_LDD(R23, Z, jvm_operand_word0+1);
            emit_LDD(R24, Z, jvm_operand_word0+2);
            emit_LDD(R25, Z, jvm_operand_word0+3);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_GETFIELD_A:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(R24); // POP the reference

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z
            emit_LDD(R24, Z, (jvm_operand_word0*2)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_LDD(R25, Z, (jvm_operand_word0*2)+1);
            emit_x_PUSH_REF(R24);
        break;
        case JVM_PUTFIELD_B:
        case JVM_PUTFIELD_C:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_16bit(R24);
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_STD(R24, Z, jvm_operand_word0);
        break;
        case JVM_PUTFIELD_S:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_16bit(R24);
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_STD(R24, Z, jvm_operand_word0);
            emit_STD(R25, Z, jvm_operand_word0+1);
        break;
        case JVM_PUTFIELD_I:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_32bit(R22);
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_STD(R22, Z, jvm_operand_word0);
            emit_STD(R23, Z, jvm_operand_word0+1);
            emit_STD(R24, Z, jvm_operand_word0+2);
            emit_STD(R25, Z, jvm_operand_word0+3);
        break;
        case JVM_PUTFIELD_A:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(R16); // POP the value to store (store in in call-saved R16-R17)
            emit_x_POP_REF(R24); // POP the reference to the object to store it in.

            // First find the location of reference fields
            emit_x_CALL((uint16_t)&dj_object_getReferences);

            // R24:R25 now points to the location of the instance references
            emit_MOVW(RZ, R24); // Move the location to Z
            emit_STD(R16, Z, (jvm_operand_word0*2)); // jvm_operand_word0 is an index in the (16 bit) array, so multiply by 2
            emit_STD(R17, Z, (jvm_operand_word0*2)+1);
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
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            // jvm_operand_byte0: the infusion.
            // jvm_operand_byte1: Get the field.
            emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

            if (jvm_operand_byte0 == 0) {
                target_infusion = ts->infusion;
            } else {
                // We need to read from another infusion. Get that infusion's address first.
                // Load the address of the referenced infusion into R24:R25
                offset = rtc_offset_for_referenced_infusion(ts->infusion, jvm_operand_byte0);
                emit_LDD(R24, Z, offset);
                emit_LDD(R25, Z, offset+1);
                // Then move R24:R25 to Z
                emit_MOVW(RZ, R24);
                // Z now points to the target infusion, but it should point to the start of the static variables
                emit_ADIW(RZ, sizeof(dj_infusion));
                // Find the target infusion to calculate the right offset in the next step
                target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
            }
            switch (opcode) {
                case JVM_GETSTATIC_B:
                case JVM_GETSTATIC_C:
                    emit_LDD(R24, Z, rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1));
                    // need to extend the sign to push the byte as a short
                    emit_CLR(R25);
                    emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                    emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign
                    emit_x_PUSH_16bit(R24);
                break;
                case JVM_GETSTATIC_S:
                    offset = rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    emit_LDD(R24, Z, offset);
                    emit_LDD(R25, Z, offset+1);
                    emit_x_PUSH_16bit(R24);
                break;
                case JVM_GETSTATIC_I:
                    offset = rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    emit_LDD(R22, Z, offset);
                    emit_LDD(R23, Z, offset+1);
                    emit_LDD(R24, Z, offset+2);
                    emit_LDD(R25, Z, offset+3);
                    emit_x_PUSH_32bit(R22);
                break;
                case JVM_GETSTATIC_A:
                    offset = rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    emit_LDD(R24, Z, offset);
                    emit_LDD(R25, Z, offset+1);
                    emit_x_PUSH_REF(R24);
                break;
                case JVM_PUTSTATIC_B:
                case JVM_PUTSTATIC_C:
                    emit_x_POP_16bit(R24);
                    emit_STD(R24, Z, rtc_offset_for_static_byte(target_infusion, jvm_operand_byte1));
                break;
                case JVM_PUTSTATIC_S:
                    emit_x_POP_16bit(R24);
                    offset = rtc_offset_for_static_short(target_infusion, jvm_operand_byte1);
                    emit_STD(R24, Z, offset);
                    emit_STD(R25, Z, offset+1);
                break;
                case JVM_PUTSTATIC_I:
                    emit_x_POP_32bit(R22);
                    offset = rtc_offset_for_static_int(target_infusion, jvm_operand_byte1);
                    emit_STD(R22, Z, offset);
                    emit_STD(R23, Z, offset+1);
                    emit_STD(R24, Z, offset+2);
                    emit_STD(R25, Z, offset+3);
                break;
                case JVM_PUTSTATIC_A:
                    emit_x_POP_REF(R24);
                    offset = rtc_offset_for_static_ref(target_infusion, jvm_operand_byte1);
                    emit_STD(R24, Z, offset);
                    emit_STD(R25, Z, offset+1);
                break;
            }
        break;
        case JVM_SADD:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            emit_ADD(R24, R22);
            emit_ADC(R25, R23);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SSUB:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            emit_SUB(R22, R24);
            emit_SBC(R23, R25);
            emit_x_PUSH_16bit(R22);
        break;
        case JVM_SMUL:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);

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

            emit_MUL(R22, R24);
            emit_MOVW(R18, R0);
            emit_MUL(R22, R25);
            emit_ADD(R19, R0);
            emit_MUL(R23, R24);
            emit_ADD(R19, R0);
            emit_CLR(R1);
            emit_MOVW(R24, R18);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SDIV:
        case JVM_SREM:
            emit_x_POP_32bit(R22);
            emit_x_CALL((uint16_t)&__divmodhi4);
            if (opcode == JVM_SDIV) {
                emit_x_PUSH_16bit(R22);
            } else { // JVM_SREM
                emit_x_PUSH_16bit(R24);
            }
        break;
        case JVM_SNEG:
            emit_x_POP_16bit(R24);
            emit_CLR(R22);
            emit_CLR(R23);
            emit_SUB(R22, R24);
            emit_SBC(R23, R25);
            emit_x_PUSH_16bit(R22);
        break;
        case JVM_SSHL:
        case JVM_SSHR:
        case JVM_SUSHR:
            #if defined(AOT_OPTIMISE_CONSTANT_SHIFTS)
            {

                // We have 3 flavors of optimisation for constant bit shifts:
                // _ALL just unrolls the shift loop, which is twice faster than the loop, but increases code size
                // _ALL_GENERATE_MOVS will generate a mov for each multiple of 8 bits, which generates smaller and faster code, but increases code size
                // _GCC_LIKE, which will behave like ALL_GENERATE_MOVS, unless (x>2 && x%8!=0 && ((ISHL && x>8) ISHR || IUSHR)). There's no reason to want to do this, other than making a fair comparison to what's possible in native code, since avr-gcc could do better here.
                // Since the last option only affect the 32 bit versions, we don't need to do anything special here.

                bool emit_loop;
                uint8_t bytes_to_shift; // = ts->do_CONST_SHIFT_optimisation > 0 ? ts->do_CONST_SHIFT_optimisation : 1;

                if (ts->do_CONST_SHIFT_optimisation == 0) {
                    emit_loop = true;
                    bytes_to_shift = 1;
                    emit_x_POP_16bit(R22);
                } else {
                    emit_loop = false;
                    bytes_to_shift = ts->do_CONST_SHIFT_optimisation;
                }

                emit_x_POP_16bit(R24); // pop the operand
                // Emit code. Here we need emit_loop and bytes_to_shift to be set.
                // If emit_loop is true, bytes_to_shift should be 1, and the loop reg should be in operand1[0]
                if (emit_loop) {
                    emit_RJMP(4);
                }
                #if defined(AOT_OPTIMISE_CONSTANT_SHIFTS_ALL_MOVE_AND_SHIFT) || defined (AOT_OPTIMISE_CONSTANT_SHIFTS_GCC_LIKE)
                while (bytes_to_shift >= 8) {
                    if (opcode == JVM_SSHL) {                
                        emit_MOV(R25, R24);
                        emit_CLR(R24);
                    } else if (opcode == JVM_SSHR) {
                        emit_CLR(RZL);
                        emit_SBRC(R25, 7);
                        emit_COM(RZL);
                        emit_MOV(R24, R25);
                        emit_MOV(R25, RZL);
                    } else if (opcode == JVM_SUSHR) {
                        emit_MOV(R24, R25);
                        emit_CLR(R25);
                    }                    
                    bytes_to_shift -= 8;
                }
                #endif
                while (bytes_to_shift > 0) {
                    if (opcode == JVM_SSHL) {
                        emit_LSL(R24);
                        emit_ROL(R25);
                    } else if (opcode == JVM_SSHR) {
                        emit_ASR(R25);
                        emit_ROR(R24);
                    } else if (opcode == JVM_SUSHR) {
                        emit_LSR(R25);
                        emit_ROR(R24);
                    }
                    bytes_to_shift--;
                }
                if (emit_loop) {
                    emit_DEC(R22);
                    emit_BRPL(-8);
                }
                // Emit shift
                emit_x_PUSH_16bit(R24);
                
                // Clear optimisation var if it was set.
                ts->do_CONST_SHIFT_optimisation = 0;
            }
            #else // AOT_OPTIMISE_CONSTANT_SHIFTS
            emit_x_POP_32bit(R22);

            emit_RJMP(4);
            if (opcode == JVM_SSHL) {
                emit_LSL(R24);
                emit_ROL(R25);
            } else if (opcode == JVM_SSHR) {
                emit_ASR(R25);
                emit_ROR(R24);
            } else if (opcode == JVM_SUSHR) {
                emit_LSR(R25);
                emit_ROR(R24);
            }
            emit_DEC(R22);
            emit_BRPL(-8);

            emit_x_PUSH_16bit(R24);
            #endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        break;
        case JVM_SAND:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            emit_AND(R24, R22);
            emit_AND(R25, R23);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SOR:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            emit_OR(R24, R22);
            emit_OR(R25, R23);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_SXOR:
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            emit_EOR(R24, R22);
            emit_EOR(R25, R23);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_IADD:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_ADD(R22, R18);
            emit_ADC(R23, R19);
            emit_ADC(R24, R20);
            emit_ADC(R25, R21);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_ISUB:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_SUB(R18, R22);
            emit_SBC(R19, R23);
            emit_SBC(R20, R24);
            emit_SBC(R21, R25);
            emit_x_PUSH_32bit(R18);
        break;
        case JVM_IMUL:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_x_CALL((uint16_t)&__mulsi3);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_IDIV:
        case JVM_IREM:
            emit_x_POP_32bit(R18);
            emit_x_POP_32bit(R22);
            emit_x_CALL((uint16_t)&__divmodsi4);
            if (opcode == JVM_IDIV) {
                emit_x_PUSH_32bit(R18);
            } else { // JVM_IREM
                emit_x_PUSH_32bit(R22);
            }
        break;
        case JVM_INEG:
            emit_x_POP_32bit(R22);
            emit_CLR(R18);
            emit_CLR(R19);
            emit_MOVW(R20, R18);
            emit_SUB(R18, R22);
            emit_SBC(R19, R23);
            emit_SBC(R20, R24);
            emit_SBC(R21, R25);
            emit_x_PUSH_32bit(R18);
        break;
        case JVM_ISHL:
        case JVM_ISHR:
        case JVM_IUSHR:
            #if defined(AOT_OPTIMISE_CONSTANT_SHIFTS)
            {

                // We have 3 flavors of optimisation for constant bit shifts:
                // _ALL just unrolls the shift loop, which is twice faster than the loop, but increases code size
                // _ALL_GENERATE_MOVS will generate a mov for each multiple of 8 bits, which generates smaller and faster code, but increases code size
                // _GCC_LIKE, which will behave like ALL_GENERATE_MOVS, unless (x>2 && x%8!=0 && ((ISHL && x>8) ISHR || IUSHR)). There's no reason to want to do this, other than making a fair comparison to what's possible in native code, since avr-gcc could do better here.

                bool emit_loop;
                uint8_t bytes_to_shift; // = ts->do_CONST_SHIFT_optimisation > 0 ? ts->do_CONST_SHIFT_optimisation : 1;

                #ifdef AOT_OPTIMISE_CONSTANT_SHIFTS_GCC_LIKE
                bytes_to_shift = ts->do_CONST_SHIFT_optimisation;
                if (bytes_to_shift>2 && bytes_to_shift%8!=0 && ((opcode==JVM_ISHL && bytes_to_shift>8) || opcode==JVM_ISHR || opcode==JVM_IUSHR)) {
                    // We should do the loop here because that's what gcc does, even though this is less efficient.
                    // Load the constant in RZ and emit the loop as usual.
                    emit_LDI(R18, bytes_to_shift);
                    ts->do_CONST_SHIFT_optimisation = 0;
                    emit_loop = true;
                    bytes_to_shift = 1;
                } else if(bytes_to_shift == 0) {
                    emit_loop = true;
                    bytes_to_shift = 1;
                    emit_x_POP_16bit(R18);
                } else {
                    emit_loop = false;
                    bytes_to_shift = ts->do_CONST_SHIFT_optimisation;
                }
                #else
                if (ts->do_CONST_SHIFT_optimisation == 0) {
                    emit_loop = true;
                    bytes_to_shift = 1;
                    emit_x_POP_16bit(R18);
                } else {
                    emit_loop = false;
                    bytes_to_shift = ts->do_CONST_SHIFT_optimisation;
                }
                #endif

                emit_x_POP_32bit(R22); // pop the operand
                // Emit code. Here we need emit_loop and bytes_to_shift to be set.
                // If emit_loop is true, bytes_to_shift should be 1, and the loop reg should be in operand1[0]
                if (emit_loop) {
                    emit_RJMP(8);
                }
                #if defined(AOT_OPTIMISE_CONSTANT_SHIFTS_ALL_MOVE_AND_SHIFT) || defined (AOT_OPTIMISE_CONSTANT_SHIFTS_GCC_LIKE)
                while (bytes_to_shift >= 8) {
                    if (opcode == JVM_ISHL) {                
                        emit_MOV(R25, R24);
                        emit_MOV(R24, R23);
                        emit_MOV(R23, R22);
                        emit_CLR(R22);
                    } else if (opcode == JVM_ISHR) {
                        emit_CLR(RZL);
                        emit_SBRC(R25, 7);
                        emit_COM(RZL);
                        emit_MOV(R22, R23);
                        emit_MOV(R23, R24);
                        emit_MOV(R24, R25);
                        emit_MOV(R25, RZL);
                    } else if (opcode == JVM_IUSHR) {
                        emit_MOV(R22, R23);
                        emit_MOV(R23, R24);
                        emit_MOV(R24, R25);
                        emit_CLR(R25);
                    }                    
                    bytes_to_shift -= 8;
                }
                #endif
                while (bytes_to_shift > 0) {
                    if (opcode == JVM_ISHL) {                
                        emit_LSL(R22);
                        emit_ROL(R23);
                        emit_ROL(R24);
                        emit_ROL(R25);
                    } else if (opcode == JVM_ISHR) {
                        emit_ASR(R25);
                        emit_ROR(R24);
                        emit_ROR(R23);
                        emit_ROR(R22);
                    } else if (opcode == JVM_IUSHR) {
                        emit_LSR(R25);
                        emit_ROR(R24);
                        emit_ROR(R23);
                        emit_ROR(R22);
                    }
                    bytes_to_shift--;
                }
                if (emit_loop) {
                    emit_DEC(R18);
                    emit_BRPL(-12);
                }
                // Emit shift
                emit_x_PUSH_32bit(R22);
                
                // Clear optimisation var if it was set.
                ts->do_CONST_SHIFT_optimisation = 0;
            }
            #else // AOT_OPTIMISE_CONSTANT_SHIFTS
            emit_x_POP_16bit(R18);
            emit_x_POP_32bit(R22);

            emit_RJMP(8);
            if (opcode == JVM_ISHL) {                
                emit_LSL(R22);
                emit_ROL(R23);
                emit_ROL(R24);
                emit_ROL(R25);
            } else if (opcode == JVM_ISHR) {
                emit_ASR(R25);
                emit_ROR(R24);
                emit_ROR(R23);
                emit_ROR(R22);
            } else if (opcode == JVM_IUSHR) {
                emit_LSR(R25);
                emit_ROR(R24);
                emit_ROR(R23);
                emit_ROR(R22);
            }
            emit_DEC(R18);
            emit_BRPL(-12);

            emit_x_PUSH_32bit(R22);
            #endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        break;
        case JVM_IAND:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_AND(R22, R18);
            emit_AND(R23, R19);
            emit_AND(R24, R20);
            emit_AND(R25, R21);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_IOR:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_OR(R22, R18);
            emit_OR(R23, R19);
            emit_OR(R24, R20);
            emit_OR(R25, R21);
            emit_x_PUSH_32bit(R22);
        break;
        case JVM_IXOR:
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            emit_EOR(R22, R18);
            emit_EOR(R23, R19);
            emit_EOR(R24, R20);
            emit_EOR(R25, R21);
            emit_x_PUSH_32bit(R22);
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
                ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            } else {
                jvm_operand_signed_word = (int16_t)(((uint16_t)jvm_operand_byte1 << 8) + jvm_operand_byte2);
                ts->pc += 3; // Skip operand (already read into jvm_operand_byte0)
            }
            offset = offset_for_intlocal_short(ts->methodimpl, jvm_operand_byte0);
            if (jvm_operand_signed_word == 1) {
                // Special case
                emit_LDD(R22, Y, offset);
                emit_INC(R22);
                emit_STD(R22, Y, offset);
                emit_BRNE(6);
                emit_LDD(R22, Y, offset+1);
                emit_INC(R22);
                emit_STD(R22, Y, offset+1);
            } else {
                emit_LDD(R22, Y, offset);
                emit_LDD(R23, Y, offset+1);
                if (jvm_operand_signed_word > 0) {
                    // Positive operand
                    emit_SUBI(R22, -(jvm_operand_signed_word & 0xFF));
                    emit_SBCI(R23, -((jvm_operand_signed_word >> 8) & 0xFF)-1);
                } else {
                    // Negative operand
                    emit_SUBI(R22, (-jvm_operand_signed_word) & 0xFF);
                    emit_SBCI(R23, ((-jvm_operand_signed_word) >> 8) & 0xFF);
                }
                emit_STD(R22, Y, offset);
                emit_STD(R23, Y, offset+1);
            }
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
                ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            } else {
                jvm_operand_signed_word = (int16_t)(((uint16_t)jvm_operand_byte1 << 8) + jvm_operand_byte2);
                ts->pc += 3; // Skip operand (already read into jvm_operand_byte0)
            }
            offset = offset_for_intlocal_int(ts->methodimpl, jvm_operand_byte0);
            if (jvm_operand_signed_word == 1) {
                // Special case
                emit_LDD(R22, Y, offset);
                emit_INC(R22);
                emit_STD(R22, Y, offset);
                emit_BRNE(22);
                emit_LDD(R22, Y, offset+1);
                emit_INC(R22);
                emit_STD(R22, Y, offset+1);
                emit_BRNE(14);
                emit_LDD(R22, Y, offset+2);
                emit_INC(R22);
                emit_STD(R22, Y, offset+2);
                emit_BRNE(6);
                emit_LDD(R22, Y, offset+3);
                emit_INC(R22);
                emit_STD(R22, Y, offset+3);
            } else {
                emit_LDD(R22, Y, offset);
                emit_LDD(R23, Y, offset+1);
                emit_LDD(R24, Y, offset+2);
                emit_LDD(R25, Y, offset+3);
                if (jvm_operand_signed_word > 0) {
                    // Positive operand
                    emit_SUBI(R22, -(jvm_operand_signed_word & 0xFF));
                    emit_SBCI(R23, -((jvm_operand_signed_word >> 8) & 0xFF)-1);
                    emit_SBCI(R24, -1);
                    emit_SBCI(R25, -1);
                } else {
                    // Negative operand
                    emit_SUBI(R22, (-jvm_operand_signed_word) & 0xFF);
                    emit_SBCI(R23, ((-jvm_operand_signed_word) >> 8) & 0xFF);
                    emit_SBCI(R24, 0);
                    emit_SBCI(R25, 0);
                }
                emit_STD(R22, Y, offset);
                emit_STD(R23, Y, offset+1);
                emit_STD(R24, Y, offset+2);
                emit_STD(R25, Y, offset+3);
            }
        break;
        case JVM_S2B:
        case JVM_S2C:
            emit_x_POP_16bit(R24);

            // need to extend the sign
            emit_CLR(R25);
            emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
            emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign

            emit_x_PUSH_16bit(R24);
        break;
        case JVM_S2I:
            emit_x_POP_16bit(R22);

            // need to extend the sign
            emit_CLR(R24);
            emit_SBRC(R23, 7); // highest bit of MSB R23 cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
            emit_COM(R24); // otherwise: flip R24 to 0xFF to extend the sign
            emit_MOV(R25, R24);

            emit_x_PUSH_32bit(R22);
        break;
        case JVM_I2S:
            emit_x_POP_32bit(R22);
            emit_x_PUSH_16bit(R22);
        break;
        case JVM_SRETURN:
            emit_x_POP_16bit(R24);
            emit_x_epilogue();
        break;
        case JVM_IRETURN:
            emit_x_POP_32bit(R22);
            emit_x_epilogue();
        break;
        case JVM_ARETURN:
            emit_x_POP_REF(R24); // POP the reference into Z
            emit_x_epilogue();
        break;
        case JVM_RETURN:
            emit_x_epilogue();
        break;
        case JVM_INVOKEVIRTUAL:
        case JVM_INVOKESPECIAL:
        case JVM_INVOKESTATIC:
        case JVM_INVOKEINTERFACE:
            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)

            if (opcode == JVM_INVOKEVIRTUAL || opcode == JVM_INVOKEINTERFACE) {
                jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            }

            rtc_common_translate_invoke(ts, opcode, jvm_operand_byte0, jvm_operand_byte1, jvm_operand_byte2);
        break;
        case JVM_NEW:
            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
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
            emit_x_PUSH_REF(R24);
        break;
        case JVM_NEWARRAY:
            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


            ts->pc += 1; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_16bit(R22); // size
            emit_LDI(R24, jvm_operand_byte0); // (int) element type
            emit_x_CALL((uint16_t)&dj_int_array_create);


            // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
            emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
            emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
            // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
            emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
            emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


            // push the reference to the new object onto the ref stack
            emit_x_PUSH_REF(R24);
        break;
        case JVM_ANEWARRAY:
            // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
            emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
            emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_16bit(R22); // size
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
            emit_x_PUSH_REF(R24);
        break;
        case JVM_ARRAYLENGTH: // The length of an array is stored as 16 bit at the start of the array
            emit_x_POP_REF(RZ); // POP the reference into Z
            emit_LD_ZINC(R24);
            emit_LD_Z(R25);
            emit_x_PUSH_16bit(R24);
        break;
        case JVM_CHECKCAST:
            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY

            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(R22); // reference to the object
            emit_x_PUSH_REF(R22); // TODO: optimise this. CHECKCAST should only peek.
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id

            emit_x_CALL((uint16_t)&RTC_CHECKCAST);
        break;
        case JVM_INSTANCEOF:
            // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY

            ts->pc += 2; // Skip operand (already read into jvm_operand_byte0)
            emit_x_POP_REF(R22); // reference to the object
            emit_LDI(R24, jvm_operand_byte0); // infusion id
            emit_LDI(R25, jvm_operand_byte1); // entity id

            emit_x_CALL((uint16_t)&RTC_INSTANCEOF);

            // push the result onto the stack
            emit_x_PUSH_16bit(R24);
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
            ts->pc += 4;
            emit_x_POP_16bit(R24);
            if (opcode == JVM_SIFEQ) {
                emit_OR(R24, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_SIFNE) {
                emit_OR(R24, R25);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_SIFLT) {
                emit_CP(R25, ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_SIFGE) {
                emit_CP(R25, ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_SIFGT) {
                emit_CP(ZERO_REG, R24);
                emit_CPC(ZERO_REG, R25);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_SIFLE) {
                emit_CP(ZERO_REG, R24);
                emit_CPC(ZERO_REG, R25);
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
            ts->pc += 4;

            emit_x_POP_32bit(R22);
            if (opcode == JVM_IIFEQ) {
                emit_OR(R22, R23);
                emit_OR(R22, R24);
                emit_OR(R22, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IIFNE) {
                emit_OR(R22, R23);
                emit_OR(R22, R24);
                emit_OR(R22, R25);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_IIFLT) {
                emit_CP(R25, ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IIFGE) {
                emit_CP(R25, ZERO_REG); // Only need to consider the highest byte to decide < 0 or >= 0
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IIFGT) {
                emit_CP(ZERO_REG, R22);
                emit_CPC(ZERO_REG, R23);
                emit_CPC(ZERO_REG, R24);
                emit_CPC(ZERO_REG, R25);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IIFLE) {
                emit_CP(ZERO_REG, R22);
                emit_CPC(ZERO_REG, R23);
                emit_CPC(ZERO_REG, R24);
                emit_CPC(ZERO_REG, R25);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IFNULL:
        case JVM_IFNONNULL:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;

            emit_x_POP_REF(R24);
            if (opcode == JVM_IFNULL) {
                emit_OR(R24, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IFNONNULL) {
                emit_OR(R24, R25);
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
            ts->pc += 4;
            emit_x_POP_16bit(R24);
            emit_x_POP_16bit(R22);
            // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
            if (opcode == JVM_IF_SCMPEQ) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPNE) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPLT) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPGE) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPGT) {
                emit_CP(R24, R22);
                emit_CPC(R25, R23);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_SCMPLE) {
                emit_CP(R24, R22);
                emit_CPC(R25, R23);
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
            ts->pc += 4;
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
            if (opcode == JVM_IF_ICMPEQ) {
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPNE) {
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPLT) {
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPGE) {
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPGT) {
                emit_CP(R22, R18);
                emit_CPC(R23, R19);
                emit_CPC(R24, R20);
                emit_CPC(R25, R21);
                emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);
            } else if (opcode == JVM_IF_ICMPLE) {
                emit_CP(R22, R18);
                emit_CPC(R23, R19);
                emit_CPC(R24, R20);
                emit_CPC(R25, R21);
                emit_x_branchtag(OPCODE_BRGE, jvm_operand_word1);
            }
        break;
        case JVM_IF_ACMPEQ:
        case JVM_IF_ACMPNE:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;
            emit_x_POP_REF(R24);
            emit_x_POP_REF(R22);
            if (opcode == JVM_IF_ACMPEQ) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word1);
            } else if (opcode == JVM_IF_ACMPNE) {
                emit_CP(R22, R24);
                emit_CPC(R23, R25);
                emit_x_branchtag(OPCODE_BRNE, jvm_operand_word1);
            }
        break;
        case JVM_GOTO:
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;

            emit_x_branchtag(OPCODE_RJMP, jvm_operand_word1);
        break;
        case JVM_TABLESWITCH: {
            // Branch instructions first have a bytecode offset, used by the interpreter, (in jvm_operand_word0)
            // followed by a branch target index used when compiling to native code. (in jvm_operand_word1)
            ts->pc += 4;

            // Pop the key value
            emit_x_POP_32bit(R22);
            // Load the upper bound
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            int32_t upperbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
            emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
            emit_LDI(R20, jvm_operand_byte1);
            emit_LDI(R19, jvm_operand_byte2);
            emit_LDI(R18, jvm_operand_byte3);
            emit_CP(R18, R22);
            emit_CPC(R19, R23);
            emit_CPC(R20, R24);
            emit_CPC(R21, R25);
            emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);

            // Lower than or equal to the upper bound: load the lower bound
            jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte1 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte2 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            jvm_operand_byte3 = dj_di_getU8(ts->jvm_code_start + ++(ts->pc));
            int32_t lowerbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
            emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
            emit_LDI(R20, jvm_operand_byte1);
            emit_LDI(R19, jvm_operand_byte2);
            emit_LDI(R18, jvm_operand_byte3);
            emit_CP(R22, R18);
            emit_CPC(R23, R19);
            emit_CPC(R24, R20);
            emit_CPC(R25, R21);
            emit_x_branchtag(OPCODE_BRLT, jvm_operand_word1);

            // Also higher than or equal to the lower bound: branch through the switch table
            // Substract lower bound from the key to find the index (assume 16b will be enough)
            emit_SUB(R22, R18);
            emit_SBC(R23, R19);

            // R22:R23 now contains the index
            // The branch targets may not have consecutive numbers, for example if there are branches within a switch case
            // We'll do a double jump instead, first IJMPing to a table of RJMPs to the branch target table
            // So a total of 3 jmps instead of 2 for a normal branch. This could be optimised a bit by making sure the branch targets
            // are consecutive, which we could enforce in the infuser, but that would only save a few cycles and given the
            // amount of work we're already doing here, it won't speed things up by much, so I can't be bothered.

            emit_RCALL(0); // RCALL to offset 0 does nothing, except get the PC on the stack, which we need here

            emit_POP(RZH); // POP PC into Z
            emit_POP(RZL);
            emit_ADIW(RZ, 7); // Will need to compensate here for the instructions inbetween RCALL(0) and the table. Now Z will point to the start of the RJMP table.
            emit_ADD(RZL, R22); // Add the index to get the target address in the RJMP table
            emit_ADC(RZH, R23);
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

            // Pop the key value
            emit_x_POP_32bit(R22);

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
                emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
                emit_LDI(R20, jvm_operand_byte1);
                emit_LDI(R19, jvm_operand_byte2);
                emit_LDI(R18, jvm_operand_byte3);
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_x_branchtag(OPCODE_BREQ, jvm_operand_word0);
            }

            emit_x_branchtag(OPCODE_RJMP, default_branch_target);
        }
        break;
        case JVM_BRTARGET:
            // This is a noop, but we need to record the offset of the next
            // instruction in the branch target table at the start of the method.
            // Record the offset IN WORDS from the method start to make sure it works
            // for addresses > 128K as well. (but this won't work for methods > 128K)
            emit_flush_to_flash(); // Finish writing, and also make sure we won't optimise across basic block boundaries.
            tmp_current_position = wkreprog_get_raw_position();
            wkreprog_close();
            wkreprog_open_raw(rtc_branch_target_table_1_address(ts, ts->branch_target_count), RTC_END_OF_COMPILED_CODE_SPACE);
            emit_raw_word(tmp_current_position/2 - ts->branch_target_table_start_ptr/2);
            emit_flush_to_flash();
            wkreprog_close();
            wkreprog_open_raw(tmp_current_position, RTC_END_OF_COMPILED_CODE_SPACE);
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

    ts->pc++;
}

#endif // defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
