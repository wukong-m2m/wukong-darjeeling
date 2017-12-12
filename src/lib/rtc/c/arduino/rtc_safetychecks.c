#ifdef AOT_SAFETY_CHECKS

#include "parse_infusion.h"
#include "rtc.h"
#include "rtc_safetychecks.h"
#include "rtc_safetychecks_opcodes.h"

// max stack should be limited to 240 bytes to prevent counters from wrapping, for example for IDUP_X (just to be on the safe side)

void rtc_safety_method_starts() {
    // Stack depths are initialised to 0, but for lightweight methods the arguments are passed on the stack.
    if (rtc_ts->methodimpl_header.flags & FLAGS_LIGHTWEIGHT) {
        rtc_ts->pre_instruction_int_stack = rtc_ts->methodimpl_header.nr_int_args;
        rtc_ts->pre_instruction_ref_stack = rtc_ts->methodimpl_header.nr_ref_args;
    }

    // Check method header fields make sense
    if  ((rtc_ts->methodimpl_header.flags & FLAGS_STATIC) && rtc_ts->methodimpl_header.length == 0) {
        // Static methods can't be abstract
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_INCORRECT_METHOD_HEADER);
    }
    if (rtc_ts->methodimpl_header.length > 0 // Skip abstract methods
        && (
            // Can't have more ref arguments than ref local variables
            (rtc_ts->methodimpl_header.nr_ref_args > rtc_ts->methodimpl_header.nr_ref_vars)
            // Can't have more int arguments than int local variables
            || (rtc_ts->methodimpl_header.nr_int_args > rtc_ts->methodimpl_header.nr_int_vars)
            // Number of own variable slots must be sum of ref and int slots
            || (rtc_ts->methodimpl_header.nr_own_var_slots != rtc_ts->methodimpl_header.nr_ref_vars + rtc_ts->methodimpl_header.nr_int_vars)
            // Number of own variable slots must be <= total variable slots (extras are used for lightweight methods)
            || (rtc_ts->methodimpl_header.nr_own_var_slots > rtc_ts->methodimpl_header.nr_total_var_slots)
            )
    ) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_INCORRECT_METHOD_HEADER);
    }
}

void rtc_safety_check_offset_valid_for_local_variable(uint16_t offset) {
    if (offset >= (2 * rtc_ts->methodimpl_header.nr_total_var_slots)) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_STORE_TO_NONEXISTANT_LOCAL_VARIABLE);
    }
}

uint16_t rtc_safety_check_offset_valid_for_static_variable(dj_infusion *infusion_ptr, uint8_t size, volatile uint16_t offset) {
    // the layout of the infusion data structure is like this:
    //  struct dj_infusion
    //  static ref fields
    //  static byte fields
    //  static short fields
    //  static int fields
    //  pointers to referenced infusions
    uint16_t sizeOfStaticFields = (void*)(infusion_ptr->referencedInfusions) - (void*)(infusion_ptr->staticReferenceFields);

    if (offset + size > sizeOfStaticFields) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_STORE_TO_NONEXISTANT_STATIC_VARIABLE);        
    }
    return offset;
}

void rtc_safety_check_opcode(uint8_t opcode) {
    uint8_t stack_cons_int = rtc_safety_get_stack_effect(opcode, RTC_STACK_CONS_INT);
    uint8_t stack_cons_ref = rtc_safety_get_stack_effect(opcode, RTC_STACK_CONS_REF);
    uint8_t stack_prod_int = rtc_safety_get_stack_effect(opcode, RTC_STACK_PROD_INT);
    uint8_t stack_prod_ref = rtc_safety_get_stack_effect(opcode, RTC_STACK_PROD_REF);

    // Check for stack underflow
    if (rtc_ts->pre_instruction_int_stack < stack_cons_int) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_INT_STACK_UNDERFLOW);
    }
    if (rtc_ts->pre_instruction_ref_stack < stack_cons_ref) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_REF_STACK_UNDERFLOW);
    }

    // Set pre instruction value for the next instruction (it's now this instruction's post value)
    rtc_ts->pre_instruction_int_stack -= stack_cons_int;
    rtc_ts->pre_instruction_ref_stack -= stack_cons_ref;
    rtc_ts->pre_instruction_int_stack += stack_prod_int;
    rtc_ts->pre_instruction_ref_stack += stack_prod_ref;

    // Check for stack overflow
    if (rtc_ts->pre_instruction_int_stack > rtc_ts->methodimpl_header.max_int_stack) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_INT_STACK_OVERFLOW);
    }
    if (rtc_ts->pre_instruction_ref_stack > rtc_ts->methodimpl_header.max_ref_stack) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_REF_STACK_OVERFLOW);
    }    

    if (rtc_ts->pre_instruction_int_stack != 0 || rtc_ts->pre_instruction_ref_stack != 0) {
        if (RTC_OPCODE_IS_RETURN(opcode) || RTC_OPCODE_IS_BRANCH(opcode) || RTC_OPCODE_IS_BRTARGET(opcode)) {
            rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_STACK_NOT_EMPTY_AFTER_RETURN_OR_BRANCH);
        }
    }
}

void rtc_safety_method_ends() {
    if (!(RTC_OPCODE_IS_RETURN(rtc_ts->current_opcode)
          || RTC_OPCODE_IS_BRANCH(rtc_ts->current_opcode))) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_METHOD_SHOULD_END_IN_BRANCH_OR_RETURN);
    }

    if (rtc_ts->methodimpl_header.nr_branch_targets != rtc_ts->branch_target_count) {
        rtc_safety_abort_with_error(RTC_SAFETY_TRANSLATIONCHECK_BRANCHTARGET_COUNT_MISMATCH_WITH_METHOD_HEADER);        
    }
}


void rtc_safety_mem_check() {

    asm volatile("   lds  r0, rtc_safety_heap_lowbound" "\n\r"
                 "   cp   r30, r0" "\n\r"
                 "   lds  r0, rtc_safety_heap_lowbound+1" "\n\r"
                 "   cpc  r31, r0" "\n\r"
                 "   brlo 1f" "\n\r"
                 "   lds  r0, right_pointer" "\n\r"
                 "   cp   r0, r30" "\n\r"
                 "   lds  r0, right_pointer+1" "\n\r"
                 "   cpc  r0, r31" "\n\r"
                 "   brlo 1f" "\n\r"
                 "   ret" "\n\r"
                 "1: ldi  r24, %[errorcode]" "\n\r"
                 "   push r16" "\n\r" // avroraPrintRegs
                 "   ldi  r16, %[printreg]" "\n\r"
                 "   sts  debugbuf1, r16" "\n\r"
                 "   pop  r16" "\n\r"
                 "   call rtc_safety_abort_with_error" "\n\r"
             :: [errorcode] "M" (RTC_SAFETY_RUNTIMECHECK_ILLEGAL_MEMORY_ACCESS), [printreg] "M" (0xE));
}

#endif // AOT_SAFETY_CHECKS

