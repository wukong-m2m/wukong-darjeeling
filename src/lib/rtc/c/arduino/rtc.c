#include <stddef.h>
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
#include "rtc.h"
#include "rtc_instructions.h"
#include "rtc_branches.h"
#include "rtc_emit.h"
#include "rtc_prologue_epilogue.h"
#include "opcodes.h"
#include <avr/pgmspace.h>
#include <avr/boot.h>
#ifdef AOT_STRATEGY_SIMPLESTACKCACHE    
#include "rtc_simplestackcache.h"
#endif
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE
#include "rtc_poppedstackcache.h"
#endif
#ifdef AOT_STRATEGY_MARKLOOP
#include "rtc_markloop.h"
#endif

// Store a global pointer to the translation state. This will point to a big struct on the heap that
// will contain all the state necessary for AOT translation.
rtc_translationstate *rtc_ts;

// This is placed at the very end of the .text section by the linker. There shouldn't be anything following this marker.
// We will use all memory above MAX(rtc_rtc_start_of_compiled_code_marker, 65536) for AOT code. (so all compiled code
// will be in far memory)
const unsigned char __attribute__((section (".rtc_code_marker"))) __attribute__ ((aligned (2))) rtc_start_of_compiled_code_marker;

// Since we're on ATmega128, and flash address / 2 should fit in a uint16_t.
uint16_t rtc_start_of_next_method;

// Offsets for static variables in an infusion, relative to the start of infusion->staticReferencesFields. (referenced infusion pointers follow the static variables)
uint16_t rtc_offset_for_static_ref(dj_infusion *infusion_ptr, uint8_t variable_index)   { return ((uint16_t)((void*)(&((infusion_ptr)->staticReferenceFields[variable_index])) - (void *)((infusion_ptr)->staticReferenceFields))); }
uint16_t rtc_offset_for_static_byte(dj_infusion *infusion_ptr, uint8_t variable_index)  { return ((uint16_t)((void*)(&((infusion_ptr)->staticByteFields[variable_index]))      - (void *)((infusion_ptr)->staticReferenceFields))); }
uint16_t rtc_offset_for_static_short(dj_infusion *infusion_ptr, uint8_t variable_index) { return ((uint16_t)((void*)(&((infusion_ptr)->staticShortFields[variable_index]))     - (void *)((infusion_ptr)->staticReferenceFields))); }
uint16_t rtc_offset_for_static_int(dj_infusion *infusion_ptr, uint8_t variable_index)   { return ((uint16_t)((void*)(&((infusion_ptr)->staticIntFields[variable_index]))       - (void *)((infusion_ptr)->staticReferenceFields))); }
uint16_t rtc_offset_for_static_long(dj_infusion *infusion_ptr, uint8_t variable_index)  { return ((uint16_t)((void*)(&((infusion_ptr)->staticLongFields[variable_index]))      - (void *)((infusion_ptr)->staticReferenceFields))); }
uint16_t rtc_offset_for_referenced_infusion(dj_infusion *infusion_ptr, uint8_t ref_inf) { return ((uint16_t)((void*)(&((infusion_ptr)->referencedInfusions[ref_inf-1]))        - (void *)((infusion_ptr)->staticReferenceFields))); }

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
    if (offset > 63) {
        dj_panic(DJ_PANIC_OFFSET_TOO_LARGE);
    }
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

