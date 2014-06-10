#include "types.h"
#include "panic.h"
#include "debug.h"
#include "execution.h"
#include "parse_infusion.h"
#include "infusion.h"
#include "wkreprog.h"
#include "asm.h"
#include "opcodes.h"
#include "rtc.h"
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>

// PUSHREF
#define asm_x_PUSHREF(reg)              asm_ST_XINC(reg)
#define asm_x_POPREF(reg)               asm_LD_DECX(reg)

// Offsets for static variables in an infusion, relative to the start of infusion->staticReferencesFields
#define offset_for_static_ref(infusion_ptr, variable_index)   ((uint16_t)((void*)(&((infusion)->staticReferenceFields[variable_index])) - (void *)((infusion)->staticReferenceFields)))
#define offset_for_static_byte(infusion_ptr, variable_index)  ((uint16_t)((void*)(&((infusion)->staticByteFields[variable_index]))      - (void *)((infusion)->staticReferenceFields)))
#define offset_for_static_short(infusion_ptr, variable_index) ((uint16_t)((void*)(&((infusion)->staticShortFields[variable_index]))     - (void *)((infusion)->staticReferenceFields)))
#define offset_for_static_int(infusion_ptr, variable_index)   ((uint16_t)((void*)(&((infusion)->staticIntFields[variable_index]))       - (void *)((infusion)->staticReferenceFields)))
#define offset_for_static_long(infusion_ptr, variable_index)  ((uint16_t)((void*)(&((infusion)->staticLonFields[variable_index]))       - (void *)((infusion)->staticReferenceFields)))

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

// rtc_instruction.c functions
extern void RTC_INVOKESTATIC(void);

// the stack pointers used by execution.c
extern int16_t *intStack;
extern ref_t *refStack;


// USED AT COMPILE TIME:
const unsigned char PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) rtc_compiled_code_buffer[RTC_COMPILED_CODE_BUFFER_SIZE] = {};
// Buffer for emitting code.
#define RTC_MAX_SIZE_FOR_SINGLE_JVM_INSTRUCTION 32 // Used to check when we need to flush the buffer (when rtc_codebuffer_position-rtc_codebuffer < RTC_MAX_SIZE_FOR_SINGLE_JVM_INSTRUCTION)
#define RTC_CODEBUFFER_SIZE 64
uint16_t *rtc_codebuffer;
uint16_t *rtc_codebuffer_position; // A pointer to somewhere within the buffer


void rtc_flush() {
    uint8_t *instructiondata = (uint8_t *)rtc_codebuffer;
    uint16_t count = rtc_codebuffer_position - rtc_codebuffer;
#ifdef DARJEELING_DEBUG
    for (int i=0; i<count; i++) {
        DEBUG_LOG(DBG_RTC, "[rtc]    %x  (%x %x)\n", rtc_codebuffer[i], instructiondata[i*2], instructiondata[i*2+1]);
    }
#endif // DARJEELING_DEBUG
    // Write to flash
    wkreprog_write(2*count, instructiondata);
    // Buffer is now empty
    rtc_codebuffer_position = rtc_codebuffer;
}

