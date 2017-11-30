#ifdef AOT_SAFETY_CHECKS

#include "panic.h"
#include "parse_infusion.h"
#include "rtc.h"
#include "rtc_safetychecks.h"
#include "rtc_safetychecks_opcodes.h"

void rtc_safety_abort_with_error(uint8_t error) {
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintHex32(0xDEADC0DE);
    avroraPrintInt16(error);
    dj_panic(DJ_PANIC_UNSAFE_CODE_REJECTED);
}

// max stack should be limited to 240 bytes to prevent counters from wrapping, for example for IDUP_X (just to be on the safe side)

void rtc_safety_check_and_update_stack_depth(uint8_t opcode) {
    uint8_t stack_cons_int = rtc_get_stack_effect(opcode, RTC_STACK_CONS_INT);
    uint8_t stack_cons_ref = rtc_get_stack_effect(opcode, RTC_STACK_CONS_REF);
    uint8_t stack_prod_int = rtc_get_stack_effect(opcode, RTC_STACK_PROD_INT);
    uint8_t stack_prod_ref = rtc_get_stack_effect(opcode, RTC_STACK_PROD_REF);

    avroraPrintHex32(0xABCDEFFF);
    avroraPrintInt16(stack_cons_int);
    avroraPrintInt16(stack_cons_ref);
    avroraPrintInt16(stack_prod_int);
    avroraPrintInt16(stack_prod_ref);

    if (rtc_ts->current_int_stack < stack_cons_int) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_INT_STACK_UNDERFLOW);
    }
    if (rtc_ts->current_ref_stack < stack_cons_ref) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_REF_STACK_UNDERFLOW);
    }

    rtc_ts->current_int_stack -= stack_cons_int;
    rtc_ts->current_ref_stack -= stack_cons_ref;
    rtc_ts->current_int_stack += stack_prod_int;
    rtc_ts->current_ref_stack += stack_prod_ref;

    if (rtc_ts->current_int_stack > dj_di_methodImplementation_getMaxStack(rtc_ts->methodimpl)) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_INT_STACK_OVERFLOW);
    }
    if (rtc_ts->current_ref_stack > dj_di_methodImplementation_getMaxStack(rtc_ts->methodimpl)) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_REF_STACK_OVERFLOW);
    }    
}

void rtc_safety_process_opcode(uint8_t opcode) {
    rtc_ts->last_opcode = opcode;

    rtc_safety_check_and_update_stack_depth(opcode);

    if (rtc_ts->current_int_stack != 0 || rtc_ts->current_ref_stack != 0) {
        if (RTC_OPCODE_IS_RETURN(opcode)) {
            rtc_safety_abort_with_error(RTC_SAFETYCHECK_STACK_NOT_EMPTY_AFTER_RETURN);
        }

        if (RTC_OPCODE_IS_BRANCH(opcode)) {
            rtc_safety_abort_with_error(RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCH);
        }

        if (RTC_OPCODE_IS_BRTARGET(opcode)) {
            rtc_safety_abort_with_error(RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCHTARGET);
        }
    }
}

void rtc_safety_method_ends() {
    // if (!RTC_OPCODE_IS_BRANCH_OR_RETURN(rtc_ts->last_opcode)) {
    //     rtc_safety_abort_with_error(RTC_SAFETYCHECK_METHOD_SHOULD_END_IN_BRANCH_OR_RETURN);
    // }
}

#endif // AOT_SAFETY_CHECKS