void rtc_update_method_pointers(dj_infusion *infusion, native_method_function_t *rtc_method_start_addresses) {
    DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
    uint16_t native_handlers_address = (uint16_t)infusion->native_handlers;
    wkreprog_open_raw(native_handlers_address, RTC_END_OF_COMPILED_CODE_SPACE);

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

void rtc_compile_method(dj_di_pointer methodimpl) {
#ifdef AVRORA
    // avroraStartRTCCompileTimer();
#endif

    // Buffer to hold the code we're building (want to keep this on the stack so it doesn't take up space at runtime)
    emit_init(rtc_ts->codebuffer); // Tell emit where the buffer is

    // Remember the start of the branch table
    uint_farptr_t branch_target_table_start_ptr = wkreprog_get_raw_position();
    uint_farptr_t prologue_start_ptr = branch_target_table_start_ptr + rtc_branch_table_size(methodimpl);
    uint_farptr_t code_start_ptr = prologue_start_ptr + RTC_PROLOGUE_MAX_SIZE;

    // Reserve space for the branch table
    DEBUG_LOG(DBG_RTC, "[rtc] Reserving %d bytes for %d branch targets at address %p\n", rtc_branch_table_size(methodimpl), dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl), branch_target_table_start_ptr);
    wkreprog_skip(rtc_branch_table_size(methodimpl) + RTC_PROLOGUE_MAX_SIZE);

    // The method prologue/epilogue will depend on the registers used.
    //  - reserve RTC_PROLOGUE_MAX_SIZE bytes
    //  - generate code without the prologue
    //  - after generation go back to prologue_start_ptr
    //  - generate prologue
    //  - prologue_end_ptr = wkreprog_get_raw_position();
    //  - move code at code_start_ptr back by (code_start_ptr-prologue_end_ptr)
    //  - compensate patch_branches by (code_start_ptr-prologue_end_ptr) bytes to account for bytes saved in the prologue

    rtc_ts->pc = 0;
    rtc_ts->methodimpl = methodimpl;
    rtc_ts->jvm_code_start = dj_di_methodImplementation_getData(methodimpl);
    rtc_ts->method_length = dj_di_methodImplementation_getLength(methodimpl);
    rtc_ts->branch_target_table_start_ptr = branch_target_table_start_ptr;
    rtc_ts->branch_target_count = 0;
    if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_USESSTATICFIELDS) {
        rtc_current_method_set_uses_reg(R2);
    }
#if defined(AOT_OPTIMISE_CONSTANT_SHIFTS)
    rtc_ts->do_CONST_SHIFT_optimisation = 0;
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS

    // If we're using stack caching, initialise the cache
#ifdef AOT_STRATEGY_SIMPLESTACKCACHE    
    rtc_stackcache_init();
#endif
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE    
    rtc_stackcache_init();
#endif
#ifdef AOT_STRATEGY_MARKLOOP
    rtc_ts->may_use_RZ = false;
    rtc_stackcache_init(dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_LIGHTWEIGHT);
#endif

    // translate the method
    DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", ts.method_length);
    while (rtc_ts->pc < rtc_ts->method_length) {
        rtc_translate_single_instruction();
    }

    rtc_mark_branchtarget(); // Mark the location of the epilogue
    rtc_emit_epilogue(); // Emit epilogue for used registers only

    // record the position of the end of the epilogue
    emit_flush_to_flash();
    uint_farptr_t code_end_ptr = wkreprog_get_raw_position();
    wkreprog_close();

// Let RTC trace know the next emitted code won't belong to the prologue anymore. Otherwise the branch patches would be assigned to the last instruction in the method (probably RET)
#ifdef AVRORA
    avroraRTCTraceEmitPrologue();
#endif

    // go back to the start of the method and emit the prologue for used registers only
    wkreprog_open_raw(prologue_start_ptr, RTC_END_OF_COMPILED_CODE_SPACE);
    rtc_emit_prologue();
    emit_flush_to_flash();

// Let RTC trace know the next emitted code won't belong to the prologue anymore. Otherwise the branch patches would be assigned to the last instruction in the method (probably RET)
#ifdef AVRORA
    avroraRTCTracePatchingBranchesOn();
