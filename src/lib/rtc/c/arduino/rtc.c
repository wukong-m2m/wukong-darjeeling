#include "types.h"
#include "panic.h"
#include "debug.h"
#include "execution.h"
#include "parse_infusion.h"
#include "infusion.h"
#include "array.h"
#include "object.h"
#include "wkreprog.h"
#include "asm.h"
#include "opcodes.h"
#include "rtc.h"
#include "rtc_instructions.h"
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>

// PUSHREF
#define emit_x_PUSHREF(reg)              emit_ST_XINC(reg)
#define emit_x_POPREF(reg)               emit_LD_DECX(reg)

// Offsets for static variables in an infusion, relative to the start of infusion->staticReferencesFields. (referenced infusion pointers follow the static variables)
#define offset_for_static_ref(infusion_ptr, variable_index)                 ((uint16_t)((void*)(&((infusion_ptr)->staticReferenceFields[variable_index])) - (void *)((infusion_ptr)->staticReferenceFields)))
#define offset_for_static_byte(infusion_ptr, variable_index)                ((uint16_t)((void*)(&((infusion_ptr)->staticByteFields[variable_index]))      - (void *)((infusion_ptr)->staticReferenceFields)))
#define offset_for_static_short(infusion_ptr, variable_index)               ((uint16_t)((void*)(&((infusion_ptr)->staticShortFields[variable_index]))     - (void *)((infusion_ptr)->staticReferenceFields)))
#define offset_for_static_int(infusion_ptr, variable_index)                 ((uint16_t)((void*)(&((infusion_ptr)->staticIntFields[variable_index]))       - (void *)((infusion_ptr)->staticReferenceFields)))
#define offset_for_static_long(infusion_ptr, variable_index)                ((uint16_t)((void*)(&((infusion_ptr)->staticLongFields[variable_index]))      - (void *)((infusion_ptr)->staticReferenceFields)))
#define offset_for_referenced_infusion(infusion_ptr, ref_inf)               ((uint16_t)((void*)(&((infusion_ptr)->referencedInfusions[ref_inf-1]))        - (void *)((infusion_ptr)->staticReferenceFields)))

                             // +---------------------------+
                             // |1             int1 16b     | stackLocalIntegerOffset
                             // |2 local ints  int2 32b,msb |      ^
                             // |3             int2 32b,lsb |      | getReferenceLocalVariableCount*sizeof(ref_t)
                             // +---------------------------+      | + (getIntegerLocalVariableCount-1)*sizeof(int16_t)
                             // |                           |      |
                             // |2 local refs  ref2         |      v
                             // |1             ref1         | getLocalRefVariables, stackEndOffset
                             // +---------------------------+

uint8_t offset_for_intlocal_short(dj_di_pointer methodimpl, uint8_t local) {
    uint32_t offset = (dj_di_methodImplementation_getReferenceLocalVariableCount(methodimpl) * sizeof(ref_t))
                        + ((dj_di_methodImplementation_getIntegerLocalVariableCount(methodimpl)-1) * sizeof(int16_t))
                        - (local * sizeof(int16_t));
    if (offset > 63)
        dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
    return offset;
}

uint8_t offset_for_intlocal_int(dj_di_pointer methodimpl, uint8_t local) {
    // Local integer slots grow down, but the bytecode will point at the slot with the lowest index, which is the top one.
    // For example, look at the 32bit short "int2" in the drawing above. The bytecode will indicate slot 2 as the start,
    // since the 32 bit int is stored in slots 3 and 2. However, slot 3's address is the start of the int in memory,
    // so we need to substract one slot from the pointer.
    return offset_for_intlocal_short(methodimpl, local) - 1*sizeof(int16_t);
}

uint8_t offset_for_intlocal_long(dj_di_pointer methodimpl, uint8_t local) {
    // Same as for ints, only need to substract 3 slots since a long occupies 4.
    return offset_for_intlocal_short(methodimpl, local) - 3*sizeof(int16_t);
}

uint8_t offset_for_reflocal(dj_di_pointer methodimpl, uint8_t local) {
    return (local * sizeof(ref_t));
}


// NOTE: Function pointers are a "PC address", so already divided by 2 since the PC counts in words, not bytes.
// avr-libgcc functions used by translation
extern void __divmodhi4(void);
extern void __mulsi3(void);
extern void __divmodsi4(void);

// the stack pointers used by execution.c
extern int16_t *intStack;
extern ref_t *refStack;
extern ref_t *localReferenceVariables;

// USED AT COMPILE TIME:
const unsigned char PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) rtc_compiled_code_buffer[RTC_COMPILED_CODE_BUFFER_SIZE] = {};
// Buffer for emitting code.
#define RTC_CODEBUFFER_SIZE 64
uint16_t *rtc_codebuffer;
uint16_t *rtc_codebuffer_position; // A pointer to somewhere within the buffer


void rtc_flush() {
    uint8_t *instructiondata = (uint8_t *)rtc_codebuffer;
    uint16_t count = rtc_codebuffer_position - rtc_codebuffer;
#ifdef DARJEELING_DEBUG
    for (int i=0; i<count; i++) {
        DEBUG_LOG(DBG_RTCTRACE, "[rtc]    %x  (%x %x)\n", rtc_codebuffer[i], instructiondata[i*2], instructiondata[i*2+1]);
    }
#endif // DARJEELING_DEBUG
    // Write to flash
    wkreprog_write(2*count, instructiondata);
    // Buffer is now empty
    rtc_codebuffer_position = rtc_codebuffer;
}

static void do_emit(uint16_t opcode) {
    *(rtc_codebuffer_position++) = opcode;

    if (rtc_codebuffer_position >= rtc_codebuffer+RTC_CODEBUFFER_SIZE) // Buffer full, do a flush.
        rtc_flush();    
}

static void emit(uint16_t opcode) {
#ifdef AVRORA
    avroraRTCTraceSingleWordInstruction(opcode);
#endif
    do_emit(opcode);
}

static void emit2(uint16_t opcode1, uint16_t opcode2) {
#ifdef AVRORA
    avroraRTCTraceDoubleWordInstruction(opcode1, opcode2);
#endif
    do_emit(opcode1);
    do_emit(opcode2);
}