static inline void emit(uint16_t wordopcode) {
    *(rtc_codebuffer_position++) = wordopcode;
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
    // Skip this number of bytes (actually it doesn't matter what we write here, but I just use the same data so nothing changes)
    wkreprog_write(branchTableSize, (uint8_t *)branch_target_table_start_ptr);

    // prologue (is this the right way?)
    emit( asm_PUSH(R3) );
    emit( asm_PUSH(R2) );
    emit( asm_PUSH(R29) ); // Push Y
    emit( asm_PUSH(R28) );
    emit( asm_MOVW(R28, R24) ); // Pointer to locals in Y
    emit( asm_MOVW(R26, R22) ); // Pointer to ref stack in X
    emit( asm_MOVW(R2, R20) ); // Pointer to static in R2 (will be MOVWed to R30 when necessary)

    // translate the method
    dj_di_pointer code = dj_di_methodImplementation_getData(methodimpl);
    uint16_t method_length = dj_di_methodImplementation_getLength(methodimpl);
    DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", method_length);

    for (uint16_t pc=0; pc<method_length; pc++) {
        if (RTC_CODEBUFFER_SIZE-(rtc_codebuffer_position-rtc_codebuffer) < RTC_MAX_SIZE_FOR_SINGLE_JVM_INSTRUCTION) {
            // There may not be enough space in the buffer to hold the current opcode.
            rtc_flush();
        }

        uint8_t opcode = dj_di_getU8(code + pc);
        DEBUG_LOG(DBG_RTC, "[rtc] JVM opcode %d (pc=%d, method length=%d)\n", opcode, pc, method_length);
        switch (opcode) {
            case JVM_SCONST_M1:
                emit( asm_LDI(R25, 0xFF) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
            break;
            case JVM_SCONST_0:
            case JVM_SCONST_1:
            case JVM_SCONST_2:
            case JVM_SCONST_3:
            case JVM_SCONST_4:
            case JVM_SCONST_5:
                jvm_operand_byte0 = opcode - JVM_SCONST_0;
                emit( asm_LDI(R24, jvm_operand_byte0) );
                emit( asm_LDI(R25, 0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_ICONST_M1:
                emit( asm_LDI(R25, 0xFF) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
            break;
            case JVM_ICONST_0:
            case JVM_ICONST_1:
            case JVM_ICONST_2:
            case JVM_ICONST_3:
            case JVM_ICONST_4:
            case JVM_ICONST_5:
                jvm_operand_byte0 = opcode - JVM_ICONST_0;
                emit( asm_LDI(R24, jvm_operand_byte0) );
                emit( asm_LDI(R25, 0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_BSPUSH:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                emit( asm_LDI(R24, jvm_operand_byte0) );
                emit( asm_LDI(R25, 0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_BIPUSH:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                emit( asm_LDI(R22, jvm_operand_byte0) );
                emit( asm_LDI(R23, 0) );
                emit( asm_LDI(R24, 0) );
                emit( asm_LDI(R25, 0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_SSPUSH:
                // bytecode is big endian
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit( asm_LDI(R24, jvm_operand_byte1) );
                emit( asm_LDI(R25, jvm_operand_byte0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_IIPUSH:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                jvm_operand_byte2 = dj_di_getU8(code + ++pc);
                jvm_operand_byte3 = dj_di_getU8(code + ++pc);
                emit( asm_LDI(R22, jvm_operand_byte3) );
                emit( asm_LDI(R23, jvm_operand_byte2) );
                emit( asm_LDI(R24, jvm_operand_byte1) );
                emit( asm_LDI(R25, jvm_operand_byte0) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
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
                emit( asm_LDD(R24, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)) );
                emit( asm_LDD(R25, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)+1) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
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
                emit( asm_LDD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)) );
                emit( asm_LDD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1) );
                emit( asm_LDD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2) );
                emit( asm_LDD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
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
                emit( asm_LDD(R24, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0)) );
                emit( asm_LDD(R25, Y, offset_for_reflocal(methodimpl, jvm_operand_byte0)+1) );
                emit( asm_x_PUSHREF(R24) );
                emit( asm_x_PUSHREF(R25) );
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
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_STD(R24, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)) );
                emit( asm_STD(R25, Y, offset_for_intlocal_short(methodimpl, jvm_operand_byte0)+1) );
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
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_STD(R22, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)) );
                emit( asm_STD(R23, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+1) );
                emit( asm_STD(R24, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+2) );
                emit( asm_STD(R25, Y, offset_for_intlocal_int(methodimpl, jvm_operand_byte0)+3) );
            break;
            case JVM_SALOAD:
                // Arrays are indexed by a 32bit int. But we don't have enough memory to hold arrays that large, so just ignore the upper two.
                // Should check that they are 0 when implementing bounds checks.
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );

                // POP the array reference into Z.
                emit( asm_x_POPREF(RZH) );
                emit( asm_x_POPREF(RZL) ); // Z now pointer to the base of the array object.

                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit( asm_LSL(R22) );
                emit( asm_ROL(R23) );

                // Add 2*the index to Z
                emit( asm_ADD(RZL, R22) );
                emit( asm_ADC(RZH, R23) );

                // Add 3 to skip 2 bytes for array length and 1 byte for array type.
                emit( asm_ADIW(RZ, 3) ); 

                // Now Z points to the target element
                emit( asm_LD_ZINC(R24) );
                emit( asm_LD_Z(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_SASTORE:
                // Pop the value we need to store in the array.
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );

                // Arrays are indexed by a 32bit int. But we don't have enough memory to hold arrays that large, so just ignore the upper two.
                // We'll pop the index to R22:R23 here, but to R22:R25 in SALOAD. This is just to make it easier for the SALOAD sequence to be
                // optimised against the previous instruction.
                // Should check that they are 0 when implementing bounds checks.
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R18) );
                emit( asm_POP(R18) );

                // POP the array reference into Z.
                emit( asm_x_POPREF(RZH) );
                emit( asm_x_POPREF(RZL) ); // Z now pointer to the base of the array object.

                // Multiply the index by 2, since we're indexing 16 bit shorts.
                emit( asm_LSL(R22) );
                emit( asm_ROL(R23) );

                // Add 2*the index to Z
                emit( asm_ADD(RZL, R22) );
                emit( asm_ADC(RZH, R23) );

                // Add 3 to skip 2 bytes for array length and 1 byte for array type.
                emit( asm_ADIW(RZ, 3) ); 

                // Now Z points to the target element
                emit( asm_ST_ZINC(R24) );
                emit( asm_ST_Z(R25) );
            break;
            case JVM_IDUP2:
                // IDUP2 duplicates the top two SLOTS on the integer stack, not the top two ints. So IDUP2 is actually IDUP, and IDUP is actually SDUP.
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_ADUP:
                emit( asm_x_POPREF(R25) );
                emit( asm_x_POPREF(R24) );
                emit( asm_x_PUSHREF(R24) );
                emit( asm_x_PUSHREF(R25) );
                emit( asm_x_PUSHREF(R24) );
                emit( asm_x_PUSHREF(R25) );
            break;
            case JVM_GETFIELD_S:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit( asm_x_POPREF(R31) ); // POP the reference into Z
                emit( asm_x_POPREF(R30) );
                emit( asm_LDD(R24, Z, jvm_operand_word) );
                emit( asm_LDD(R25, Z, jvm_operand_word+1) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_PUTFIELD_S:
                jvm_operand_word = (dj_di_getU8(code + pc + 1) << 8) | dj_di_getU8(code + pc + 2);
                pc += 2;
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_x_POPREF(R31) ); // POP the reference into Z
                emit( asm_x_POPREF(R30) );
                emit( asm_STD(R24, Z, jvm_operand_word) );
                emit( asm_STD(R25, Z, jvm_operand_word+1) );
            break;
            case JVM_GETSTATIC_S:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion. Should be 0.
                if (jvm_operand_byte0 != 0) {
                    DEBUG_LOG(DBG_RTC, "JVM_GETSTATIC_S only supported within current infusion. infusion=%d pc=%d\n", jvm_operand_byte0, pc);
                    dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                }
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the field.
                emit( asm_MOVW(R30, R2) );
                emit( asm_LDD(R24, Z, offset_for_static_short(infusion, jvm_operand_byte0)) );
                emit( asm_LDD(R25, Z, offset_for_static_short(infusion, jvm_operand_byte0)+1) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_PUTSTATIC_S:
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the infusion. Should be 0.
                if (jvm_operand_byte0 != 0) {
                    DEBUG_LOG(DBG_RTC, "JVM_GETSTATIC_S only supported within current infusion. infusion=%d pc=%d\n", jvm_operand_byte0, pc);
                    dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                }
                jvm_operand_byte0 = dj_di_getU8(code + ++pc); // Get the field.
                emit( asm_MOVW(R30, R2) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_STD(R24, Z, offset_for_static_short(infusion, jvm_operand_byte0)) );
                emit( asm_STD(R25, Z, offset_for_static_short(infusion, jvm_operand_byte0)+1) );
            break;
            case JVM_SADD:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_ADD(R24, R22) );
                emit( asm_ADC(R25, R23) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_SSUB:
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_SUB(R24, R22) );
                emit( asm_SBC(R25, R23) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R25) );
            break;
            case JVM_SMUL:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );

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

                emit( asm_MUL(R24, R22) );
                emit( asm_MOVW(R18, R0) );
                emit( asm_MUL(R24, R23) );
                emit( asm_ADD(R19, R0) );
                emit( asm_MUL(R25, R22) );
                emit( asm_ADD(R19, R0) );
                // gcc generates "clr r1" here, but it doesn't seem necessary?
                emit( asm_MOVW(R24, R18) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_SDIV:
            case JVM_SREM:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_CALL1((uint16_t)&__divmodhi4) );
                emit( asm_CALL2((uint16_t)&__divmodhi4) );
                if (opcode == JVM_SDIV) {
                    emit( asm_PUSH(R23) );
                    emit( asm_PUSH(R22) );
                } else { // JVM_SREM
                    emit( asm_PUSH(R25) );
                    emit( asm_PUSH(R24) );
                }
            break;
            case JVM_SNEG:
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_CLR(R22) );
                emit( asm_CLR(R23) );
                emit( asm_SUB(R22, R24) );
                emit( asm_SBC(R23, R25) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_SAND:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_AND(R24, R22) );
                emit( asm_AND(R25, R23) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_SOR:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_OR(R24, R22) );
                emit( asm_OR(R25, R23) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_SXOR:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_EOR(R24, R22) );
                emit( asm_EOR(R25, R23) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
            break;
            case JVM_IADD:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_ADD(R22, R18) );
                emit( asm_ADC(R23, R19) );
                emit( asm_ADC(R24, R20) );
                emit( asm_ADC(R25, R21) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_ISUB:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_SUB(R18, R22) );
                emit( asm_SBC(R19, R23) );
                emit( asm_SBC(R20, R24) );
                emit( asm_SBC(R21, R25) );
                emit( asm_PUSH(R21) );
                emit( asm_PUSH(R20) );
                emit( asm_PUSH(R19) );
                emit( asm_PUSH(R18) );
            break;
            case JVM_IMUL:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_CALL1((uint16_t)&__mulsi3) );
                emit( asm_CALL2((uint16_t)&__mulsi3) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_IDIV:
            case JVM_IREM:
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_CALL1((uint16_t)&__divmodsi4) );
                emit( asm_CALL2((uint16_t)&__divmodsi4) );
                if (opcode == JVM_IDIV) {
                    emit( asm_PUSH(R21) );
                    emit( asm_PUSH(R20) );
                    emit( asm_PUSH(R19) );
                    emit( asm_PUSH(R18) );
                } else { // JVM_IREM
                    emit( asm_PUSH(R25) );
                    emit( asm_PUSH(R24) );
                    emit( asm_PUSH(R23) );
                    emit( asm_PUSH(R22) );
                }
            break;
            case JVM_INEG:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_CLR(R18) );
                emit( asm_CLR(R19) );
                emit( asm_MOVW(R20, R18) );
                emit( asm_SUB(R18, R22) );
                emit( asm_SBC(R19, R23) );
                emit( asm_SBC(R20, R24) );
                emit( asm_SBC(R21, R25) );
                emit( asm_PUSH(R21) );
                emit( asm_PUSH(R20) );
                emit( asm_PUSH(R19) );
                emit( asm_PUSH(R18) );
            break;
            case JVM_IAND:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_AND(R22, R18) );
                emit( asm_AND(R23, R19) );
                emit( asm_AND(R24, R20) );
                emit( asm_AND(R25, R21) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_IOR:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_OR(R22, R18) );
                emit( asm_OR(R23, R19) );
                emit( asm_OR(R24, R20) );
                emit( asm_OR(R25, R21) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_IXOR:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_POP(R18) );
                emit( asm_POP(R19) );
                emit( asm_POP(R20) );
                emit( asm_POP(R21) );
                emit( asm_EOR(R22, R18) );
                emit( asm_EOR(R23, R19) );
                emit( asm_EOR(R24, R20) );
                emit( asm_EOR(R25, R21) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
                emit( asm_PUSH(R23) );
                emit( asm_PUSH(R22) );
            break;
            case JVM_S2I:
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                emit( asm_PUSH(ZERO_REG) );
                emit( asm_PUSH(ZERO_REG) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
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

                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_IF_SCMPEQ) {
                    emit( asm_CP(R22, R24) );
                    emit( asm_CPC(R23, R25) );
                    emit( asm_BRNE(SIZEOF_RJMP) );
                } else if (opcode == JVM_IF_SCMPNE) {
                    emit( asm_CP(R22, R24) );
                    emit( asm_CPC(R23, R25) );
                    emit( asm_BREQ(SIZEOF_RJMP) );
                } else if (opcode == JVM_IF_SCMPLT) {
                    emit( asm_CP(R22, R24) );
                    emit( asm_CPC(R23, R25) );
                    emit( asm_BRGE(SIZEOF_RJMP) );
                } else if (opcode == JVM_IF_SCMPGE) {
                    emit( asm_CP(R22, R24) );
                    emit( asm_CPC(R23, R25) );
                    emit( asm_BRLT(SIZEOF_RJMP) );
                } else if (opcode == JVM_IF_SCMPGT) {
                    emit( asm_CP(R24, R22) );
                    emit( asm_CPC(R25, R23) );
                    emit( asm_BRGE(SIZEOF_RJMP) );
                } else if (opcode == JVM_IF_SCMPLE) {
                    emit( asm_CP(R24, R22) );
                    emit( asm_CPC(R25, R23) );
                    emit( asm_BRLT(SIZEOF_RJMP) );
                }
                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit( asm_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2) ); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_GOTO:
                // Branch instructions first have a bytecode offset, used by the interpreter,
                // followed by a branch target index used when compiling to native code.
                jvm_operand_word = (dj_di_getU8(code + pc + 3) << 8) | dj_di_getU8(code + pc + 4);
                pc += 4;

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit( asm_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2) ); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_SRETURN:
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );

                // epilogue (is this the right way?)
                emit( asm_POP(R28) ); // Pop Y
                emit( asm_POP(R29) );
                emit( asm_POP(R2) );
                emit( asm_POP(R3) );
                emit( asm_RET );
            break;
            case JVM_IRETURN:
                emit( asm_POP(R22) );
                emit( asm_POP(R23) );
                emit( asm_POP(R24) );
                emit( asm_POP(R25) );

                // epilogue (is this the right way?)
                emit( asm_POP(R28) ); // Pop Y
                emit( asm_POP(R29) );
                emit( asm_POP(R2) );
                emit( asm_POP(R3) );
                emit( asm_RET );
            break;
            case JVM_RETURN:
                // epilogue (is this the right way?)
                emit( asm_POP(R28) ); // Pop Y
                emit( asm_POP(R29) );
                emit( asm_POP(R2) );
                emit( asm_POP(R3) );
                emit( asm_RET );
            break;
            case JVM_INVOKESTATIC:
                // set intStack and refStack pointers to SP and X resp.
                emit( asm_PUSH(ZERO_REG) ); // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT. 
                                            // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                            // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.

                emit( asm_LDS1(R24, SPaddress_L) ); // Load SP into R24:R25
                emit( asm_LDS2(R24, SPaddress_L) ); // Load SP into R24:R25
                emit( asm_LDS1(R25, SPaddress_H) ); // Load SP into R24:R25
                emit( asm_LDS2(R25, SPaddress_H) ); // Load SP into R24:R25
                emit( asm_LDI(RZL, ((uint16_t)&(intStack)) & 0xFF) ); // Load the address of intStack into Z
                emit( asm_LDI(RZH, (((uint16_t)&(intStack)) >> 8) & 0xFF) );
                emit( asm_ST_ZINC(R24) ); // Store SP into intStack
                emit( asm_ST_Z(R25) );

                emit( asm_LDI(RZL, ((uint16_t)&(refStack)) & 0xFF) ); // Load the address of refStack into Z
                emit( asm_LDI(RZH, (((uint16_t)&(refStack)) >> 8) & 0xFF) );
                emit( asm_ST_ZINC(RXL) ); // Store X into refStack
                emit( asm_ST_Z(RXH) );


                // Reserve 8 bytes of space on the stack, in case the returned int is large than passed ints
                // TODO: make this more efficient by looking up the method, and seeing if the return type is int,
                //       and if so, if the size of the return type is larger than the integers passed. Then only
                //       reserve the space that's needed.
                //       This is for the worst case, where no ints are passed, so there's no space reserved, and
                //       a 64 bit long is returned.
                emit( asm_RCALL(0) ); // RCALL to offset 0 does nothing, except reserving 2 bytes on the stack. cheaper than two useless pushes.
                emit( asm_RCALL(0) );
                emit( asm_RCALL(0) );
                emit( asm_RCALL(0) );

                // PUSH important stuff
                
                // make the call
                jvm_operand_byte0 = dj_di_getU8(code + ++pc);
                jvm_operand_byte1 = dj_di_getU8(code + ++pc);
                emit( asm_LDI(R24, jvm_operand_byte0) ); // infusion id
                emit( asm_LDI(R25, jvm_operand_byte1) ); // entity id
                emit( asm_CALL1((uint16_t)&RTC_INVOKESTATIC) );
                emit( asm_CALL2((uint16_t)&RTC_INVOKESTATIC) );

                // POP important stuff

                // set SP and X to intStack and refStack resp.
                emit( asm_LDI(RZL, ((uint16_t)&(intStack)) & 0xFF) ); // Load the address of intStack into Z
                emit( asm_LDI(RZH, (((uint16_t)&(intStack)) >> 8) & 0xFF) );
                emit( asm_LD_ZINC(R24) ); // Load intStack into R24:25
                emit( asm_LD_Z(R25) );

                emit( asm_STS1(SPaddress_L, R24) ); // Store R24:25 into SP
                emit( asm_STS2(SPaddress_L, R24) ); // Store R24:25 into SP
                emit( asm_STS1(SPaddress_H, R25) ); // Store R24:25 into SP
                emit( asm_STS2(SPaddress_H, R25) ); // Store R24:25 into SP

                emit( asm_POP(R25) ); // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE FIRST LINE FOR INVOKESTATIC. SEE COMMENT ABOVE.

                emit( asm_LDI(RZL, ((uint16_t)&(refStack)) & 0xFF) ); // Load the address of refStack into Z
                emit( asm_LDI(RZH, (((uint16_t)&(refStack)) >> 8) & 0xFF) );
                emit( asm_LD_ZINC(RXL) ); // Load refStack into X
                emit( asm_LD_Z(RXH) );
            break;
            case JVM_ARRAYLENGTH: // The length of an array is stored as 16 bit at the start of the array
                emit( asm_x_POPREF(R31) ); // POP the reference into Z
                emit( asm_x_POPREF(R30) );
                emit( asm_LD_ZINC(R24) );
                emit( asm_LD_Z(R25) );
                emit( asm_PUSH(R25) );
                emit( asm_PUSH(R24) );
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

                emit( asm_POP(R24) );
                emit( asm_POP(R25) );
                // Do the complementary branch. Not taking a branch means jumping over the unconditional branch to the branch target table
                if (opcode == JVM_SIFEQ) {
                    emit( asm_OR(R24, R25) );
                    emit( asm_BRNE(SIZEOF_RJMP) );
                } else if (opcode == JVM_SIFNE) {
                    emit( asm_OR(R24, R25) );
                    emit( asm_BREQ(SIZEOF_RJMP) );
                } else if (opcode == JVM_SIFLT) {
                    emit( asm_SBRC(R25, 7) ); // value is >0 if the highest bit is cleared
                } else if (opcode == JVM_SIFGE) {
                    emit( asm_SBRS(R25, 7) ); // value is <0 if the highest bit is set
                } else if (opcode == JVM_SIFGT) {
                    emit( asm_CP(ZERO_REG, R24) );
                    emit( asm_CPC(ZERO_REG, R25) );
                    emit( asm_BRGE(SIZEOF_RJMP) ); // if (0 >= x), then NOT (x > 0)
                } else if (opcode == JVM_SIFLE) {
                    emit( asm_CP(ZERO_REG, R24) );
                    emit( asm_CPC(ZERO_REG, R25) );
                    emit( asm_BRLT(SIZEOF_RJMP) ); // if (0 < x), then NOT (x <= 0)
                }

                rtc_flush(); // To make sure wkreprog_get_raw_position returns the right value;
                emit( asm_RJMP(rtc_branch_target_table_address(jvm_operand_word) - wkreprog_get_raw_position() - 2) ); // -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
            break;
            case JVM_BRTARGET:
                // This is a noop, but we need to record the address of the next instruction
                // in the branch table as a RJMP instruction.
                tmp_current_position = wkreprog_get_raw_position();
                rtc_flush(); // Not strictly necessary at the moment
                wkreprog_close();
                wkreprog_open_raw(rtc_branch_target_table_address(branch_target_count));
                emit( asm_RJMP(tmp_current_position - rtc_branch_target_table_address(branch_target_count) - 2) ); // Relative jump to tmp_current_position from the branch target table. -2 is because RJMP will add 1 WORD to the PC in addition to the jump offset
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
        // For now, flush after each opcode
        rtc_flush();
    }

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

        rtc_compile_method(methodimpl, infusion);
    }

    wkreprog_close();

    // At this point, the addresses in the rtc_method_start_addresses are 0
    // for the native methods, while the handler table is 0 for the java methods.
    // We need to fill in the addresses in rtc_method_start_addresses in the
    // empty slots in the handler table.
    rtc_update_method_pointers(infusion, rtc_method_start_addresses);

    // Mark the infusion as translated (how?)

}