#endif

    // this is how many bytes we saved, and will shift the code block by,
    // because we used a smaller than maximum prologue.
    uint8_t shift_because_of_smaller_prologue = code_start_ptr - wkreprog_get_raw_position();



    // shift all code back by this many bytes
    for (uint_farptr_t pos = code_start_ptr; pos < code_end_ptr; pos+=2) {
        emit_without_optimisation(dj_di_getU16(pos));
    }
    emit_flush_to_flash();

    wkreprog_close();

    // All branchtarget addresses should be known now.
    // Scan for branch tags, and replace them with the proper instructions.
    rtc_patch_branches(branch_target_table_start_ptr,
                       code_end_ptr - shift_because_of_smaller_prologue,
                       rtc_branch_table_size(methodimpl),
                       shift_because_of_smaller_prologue);
//avroraStopRTCCompileTimer();
// Let RTC trace know we're done patching branches
#ifdef AVRORA
    avroraRTCTracePatchingBranchesOff();
#endif
}

void rtc_compile_lib(dj_infusion *infusion) {
#ifdef AVRORA
    avroraRTCSetCurrentInfusion(dj_di_header_getInfusionName(infusion->header));
#endif

    // Allocate memory for translation state datastructure
    rtc_ts = dj_mem_checked_alloc(sizeof(rtc_translationstate), CHUNKID_RTC_TSSTATE);
    dj_mem_addSafePointer((void**)&rtc_ts); // GC shouldn't run during RTC, but just to be safe.

    // uses 512bytes on the stack... maybe optimise this later
    // RTC code should always be in the 64K-128K segment.
    // Luckily we will store function pointers as word addresses, so this should still fit in 16 bit elements.
    native_method_function_t method_start_addresses[256];
    uint8_t call_saved_registers_used_per_method[256];
    for (uint16_t i=0; i<256; i++) {
        method_start_addresses[i] = 0;
        call_saved_registers_used_per_method[i] = 0;
    }
    rtc_ts->call_saved_registers_used_per_method = call_saved_registers_used_per_method;
    rtc_ts->method_start_addresses = method_start_addresses;
    rtc_ts->infusion = infusion;

    // rtc_start_of_next_method contains the address/2 so that any address < 128K will fit in a uint16_t.
    wkreprog_open_raw(((uint32_t)rtc_start_of_next_method)*2, RTC_END_OF_COMPILED_CODE_SPACE);

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
        
        DEBUG_LOG(DBG_RTC, "[rtc] compiling method %d\n", i);

        // store the starting address for this method;
        // IMPORTANT!!!! the PC in AVR stores WORD addresses, so we need to divide the address
        // of a function by 2 in order to get a function pointer!
        uint_farptr_t method_address = wkreprog_get_raw_position() + rtc_branch_table_size(methodimpl);
        method_start_addresses[i] = (native_method_function_t)(uint16_t)(method_address/2);

#ifdef AVRORA
        avroraRTCTraceStartMethod(i, wkreprog_get_raw_position());
#endif

        rtc_ts->current_method_index = i;
        rtc_compile_method(methodimpl);

        emit_flush_to_flash();
        // rtc_start_of_next_method contains the address/2 so that any address < 128K will fit in a uint16_t.
        RTC_SET_START_OF_NEXT_METHOD(wkreprog_get_raw_position());
#ifdef AVRORA
        // Don't really need to do this unless we want to print the contents of Flash memory at this point.
        wkreprog_close();
        wkreprog_open_raw(RTC_GET_START_OF_NEXT_METHOD(), RTC_END_OF_COMPILED_CODE_SPACE);
        avroraRTCTraceEndMethod(wkreprog_get_raw_position(), dj_di_methodImplementation_getLength(methodimpl), rtc_branch_table_size(methodimpl)/4);
#endif
    }


    // At this point, the addresses in the method_start_addresses are 0
    // for the native methods, while the handler table is 0 for the java methods.
    // We need to fill in the addresses in method_start_addresses in the
    // empty slots in the handler table.
    rtc_update_method_pointers(infusion, method_start_addresses);

    // Free memory for translation state datastructure
    dj_mem_removeSafePointer((void**)&rtc_ts);
    dj_mem_free(rtc_ts);
    rtc_ts = NULL;
}