void rtc_update_method_pointers(dj_infusion *infusion, native_method_function_t *rtc_method_start_addresses) {
    DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
    uint16_t native_handlers_address = (uint16_t)infusion->native_handlers;
    wkreprog_open_raw(native_handlers_address);

    uint16_t number_of_methodimpls = dj_di_parentElement_getListSize(infusion->methodImplementationList);

    for (uint16_t i=0; i<number_of_methodimpls; i++) {
        dj_di_pointer methodimpl = dj_infusion_getMethodImplementation(infusion, i);
        native_method_function_t handler;
        if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_NATIVE) {
            // Copy existing pointer
            const DJ_PROGMEM native_method_function_t *native_handlers = infusion->native_handlers;
            handler = native_handlers[i];
            DEBUG_LOG(DBG_RTC, "[rtc] method %d is native, copying native handler: %p\n", i, handler);
        } else {
            // Fill in address of RTC compiled method
            handler = rtc_method_start_addresses[i];
            DEBUG_LOG(DBG_RTC, "[rtc] method %d is not native, filling in address from rtc buffer: %p\n", i, handler);
        }
        wkreprog_write(2, (uint8_t *)&handler);
    }

    wkreprog_close();
}


#define rtc_branch_table_size(methodimpl) (dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl)*SIZEOF_RJMP)
#define rtc_branch_target_table_address(i) (branch_target_table_start_ptr + i*SIZEOF_RJMP)
void rtc_compile_method(dj_di_pointer methodimpl, dj_infusion *infusion) {
    uint8_t jvm_operand_byte0;
    uint8_t jvm_operand_byte1;
    uint8_t jvm_operand_byte2;
    uint8_t jvm_operand_byte3;
    uint16_t jvm_operand_word;
    int16_t jvm_operand_signed_word;
    dj_infusion *target_infusion;
    dj_di_pointer tmp_current_position; // Used to temporarily store the current position when processing brtarget instructions.

    uint16_t branch_target_count = 0; // Keep track of how many branch targets we've seen

    // Buffer to hold the code we're building (want to keep this on the stack so it doesn't take up space at runtime)
    uint16_t codebuffer[RTC_CODEBUFFER_SIZE];
    rtc_codebuffer = codebuffer;
    rtc_codebuffer_position = codebuffer;

    // Reserve space for the branch table
    uint16_t branchTableSize = rtc_branch_table_size(methodimpl);
    // Remember the start of the branch table
    dj_di_pointer branch_target_table_start_ptr = wkreprog_get_raw_position();
    DEBUG_LOG(DBG_RTC, "[rtc] Reserving %d bytes for %d branch targets at address %p\n", branchTableSize, dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl), branch_target_table_start_ptr);
    // Skip this number of bytes
    wkreprog_skip(branchTableSize);

    // prologue (is this the right way?)
    emit_PUSH(R3);
    emit_PUSH(R2);
    emit_PUSH(R29); // Push Y
    emit_PUSH(R28);
    emit_MOVW(R28, R24); // Pointer to locals in Y
    emit_MOVW(R26, R22); // Pointer to ref stack in X
    emit_MOVW(R2, R20); // Pointer to static in R2 (will be MOVWed to R30 when necessary)

    // translate the method
    dj_di_pointer code = dj_di_methodImplementation_getData(methodimpl);
    uint16_t method_length = dj_di_methodImplementation_getLength(methodimpl);
    DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", method_length);

    for (uint16_t pc=0; pc<method_length; pc++) {
        uint8_t opcode = dj_di_getU8(code + pc);
        DEBUG_LOG(DBG_RTCTRACE, "[rtc] JVM opcode %d (pc=%d, method length=%d)\n", opcode, pc, method_length);
#ifdef AVRORA
        avroraRTCTraceJavaOpcode(opcode);
#endif
        switch (opcode) {
            case JVM_NOP:
            break;
            case JVM_SCONST_M1:
                emit_LDI(R25, 0xFF);
                emit_PUSH(R25);
                emit_PUSH(R25);
            break;
            case JVM_SCONST_0:
            case JVM_SCONST_1:
            case JVM_SCONST_2:
            case JVM_SCONST_3:
            case JVM_SCONST_4:
            case JVM_SCONST_5:
                jvm_operand_byte0 = opcode - JVM_SCONST_0;
                emit_LDI(R24, jvm_operand_byte0);
                emit_LDI(R25, 0);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_ICONST_M1:
                emit_LDI(R25, 0xFF);
                emit_PUSH(R25);
                emit_PUSH(R25);
                emit_PUSH(R25);
                emit_PUSH(R25);
            break;
            case JVM_ICONST_0:
            case JVM_ICONST_1:
            case JVM_ICONST_2:
            case JVM_ICONST_3:
            case JVM_ICONST_4:
            case JVM_ICONST_5:
                jvm_operand_byte0 = opcode - JVM_ICONST_0;
                emit_LDI(R24, jvm_operand_byte0);
                emit_LDI(R25, 0);
                emit_PUSH(R25);
                emit_PUSH(R25);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_ACONST_NULL:
                emit_LDI(R25, 0);
                emit_x_PUSHREF(R25);                
                emit_x_PUSHREF(R25);                
            break;
            case JVM_BSPUSH:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                emit_LDI(R24, jvm_operand_byte0);
                emit_LDI(R25, 0);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_BIPUSH:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                emit_LDI(R22, jvm_operand_byte0);
                emit_LDI(R23, 0);
                emit_LDI(R24, 0);
                emit_LDI(R25, 0);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_SSPUSH:
                // bytecode is big endian
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_LDI(R24, jvm_operand_byte1);
                emit_LDI(R25, jvm_operand_byte0);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SIPUSH:
                // bytecode is big endian
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_LDI(R22, jvm_operand_byte1);
                emit_LDI(R23, jvm_operand_byte0);
                emit_LDI(R24, 0);
                emit_LDI(R25, 0);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IIPUSH:
                // bytecode is big endian
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                jvm_operand_byte3 = dj_di_getU8(code + ++pc);
                emit_LDI(R22, jvm_operand_byte3);
                emit_LDI(R23, jvm_operand_byte2);
                emit_LDI(R24, jvm_operand_byte1);
                emit_LDI(R25, jvm_operand_byte0);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_LDS:
                // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
                emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id
                emit_2_CALL((uint16_t)&RTC_LDS);


                // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
                emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
                // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
                emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


                // push the reference to the string onto the ref stack
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_SLOAD:
            case JVM_SLOAD_0:
            case JVM_SLOAD_1:
            case JVM_SLOAD_2:
            case JVM_SLOAD_3:
                if (opcode == JVM_SLOAD)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_SLOAD_0;
                emit_LDD(R24, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0));
                emit_LDD(R25, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)+1);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_ILOAD:
            case JVM_ILOAD_0:
            case JVM_ILOAD_1:
            case JVM_ILOAD_2:
            case JVM_ILOAD_3:
                if (opcode == JVM_ILOAD)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_ILOAD_0;
                emit_LDD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0));
                emit_LDD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1);
                emit_LDD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2);
                emit_LDD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_ALOAD:
            case JVM_ALOAD_0:
            case JVM_ALOAD_1:
            case JVM_ALOAD_2:
            case JVM_ALOAD_3:
                if (opcode == JVM_ALOAD)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_ALOAD_0;
                emit_LDD(R24, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0));
                emit_LDD(R25, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0)+1);
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_SSTORE:
            case JVM_SSTORE_0:
            case JVM_SSTORE_1:
            case JVM_SSTORE_2:
            case JVM_SSTORE_3:
                if (opcode == JVM_SSTORE)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_SSTORE_0;
                emit_POP(R24);
                emit_POP(R25);
                emit_STD(R24, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0));
                emit_STD(R25, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)+1);
            break;
            case JVM_ISTORE:
            case JVM_ISTORE_0:
            case JVM_ISTORE_1:
            case JVM_ISTORE_2:
            case JVM_ISTORE_3:
                if (opcode == JVM_ISTORE)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_ISTORE_0;
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_STD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0));
                emit_STD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1);
                emit_STD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2);
                emit_STD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3);
            break;
            case JVM_ASTORE:
            case JVM_ASTORE_0:
            case JVM_ASTORE_1:
            case JVM_ASTORE_2:
            case JVM_ASTORE_3:
                if (opcode == JVM_ASTORE)
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                else
                    jvm_operand_byte0 = opcode - JVM_ASTORE_0;
                emit_x_POPREF(R25);
                emit_x_POPREF(R24);
                emit_STD(R24, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0));
                emit_STD(R25, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0)+1);
            break;
            case JVM_BALOAD:
            case JVM_CALOAD:
            case JVM_SALOAD:
            case JVM_IALOAD:
            case JVM_AALOAD:
                // Arrays are indexed by a 32bit int. But we don't have enough memory to hold arrays that large, so just ignore the upper two.
                // Should check that they are 0 when implementing bounds checks.
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                // POP the array reference into Z.
                emit_x_POPREF(RZH);
                emit_x_POPREF(RZL); // Z now pointer to the base of the array object.

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
                        emit_PUSH(R25);
                        emit_PUSH(R24);
                    break;
                    case JVM_SALOAD:
                        emit_LD_ZINC(R24);
                        emit_LD_Z(R25);
                        emit_PUSH(R25);
                        emit_PUSH(R24);
                    break;
                    case JVM_IALOAD:
                        emit_LD_ZINC(R22);
                        emit_LD_ZINC(R23);
                        emit_LD_ZINC(R24);
                        emit_LD_Z(R25);
                        emit_PUSH(R25);
                        emit_PUSH(R24);
                        emit_PUSH(R23);
                        emit_PUSH(R22);
                    break;
                    case JVM_AALOAD:
                        emit_LD_ZINC(R24);
                        emit_LD_Z(R25);
                        emit_x_PUSHREF(R24);
                        emit_x_PUSHREF(R25);
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
                        emit_POP(R24);
                        emit_POP(R25);
                    break;
                    case JVM_IASTORE:
                        emit_POP(R22);
                        emit_POP(R23);
                        emit_POP(R24);
                        emit_POP(R25);
                    break;
                    case JVM_AASTORE:
                        emit_x_POPREF(R25);
                        emit_x_POPREF(R24);
                    break;
                }

                // Arrays are indexed by a 32bit int. But we don't have enough memory to hold arrays that large, so just ignore the upper two.
                // Should check that they are 0 when implementing bounds checks.
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);

                // POP the array reference into Z.
                emit_x_POPREF(RZH);
                emit_x_POPREF(RZL); // Z now pointer to the base of the array object.

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
            case JVM_IDUP2:
                // IDUP2 duplicates the top two SLOTS on the integer stack, not the top two ints. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_APOP:
                emit_x_POPREF(R25);
                emit_x_POPREF(R24);                
            break;
            case JVM_ADUP:
                emit_x_POPREF(R25);
                emit_x_POPREF(R24);
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_GETFIELD_B:
            case JVM_GETFIELD_C:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_LDD(R24, Z, jvm_operand_word);

                // need to extend the sign to push it as a short
                emit_CLR(R25);
                emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign

                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_GETFIELD_S:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_LDD(R24, Z, jvm_operand_word);
                emit_LDD(R25, Z, jvm_operand_word+1);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_GETFIELD_I:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_LDD(R22, Z, jvm_operand_word);
                emit_LDD(R23, Z, jvm_operand_word+1);
                emit_LDD(R24, Z, jvm_operand_word+2);
                emit_LDD(R25, Z, jvm_operand_word+3);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_GETFIELD_A:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_x_POPREF(R25); // POP the reference
                emit_x_POPREF(R24);

                // First find the location of reference fields
                // PUSH important stuff
                emit_PUSH(RXH);
                emit_PUSH(RXL);

                // make the call
                emit_2_CALL((uint16_t)&dj_object_getReferences);

                // POP important stuff
                emit_POP(RXL);
                emit_POP(RXH);

                // R24:R25 now points to the location of the instance references
                emit_MOVW(RZ, R24); // Move the location to Z
                emit_LDD(R24, Z, (jvm_operand_word*2)); // jvm_operand_word is an index in the (16 bit) array, so multiply by 2
                emit_LDD(R25, Z, (jvm_operand_word*2)+1);
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_PUTFIELD_B:
            case JVM_PUTFIELD_C:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_POP(R24);
                emit_POP(R25);
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_STD(R24, Z, jvm_operand_word);
            break;
            case JVM_PUTFIELD_S:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_POP(R24);
                emit_POP(R25);
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_STD(R24, Z, jvm_operand_word);
                emit_STD(R25, Z, jvm_operand_word+1);
            break;
            case JVM_PUTFIELD_I:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_STD(R22, Z, jvm_operand_word);
                emit_STD(R23, Z, jvm_operand_word+1);
                emit_STD(R24, Z, jvm_operand_word+2);
                emit_STD(R25, Z, jvm_operand_word+3);
            break;
            case JVM_PUTFIELD_A:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit_x_POPREF(R17); // POP the value to store (store in in call-saved R16-R17)
                emit_x_POPREF(R16);
                emit_x_POPREF(R25); // POP the reference to the object to store it in.
                emit_x_POPREF(R24);

                // First find the location of reference fields
                // PUSH important stuff
                emit_PUSH(RXH);
                emit_PUSH(RXL);

                // make the call
                emit_2_CALL((uint16_t)&dj_object_getReferences);

                // POP important stuff
                emit_POP(RXL);
                emit_POP(RXH);

                // R24:R25 now points to the location of the instance references
                emit_MOVW(RZ, R24); // Move the location to Z
                emit_STD(R16, Z, (jvm_operand_word*2)); // jvm_operand_word is an index in the (16 bit) array, so multiply by 2
                emit_STD(R17, Z, (jvm_operand_word*2)+1);
            break;
            case JVM_GETSTATIC_B:
            case JVM_GETSTATIC_C:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_LDD(R24, Z, offset_for_static_byte(target_infusion, jvm_operand_byte1));
                // need to extend the sign to push the byte as a short
                emit_CLR(R25);
                emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_GETSTATIC_S:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_LDD(R24, Z, offset_for_static_short(target_infusion, jvm_operand_byte1));
                emit_LDD(R25, Z, offset_for_static_short(target_infusion, jvm_operand_byte1)+1);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_GETSTATIC_I:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_LDD(R22, Z, offset_for_static_int(target_infusion, jvm_operand_byte1));
                emit_LDD(R23, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+1);
                emit_LDD(R24, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+2);
                emit_LDD(R25, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+3);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_GETSTATIC_A:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_LDD(R24, Z, offset_for_static_ref(target_infusion, jvm_operand_byte1));
                emit_LDD(R25, Z, offset_for_static_ref(target_infusion, jvm_operand_byte1)+1);
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_PUTSTATIC_B:
            case JVM_PUTSTATIC_C:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_POP(R24);
                emit_POP(R25);
                emit_STD(R24, Z, offset_for_static_byte(target_infusion, jvm_operand_byte1));
            break;
            case JVM_PUTSTATIC_S:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_POP(R24);
                emit_POP(R25);
                emit_STD(R24, Z, offset_for_static_short(target_infusion, jvm_operand_byte1));
                emit_STD(R25, Z, offset_for_static_short(target_infusion, jvm_operand_byte1)+1);
            break;
            case JVM_PUTSTATIC_I:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_STD(R22, Z, offset_for_static_int(target_infusion, jvm_operand_byte1));
                emit_STD(R23, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+1);
                emit_STD(R24, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+2);
                emit_STD(R25, Z, offset_for_static_int(target_infusion, jvm_operand_byte1)+3);
            break;
            case JVM_PUTSTATIC_A:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion.
                jvm_operand_byte1 = dj_di_getU8(code + ++pc); // Get the field.
                emit_MOVW(RZ, R2); // Z now points to the current infusion (0)

                if (jvm_operand_byte0 == 0) {
                    target_infusion = infusion;
                } else {
                    // We need to read from another infusion. Get that infusion's address first.
                    // Load the address of the referenced infusion into R24:R25
                    emit_LDD(R24, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0));
                    emit_LDD(R25, Z, offset_for_referenced_infusion(infusion, jvm_operand_byte0)+1);
                    // Then move R24:R25 to Z
                    emit_MOVW(RZ, R24);
                    // Z now points to the target infusion, but it should point to the start of the static variables
                    emit_ADIW(RZ, sizeof(dj_infusion));
                    // Find the target infusion to calculate the right offset in the next step
                    target_infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), jvm_operand_byte0);
                }

                emit_x_POPREF(R25);
                emit_x_POPREF(R24);
                emit_STD(R24, Z, offset_for_static_ref(target_infusion, jvm_operand_byte1));
                emit_STD(R25, Z, offset_for_static_ref(target_infusion, jvm_operand_byte1)+1);
            break;
            case JVM_SADD:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_ADD(R24, R22);
                emit_ADC(R25, R23);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SSUB:
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R22);
                emit_POP(R23);
                emit_SUB(R22, R24);
                emit_SBC(R23, R25);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_SMUL:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

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

                emit_MUL(R24, R22);
                emit_MOVW(R18, R0);
                emit_MUL(R24, R23);
                emit_ADD(R19, R0);
                emit_MUL(R25, R22);
                emit_ADD(R19, R0);
                emit_CLR(R1);
                emit_MOVW(R24, R18);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SDIV:
            case JVM_SREM:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_2_CALL((uint16_t)&__divmodhi4);
                if (opcode == JVM_SDIV) {
                    emit_PUSH(R23);
                    emit_PUSH(R22);
                } else { // JVM_SREM
                    emit_PUSH(R25);
                    emit_PUSH(R24);
                }
            break;
            case JVM_SNEG:
                emit_POP(R24);
                emit_POP(R25);
                emit_CLR(R22);
                emit_CLR(R23);
                emit_SUB(R22, R24);
                emit_SBC(R23, R25);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_SSHL:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(4);
                emit_LSL(R24);
                emit_ROL(R25);
                emit_DEC(R22);
                emit_BRPL(-8);

                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SSHR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(4);
                emit_ASR(R25);
                emit_ROR(R24);
                emit_DEC(R22);
                emit_BRPL(-8);

                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SUSHR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(4);
                emit_LSR(R25);
                emit_ROR(R24);
                emit_DEC(R22);
                emit_BRPL(-8);

                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SAND:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_AND(R24, R22);
                emit_AND(R25, R23);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SOR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_OR(R24, R22);
                emit_OR(R25, R23);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_SXOR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_EOR(R24, R22);
                emit_EOR(R25, R23);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_IADD:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_ADD(R22, R18);
                emit_ADC(R23, R19);
                emit_ADC(R24, R20);
                emit_ADC(R25, R21);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_ISUB:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_SUB(R18, R22);
                emit_SBC(R19, R23);
                emit_SBC(R20, R24);
                emit_SBC(R21, R25);
                emit_PUSH(R21);
                emit_PUSH(R20);
                emit_PUSH(R19);
                emit_PUSH(R18);
            break;
            case JVM_IMUL:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_2_CALL((uint16_t)&__mulsi3);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IDIV:
            case JVM_IREM:
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_2_CALL((uint16_t)&__divmodsi4);
                if (opcode == JVM_IDIV) {
                    emit_PUSH(R21);
                    emit_PUSH(R20);
                    emit_PUSH(R19);
                    emit_PUSH(R18);
                } else { // JVM_IREM
                    emit_PUSH(R25);
                    emit_PUSH(R24);
                    emit_PUSH(R23);
                    emit_PUSH(R22);
                }
            break;
            case JVM_INEG:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_CLR(R18);
                emit_CLR(R19);
                emit_MOVW(R20, R18);
                emit_SUB(R18, R22);
                emit_SBC(R19, R23);
                emit_SBC(R20, R24);
                emit_SBC(R21, R25);
                emit_PUSH(R21);
                emit_PUSH(R20);
                emit_PUSH(R19);
                emit_PUSH(R18);
            break;
            case JVM_ISHL:
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(8);
                emit_LSL(R22);
                emit_ROL(R23);
                emit_ROL(R24);
                emit_ROL(R25);
                emit_DEC(R18);
                emit_BRPL(-12);

                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_ISHR:
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(8);
                emit_ASR(R25);
                emit_ROR(R24);
                emit_ROR(R23);
                emit_ROR(R22);
                emit_DEC(R18);
                emit_BRPL(-12);

                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IUSHR: // x >>> y
                emit_POP(R20); // short y
                emit_POP(R21);
                emit_POP(R22); // int x
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                emit_RJMP(8);
                emit_LSR(R25);
                emit_ROR(R24);
                emit_ROR(R23);
                emit_ROR(R22);
                emit_DEC(R20);
                emit_BRPL(-12);

                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IAND:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_AND(R22, R18);
                emit_AND(R23, R19);
                emit_AND(R24, R20);
                emit_AND(R25, R21);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IOR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_OR(R22, R18);
                emit_OR(R23, R19);
                emit_OR(R24, R20);
                emit_OR(R25, R21);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IXOR:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_EOR(R22, R18);
                emit_EOR(R23, R19);
                emit_EOR(R24, R20);
                emit_EOR(R25, R21);
                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IINC:
            case JVM_IINC_W:
                // -129 -> JVM_IINC_W
                // -128 -> JVM_IINC
                // +127 -> JVM_IINC
                // +128 -> JVM_IINC_W
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // index of int local
                if (opcode == JVM_IINC) {
                    jvm_operand_signed_word = (int8_t)dj_di_getU8(code + ++pc);
                } else {
                    jvm_operand_signed_word = (int16_t)(((uint16_t)dj_di_getU8(code + ++pc) << 8) + dj_di_getU8(code + ++pc));
                }
                emit_LDD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0));
                emit_LDD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1);
                emit_LDD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2);
                emit_LDD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3);
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
                emit_STD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0));
                emit_STD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1);
                emit_STD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2);
                emit_STD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3);
            break;
            case JVM_S2B:
            case JVM_S2C:
                emit_POP(R24);
                emit_POP(R25);

                // need to extend the sign
                emit_CLR(R25);
                emit_SBRC(R24, 7); // highest bit of the byte value cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                emit_COM(R25); // otherwise: flip R24 to 0xFF to extend the sign

                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_S2I:
                emit_POP(R22);
                emit_POP(R23);

                // need to extend the sign
                emit_CLR(R24);
                emit_SBRC(R23, 7); // highest bit of MSB R23 cleared -> S value is positive, so R24 can stay 0 (skip next instruction)
                emit_COM(R24); // otherwise: flip R24 to 0xFF to extend the sign
                emit_MOV(R25, R24);

                emit_PUSH(R25);
                emit_PUSH(R24);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_I2S:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                emit_PUSH(R23);
                emit_PUSH(R22);
            break;
            case JVM_IIFEQ:
            case JVM_IIFNE:
            case JVM_IIFLT:
            case JVM_IIFGE:
            case JVM_IIFGT:
            case JVM_IIFLE:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IIFEQ) {
                    emit_OR(R22, R23);
                    emit_OR(R22, R24);
                    emit_OR(R22, R25);
                    emit_BRNE(SIZEOF_RJMP);
                } else if (opcode == JVM_IIFNE) {
                    emit_OR(R22, R23);
                    emit_OR(R22, R24);
                    emit_OR(R22, R25);
                    emit_BREQ(SIZEOF_RJMP);
                } else if (opcode == JVM_IIFLT) {
                    emit_SBRC(R25, 7); // value is >=0 if the highest bit is cleared
                } else if (opcode == JVM_IIFGE) {
                    emit_SBRS(R25, 7); // value is <0 if the highest bit is set
                } else if (opcode == JVM_IIFGT) {
                    emit_CP(ZERO_REG, R22);
                    emit_CPC(ZERO_REG, R23);
                    emit_CPC(ZERO_REG, R24);
                    emit_CPC(ZERO_REG, R25);
                    emit_BRGE(SIZEOF_RJMP); // if (0 >= x), then NOT (x > 0)
                } else if (opcode == JVM_IIFLE) {
                    emit_CP(ZERO_REG, R22);
                    emit_CPC(ZERO_REG, R23);
                    emit_CPC(ZERO_REG, R24);
                    emit_CPC(ZERO_REG, R25);
                    emit_BRLT(SIZEOF_RJMP); // if (0 < x), then NOT (x <= 0)
                }

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_IFNULL:
            case JVM_IFNONNULL:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                emit_x_POPREF(R25);
                emit_x_POPREF(R24);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IFNULL) {
                    emit_OR(R24, R25);
                    emit_BRNE(SIZEOF_RJMP);                    
                } else if (opcode == JVM_IFNONNULL) {
                    emit_OR(R24, R25);
                    emit_BREQ(SIZEOF_RJMP);                    
                }
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_IF_SCMPEQ:
            case JVM_IF_SCMPNE:
            case JVM_IF_SCMPLT:
            case JVM_IF_SCMPGE:
            case JVM_IF_SCMPGT:
            case JVM_IF_SCMPLE:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IF_SCMPEQ) {
                    emit_CP(R24, R22);
                    emit_CPC(R25, R23);
                    emit_BRNE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_SCMPNE) {
                    emit_CP(R24, R22);
                    emit_CPC(R25, R23);
                    emit_BREQ(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_SCMPLT) {
                    emit_CP(R24, R22);
                    emit_CPC(R25, R23);
                    emit_BRGE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_SCMPGE) {
                    emit_CP(R24, R22);
                    emit_CPC(R25, R23);
                    emit_BRLT(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_SCMPGT) {
                    emit_CP(R22, R24);
                    emit_CPC(R23, R25);
                    emit_BRGE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_SCMPLE) {
                    emit_CP(R22, R24);
                    emit_CPC(R23, R25);
                    emit_BRLT(SIZEOF_RJMP);
                }
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_IF_ICMPEQ:
            case JVM_IF_ICMPNE:
            case JVM_IF_ICMPLT:
            case JVM_IF_ICMPGE:
            case JVM_IF_ICMPGT:
            case JVM_IF_ICMPLE:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;
                emit_POP(R18);
                emit_POP(R19);
                emit_POP(R20);
                emit_POP(R21);
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IF_ICMPEQ) {
                    emit_CP(R22, R18);
                    emit_CPC(R23, R19);
                    emit_CPC(R24, R20);
                    emit_CPC(R25, R21);
                    emit_BRNE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ICMPNE) {
                    emit_CP(R22, R18);
                    emit_CPC(R23, R19);
                    emit_CPC(R24, R20);
                    emit_CPC(R25, R21);
                    emit_BREQ(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ICMPLT) {
                    emit_CP(R22, R18);
                    emit_CPC(R23, R19);
                    emit_CPC(R24, R20);
                    emit_CPC(R25, R21);
                    emit_BRGE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ICMPGE) {
                    emit_CP(R22, R18);
                    emit_CPC(R23, R19);
                    emit_CPC(R24, R20);
                    emit_CPC(R25, R21);
                    emit_BRLT(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ICMPGT) {
                    emit_CP(R18, R22);
                    emit_CPC(R19, R23);
                    emit_CPC(R20, R24);
                    emit_CPC(R21, R25);
                    emit_BRGE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ICMPLE) {
                    emit_CP(R18, R22);
                    emit_CPC(R19, R23);
                    emit_CPC(R20, R24);
                    emit_CPC(R21, R25);
                    emit_BRLT(SIZEOF_RJMP);
                }
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_IF_ACMPEQ:
            case JVM_IF_ACMPNE:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                emit_x_POPREF(R25);
                emit_x_POPREF(R24);
                emit_x_POPREF(R23);
                emit_x_POPREF(R22);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IF_ACMPEQ) {
                    emit_CP(R22, R24);
                    emit_CPC(R23, R25);
                    emit_BRNE(SIZEOF_RJMP);
                } else if (opcode == JVM_IF_ACMPNE) {
                    emit_CP(R22, R24);
                    emit_CPC(R23, R25);
                    emit_BREQ(SIZEOF_RJMP);
                }
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_GOTO:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_TABLESWITCH: {
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                // Pop the key value
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);
                // Load the upper bound
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                jvm_operand_byte3 = dj_di_getU8(code + ++pc);
                int32_t upperbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
                emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
                emit_LDI(R20, jvm_operand_byte1);
                emit_LDI(R19, jvm_operand_byte2);
                emit_LDI(R18, jvm_operand_byte3);
                emit_CP(R18, R22);
                emit_CPC(R19, R23);
                emit_CPC(R20, R24);
                emit_CPC(R21, R25);
                emit_BRGE(SIZEOF_RJMP);
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset

                // Lower than or equal to the upper bound: load the lower bound
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                jvm_operand_byte3 = dj_di_getU8(code + ++pc);
                int32_t lowerbound = (int32_t)(((uint32_t)jvm_operand_byte0 << 24) | ((uint32_t)jvm_operand_byte1 << 16) | ((uint32_t)jvm_operand_byte2 << 8) | ((uint32_t)jvm_operand_byte3 << 0));
                emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
                emit_LDI(R20, jvm_operand_byte1);
                emit_LDI(R19, jvm_operand_byte2);
                emit_LDI(R18, jvm_operand_byte3);
                emit_CP(R22, R18);
                emit_CPC(R23, R19);
                emit_CPC(R24, R20);
                emit_CPC(R25, R21);
                emit_BRGE(SIZEOF_RJMP);
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset

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

                emit_POP(RZH); // POP PC into Z (ignore the highest (>128K) byte for now)
                emit_POP(RZH);
                emit_POP(RZL);
                emit_ADIW(RZ, 7); // Will need to compensate here for the instructions inbetween RCALL(0) and the table. Now Z will point to the start of the RJMP table.
                emit_ADD(RZL, R22); // Add the index to get the target address in the RJMP table
                emit_ADC(RZH, R23);
                emit_IJMP(); // All this fuss because there's no relative indirect jump...

                // Now emit the RJMP table itself
                for (int i=0; i<(upperbound-lowerbound+1); i++) { // +1 since both bounds are inclusive
                    jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                    pc += 4;
                    rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                    emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
                }
            }
            break;
            case JVM_LOOKUPSWITCH: {
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                uint16_t default_branch_target = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                // Pop the key value
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                uint16_t number_of_cases = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                for (int i=0; i<number_of_cases; i++) {
                    // Get the case label
                    jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                    jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                    jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                    jvm_operand_byte3 = dj_di_getU8(code + ++pc);
                    // Get the branch target (and skip the branch address used by the interpreter)
                    jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                    pc += 4;
                    emit_LDI(R21, jvm_operand_byte0); // Bytecode is big endian
                    emit_LDI(R20, jvm_operand_byte1);
                    emit_LDI(R19, jvm_operand_byte2);
                    emit_LDI(R18, jvm_operand_byte3);
                    emit_CP(R18, R22);
                    emit_CPC(R19, R23);
                    emit_CPC(R20, R24);
                    emit_CPC(R21, R25);
                    emit_BRNE(SIZEOF_RJMP);  

                  
                    rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                    emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
                }

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(default_branch_target) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            }
            break;
            case JVM_SRETURN:
                emit_POP(R24);
                emit_POP(R25);

                // epilogue (is this the right way?)
                emit_POP(R28); // Pop Y
                emit_POP(R29);
                emit_POP(R2);
                emit_POP(R3);
                emit_RET();
            break;
            case JVM_IRETURN:
                emit_POP(R22);
                emit_POP(R23);
                emit_POP(R24);
                emit_POP(R25);

                // epilogue (is this the right way?)
                emit_POP(R28); // Pop Y
                emit_POP(R29);
                emit_POP(R2);
                emit_POP(R3);
                emit_RET();
            break;
            case JVM_ARETURN:
                emit_x_POPREF(R25); // POP the reference into Z
                emit_x_POPREF(R24);

                // epilogue (is this the right way?)
                emit_POP(R28); // Pop Y
                emit_POP(R29);
                emit_POP(R2);
                emit_POP(R3);
                emit_RET();
            break;
            case JVM_RETURN:
                // epilogue (is this the right way?)
                emit_POP(R28); // Pop Y
                emit_POP(R29);
                emit_POP(R2);
                emit_POP(R3);
                emit_RET();
            break;
            case JVM_INVOKEVIRTUAL:
            case JVM_INVOKESPECIAL:
            case JVM_INVOKESTATIC:
            case JVM_INVOKEINTERFACE:
                // set intStack to SP
                emit_PUSH(ZERO_REG); // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT. 
                                            // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                            // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.
                emit_2_LDS(R24, SPaddress_L); // Load SP into R24:R25
                emit_2_LDS(R25, SPaddress_H); // Load SP into R24:R25
                emit_2_STS((uint16_t)&(intStack), R24); // Store SP into intStack
                emit_2_STS((uint16_t)&(intStack)+1, R25); // Store SP into intStack


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

                
                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id
                if        (opcode == JVM_INVOKEVIRTUAL
                        || opcode == JVM_INVOKEINTERFACE) {
                    jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                    emit_LDI(R22, jvm_operand_byte2); // nr_ref_args
                    emit_2_CALL((uint16_t)&RTC_INVOKEVIRTUAL_OR_INTERFACE);
                } else if (opcode == JVM_INVOKESPECIAL) {
                    emit_2_CALL((uint16_t)&RTC_INVOKESPECIAL);
                } else if (opcode == JVM_INVOKESTATIC) {
                    emit_2_CALL((uint16_t)&RTC_INVOKESTATIC);
                }


                // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
                emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
                // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
                emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


                // get SP from intStack
                emit_2_LDS(R24, (uint16_t)&(intStack)); // Load intStack into R24:R25
                emit_2_LDS(R25, (uint16_t)&(intStack)+1); // Load intStack into R24:R25
                emit_2_STS(SPaddress_L, R24); // Store R24:25 into SP
                emit_2_STS(SPaddress_H, R25); // Store R24:25 into SP
                emit_POP(R25); // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE FIRST LINE FOR INVOKESTATIC. SEE COMMENT ABOVE.
            break;
            case JVM_NEW:
                // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
                emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id
                emit_2_CALL((uint16_t)&RTC_NEW);


                // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
                emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
                // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
                emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


                // push the reference to the new object onto the ref stack
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_NEWARRAY:
                // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
                emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                emit_POP(R22); // size
                emit_POP(R23);
                emit_LDI(R24, jvm_operand_byte0); // (int) element type
                emit_2_CALL((uint16_t)&dj_int_array_create);


                // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
                emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
                // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
                emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


                // push the reference to the new object onto the ref stack
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_ANEWARRAY:
                // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                emit_2_STS((uint16_t)&(refStack), RXL); // Store X into refStack
                emit_2_STS((uint16_t)&(refStack)+1, RXH); // Store X into refStack


                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_POP(R22); // size
                emit_POP(R23);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id
                emit_2_CALL((uint16_t)&RTC_ANEWARRAY);


                // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                emit_2_LDS(RYL, (uint16_t)&(localReferenceVariables)); // Load localReferenceVariables into Y
                emit_2_LDS(RYH, (uint16_t)&(localReferenceVariables)+1); // Load localReferenceVariables into Y
                // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                emit_2_LDS(RXL, (uint16_t)&(refStack)); // Load refStack into X
                emit_2_LDS(RXH, (uint16_t)&(refStack)+1); // Load refStack into X


                // push the reference to the new object onto the ref stack
                emit_x_PUSHREF(R24);
                emit_x_PUSHREF(R25);
            break;
            case JVM_ARRAYLENGTH: // The length of an array is stored as 16 bit at the start of the array
                emit_x_POPREF(R31); // POP the reference into Z
                emit_x_POPREF(R30);
                emit_LD_ZINC(R24);
                emit_LD_Z(R25);
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            case JVM_CHECKCAST:
                // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY

                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_x_POPREF(R23); // reference to the object
                emit_x_POPREF(R22);
                emit_x_PUSHREF(R22); // TODO: optimise this. CHECKCAST should only peek.
                emit_x_PUSHREF(R23);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id

                // PUSH important stuff
                emit_PUSH(RXH);
                emit_PUSH(RXL);

                // make the call
                emit_2_CALL((uint16_t)&RTC_CHECKCAST);

                // POP important stuff
                emit_POP(RXL);
                emit_POP(RXH);
            break;
            case JVM_INSTANCEOF:
                // THIS WILL BREAK IF GC RUNS, BUT IT COULD ONLY RUN IF AN EXCEPTION IS THROWN, WHICH MEANS WE CRASH ANYWAY

                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit_x_POPREF(R23); // reference to the object
                emit_x_POPREF(R22);
                emit_LDI(R24, jvm_operand_byte0); // infusion id
                emit_LDI(R25, jvm_operand_byte1); // entity id

                // PUSH important stuff
                emit_PUSH(RXH);
                emit_PUSH(RXL);

                // make the call
                emit_2_CALL((uint16_t)&RTC_INSTANCEOF);

                // POP important stuff
                emit_POP(RXL);
                emit_POP(RXH);

                // push the reference to the new object onto the ref stack
                emit_PUSH(R25);
                emit_PUSH(R24);
            break;
            // BRANCHES
            case JVM_SIFEQ:
            case JVM_SIFNE:
            case JVM_SIFLT:
            case JVM_SIFGE:
            case JVM_SIFGT:
            case JVM_SIFLE:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                emit_POP(R24);
                emit_POP(R25);
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_SIFEQ) {
                    emit_OR(R24, R25);
                    emit_BRNE(SIZEOF_RJMP);
                } else if (opcode == JVM_SIFNE) {
                    emit_OR(R24, R25);
                    emit_BREQ(SIZEOF_RJMP);
                } else if (opcode == JVM_SIFLT) {
                    emit_SBRC(R25, 7); // value is >=0 if the highest bit is cleared
                } else if (opcode == JVM_SIFGE) {
                    emit_SBRS(R25, 7); // value is <0 if the highest bit is set
                } else if (opcode == JVM_SIFGT) {
                    emit_CP(ZERO_REG, R24);
                    emit_CPC(ZERO_REG, R25);
                    emit_BRGE(SIZEOF_RJMP); // if (0 >= x), then NOT (x > 0)
                } else if (opcode == JVM_SIFLE) {
                    emit_CP(ZERO_REG, R24);
                    emit_CPC(ZERO_REG, R25);
                    emit_BRLT(SIZEOF_RJMP); // if (0 < x), then NOT (x <= 0)
                }

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_BRTARGET:
                // This is a noop, but we need to record the address of the next instruction
                // in the branch table as a RJMP instruction.
                rtc_flush(); // Finish writing, and also make sure we won't optimise across basic block boundaries.
                tmp_current_position = wkreprog_get_raw_position();
                wkreprog_close();
                wkreprog_open_raw(rtc_branch_target_table_address(branch_target_count));
                emit_RJMP(tmp_current_position - rtc_branch_target_table_address(branch_target_count) - 2); // Relative jump to tmp_current_position from the branch target table. -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
                rtc_flush();
                wkreprog_close();
                wkreprog_open_raw(tmp_current_position);
                branch_target_count++;
            break;

            // Not implemented
            default:
                DEBUG_LOG(DBG_RTC, "Unimplemented Java opcode %d at pc=%d\n", opcode, pc);
                dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
            break;
        }
    }
    rtc_flush();
}

