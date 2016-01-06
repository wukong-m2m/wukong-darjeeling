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
#include <avr/pgmspace.h>
#include <avr/boot.h>
#ifdef AOT_STRATEGY_SIMPLESTACKCACHE    
#include "rtc_simplestackcache.h"
#endif
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE    
#include "rtc_poppedstackcache.h"
#endif


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

// USED AT COMPILE TIME:
const unsigned char PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) rtc_compiled_code_buffer[RTC_COMPILED_CODE_BUFFER_SIZE] = {};
#define END_OF_SAFE_REGION ((dj_di_pointer)rtc_compiled_code_buffer + RTC_COMPILED_CODE_BUFFER_SIZE)
// Buffer for emitting code.

void rtc_update_method_pointers(dj_infusion *infusion, native_method_function_t *rtc_method_start_addresses) {
    DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
    uint16_t native_handlers_address = (uint16_t)infusion->native_handlers;
    wkreprog_open_raw(native_handlers_address, END_OF_SAFE_REGION);

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

void rtc_compile_method(dj_di_pointer methodimpl, dj_infusion *infusion) {

    // Buffer to hold the code we're building (want to keep this on the stack so it doesn't take up space at runtime)
    uint16_t codebuffer[RTC_CODEBUFFER_SIZE];
    emit_init(codebuffer); // Tell emit where the buffer is

    // Remember the start of the branch table
    dj_di_pointer branch_target_table_start_ptr = wkreprog_get_raw_position();
    // Reserve space for the branch table
    DEBUG_LOG(DBG_RTC, "[rtc] Reserving %d bytes for %d branch targets at address %p\n", rtc_branch_table_size(methodimpl), dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl), branch_target_table_start_ptr);
    wkreprog_skip(rtc_branch_table_size(methodimpl));
    // If we're using stack caching, initialise the cache
#ifdef AOT_STRATEGY_SIMPLESTACKCACHE    
    rtc_stackcache_init();
#endif
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE    
    rtc_stackcache_init();
#endif

    emit_x_prologue();

    // translate the method
    DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", method_length);

    uint16_t pc = 0;
    rtc_translationstate translationstate;
    translationstate.infusion = infusion;
    translationstate.methodimpl = methodimpl;
    translationstate.jvm_code_start = dj_di_methodImplementation_getData(methodimpl);
    translationstate.method_length = dj_di_methodImplementation_getLength(methodimpl);
    translationstate.branch_target_table_start_ptr = branch_target_table_start_ptr;
    translationstate.end_of_safe_region = END_OF_SAFE_REGION;
    translationstate.branch_target_count = 0;
#ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
    translationstate.rtc_next_instruction_shifts_1_bit = false;
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS
    
    while (pc < translationstate.method_length) {
        pc = rtc_translate_single_instruction(pc, &translationstate);
    }

    emit_flush_to_flash();

    dj_di_pointer tmp_current_position = wkreprog_get_raw_position();
    wkreprog_close();

    // Second pass:
    // All branchtarget addresses should be known now.
    // Scan for branch tags, and replace them with the proper instructions.
    rtc_patch_branches(branch_target_table_start_ptr, END_OF_SAFE_REGION, rtc_branch_table_size(methodimpl), tmp_current_position);
}

void rtc_compile_lib(dj_infusion *infusion) {
    // uses 512bytes on the stack... maybe optimise this later
    native_method_function_t rtc_method_start_addresses[256];
    for (uint16_t i=0; i<256; i++)
        rtc_method_start_addresses[i] = 0;

    wkreprog_open_raw((dj_di_pointer)rtc_compiled_code_buffer, END_OF_SAFE_REGION);

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
    // Don't really need to do this unless we want to print the contents of Flash memory at this point.
    emit_flush_to_flash();
    dj_di_pointer tmp_address = wkreprog_get_raw_position();
    wkreprog_close();
    wkreprog_open_raw(tmp_address, END_OF_SAFE_REGION);
    avroraRTCTraceEndMethod(wkreprog_get_raw_position(), dj_di_methodImplementation_getLength(methodimpl), rtc_branch_table_size(methodimpl)/2);
#endif
    }


    // At this point, the addresses in the rtc_method_start_addresses are 0
    // for the native methods, while the handler table is 0 for the java methods.
    // We need to fill in the addresses in rtc_method_start_addresses in the
    // empty slots in the handler table.
    rtc_update_method_pointers(infusion, rtc_method_start_addresses);

    // Mark the infusion as translated (how?)

}