uint8_t rtc_number_of_operandbytes_for_opcode(uint8_t opcode) {
    if (opcode == JVM_BSPUSH
        || opcode == JVM_BIPUSH
        || opcode == JVM_SLOAD
        || opcode == JVM_ILOAD
        || opcode == JVM_ALOAD
        || opcode == JVM_SSTORE
        || opcode == JVM_ISTORE
        || opcode == JVM_ASTORE
        || opcode == JVM_NEWARRAY
        || opcode == JVM_IDUP_X
        || opcode == JVM_ISWAP_X) {
        return 1;
    }

    if (opcode == JVM_SSPUSH
        || opcode == JVM_SIPUSH
        || opcode == JVM_LDS
        || (opcode >= JVM_GETFIELD_B && opcode <= JVM_PUTSTATIC_A)
        || opcode == JVM_SINC
        || opcode == JVM_IINC
        || opcode == JVM_INVOKESPECIAL
        || opcode == JVM_INVOKESTATIC
        || opcode == JVM_INVOKELIGHT
        || opcode == JVM_NEW
        || opcode == JVM_ANEWARRAY
        || opcode == JVM_CHECKCAST
        || opcode == JVM_INSTANCEOF) {
        return 2;
    }

    if (opcode == JVM_SINC_W
        || opcode == JVM_IINC_W
        || opcode == JVM_INVOKEVIRTUAL
        || opcode == JVM_INVOKEINTERFACE) {
        return 3;
    }

    if (opcode == JVM_IIPUSH
        || opcode == JVM_GETFIELD_A_FIXED
        || opcode == JVM_PUTFIELD_A_FIXED
        || (opcode >= JVM_IIFEQ && opcode <= JVM_GOTO)
        || (opcode >= JVM_SIFEQ && opcode <= JVM_SIFLE)) {
        return 4;
    }


    // JVM_TABLESWITCH
    // JVM_LOOKUPSWITCH
    // need to skip a lot, but we'll handle during codegen.
    // these won't be optimised away by stackcaching anyway.

    // JVM_MARKLOOP_START
    // need to skip a lot, but we'll handle during codegen.
    // these won't be optimised away by stackcaching anyway.

    return 0;
}

    // uint8_t current_method_index;
    // uint8_t *call_saved_registers_used_per_method // Used to generate the method prologue/epilogue and by INVOKELIGHT inside of a MARKLOOP loop to determine which pinned registers to save/restore.

void rtc_current_method_set_uses_reg(uint8_t reg) {
    // R2 : bit 0
    // R4 : bit 1
    // R6 : bit 2
    // R8 : bit 3
    // R10 : bit 4
    // R12 : bit 5
    // R14 : bit 6
    // R16 : bit 7

    // This makes sure only R2 through R16 will update a bit. We can safely call it with other values, but current_method_used_call_saved_reg will not be affected
    rtc_ts->call_saved_registers_used_per_method[rtc_ts->current_method_index] |= 1<<((reg-2)/2);
}

void rtc_current_method_set_uses_reg_used_in_lightweight_invoke(uint8_t lightweightmethod_id) {
    // a method calling a lightweight method will have to mark all the call saved registers
    // used by the lightweight method as used in it's own context.
    // (since the calling method will be responsible for saving them in it's prologue)
    rtc_ts->call_saved_registers_used_per_method[rtc_ts->current_method_index] |= rtc_ts->call_saved_registers_used_per_method[lightweightmethod_id];
}

bool rtc_method_get_uses_reg(uint8_t method, uint8_t reg) {
#ifdef AOT_STRATEGY_MARKLOOP    
    return (rtc_ts->call_saved_registers_used_per_method[method] & 1<<((reg-2)/2)) != 0;
#else // TODO: implement this optimisation for other strategies as well
    return true;
#endif
}

bool rtc_current_method_get_uses_reg(uint8_t reg) {
    return rtc_method_get_uses_reg(rtc_ts->current_method_index, reg);
}