void rtc_compile_lib(dj_infusion *infusion) {
    // uses 512bytes on the stack... maybe optimise this later
    native_method_function_t rtc_method_start_addresses[256];
    for (uint16_t i=0; i<256; i++)
        rtc_method_start_addresses[i] = 0;

    wkreprog_open_raw((dj_di_pointer)rtc_compiled_code_buffer);

    uint16_t number_of_methodimpls = dj_di_parentElement_getListSize(infusion->methodImplementationList);
    DEBUG_LOG(DBG_RTC, "[rtc] infusion contains %d methods\n", number_of_methodimpls);

    const DJ_PROGMEM native_method_function_t *handlers = infusion->native_handlers;
    DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
    for (uint16_t i=0; i<number_of_methodimpls; i++) {      
        DEBUG_LOG(DBG_RTC, "[rtc] (compile) pointer for method %i %p\n", i, infusion->native_handlers[i]);  

        dj_di_pointer methodimpl = dj_infusion_getMethodImplementation(infusion, i);
        if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_NATIVE) {
            DEBUG_LOG(DBG_RTC, "[rtc] skipping native method %d\n", i);
            continue;
        }

        if (handlers[i] != NULL) {
            DEBUG_LOG(DBG_RTC, "[rtc] should skip already compiled method %d with pointer %p, but won't for now\n", i, handlers[i]);
            // continue; // Skip native or already rtc compiled methods
        }

        // TMPRTC
        if (i==0) {
            DEBUG_LOG(DBG_RTC, "[rtc] skipping method 0 for now\n", i);
            continue;
        }
        
        DEBUG_LOG(DBG_RTC, "[rtc] compiling method %d\n", i);

        // store the starting address for this method;
        // IMPORTANT!!!! the PC in AVR stores WORD addresses, so we need to divide the address
        // of a function by 2 in order to get a function pointer!
        dj_di_pointer method_address = wkreprog_get_raw_position() + rtc_branch_table_size(methodimpl);
        rtc_method_start_addresses[i] = (native_method_function_t)(method_address/2);

#ifdef AVRORA
    avroraRTCTraceStartMethod(i, wkreprog_get_raw_position());
#endif

        rtc_compile_method(methodimpl, infusion);

#ifdef AVRORA
    rtc_flush(); // Don't really need to do this unless we want to print the contents of Flash memory at this point.
    dj_di_pointer tmp_address = wkreprog_get_raw_position();
    wkreprog_close();
    wkreprog_open_raw(tmp_address);
    avroraRTCTraceEndMethod(wkreprog_get_raw_position());
#endif
    }


    // At this point, the addresses in the rtc_method_start_addresses are 0
    // for the native methods, while the handler table is 0 for the java methods.
    // We need to fill in the addresses in rtc_method_start_addresses in the
    // empty slots in the handler table.
    rtc_update_method_pointers(infusion, rtc_method_start_addresses);

    // Mark the infusion as translated (how?)

}


