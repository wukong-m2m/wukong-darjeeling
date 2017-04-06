#include "rtc.h"
#include "asm.h"
#include "parse_infusion.h"

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
	rtc_ts->current_method_used_call_saved_reg |= 1<<((reg-2)/2);
}

bool rtc_current_method_is_lightweight() {
	return dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_LIGHTWEIGHT;
}

bool rtc_current_method_get_uses_reg(uint8_t reg) {
#ifdef AOT_STRATEGY_MARKLOOP	
	return (rtc_ts->current_method_used_call_saved_reg & 1<<((reg-2)/2)) != 0;
#else // TODO: implement this optimisation for other strategies as well
	return true;
#endif
}

void rtc_emit_prologue() {
	if (rtc_current_method_is_lightweight()) {
		// Lightweight methods are directly CALLed, so the return pointer is on the stack, hiding the JVM operand stack.
		// For these methods we won't use R18:R19 for stack caching, but use them to pop the return value into. The reduced
		// number of registers available for stack caching probably won't be a problem since they are typically small methods.

		// We also don't need to PUSH the call saved registers that will be used in this method. (this would also hide the
		// operands) We don't follow the normal avr-gcc ABI here. Lightweight methods can only be called from the context
		// of an AOT compiled method, and the INVOKELIGHT instruction will guarantee all registers are not used.
		emit_POP(R18);
		emit_POP(R19);		
	} else {
		if (rtc_current_method_get_uses_reg(R2)) {
			emit_PUSH(R2);
			emit_PUSH(R3);
			uint16_t rtc_statics_start = (uint16_t)rtc_ts->infusion->staticReferenceFields;
			emit_LDI(R20, rtc_statics_start&0xFF);
			emit_LDI(R21, (rtc_statics_start>>8)&0xFF);
			emit_MOVW(R2, R20);
		}
		if (rtc_current_method_get_uses_reg(R4)) {
			emit_PUSH(R4);
			emit_PUSH(R5);
		}
		if (rtc_current_method_get_uses_reg(R6)) {
			emit_PUSH(R6);
			emit_PUSH(R7);
		}
		if (rtc_current_method_get_uses_reg(R8)) {
			emit_PUSH(R8);
			emit_PUSH(R9);
		}
		if (rtc_current_method_get_uses_reg(R10)) {
			emit_PUSH(R10);
			emit_PUSH(R11);
		}
		if (rtc_current_method_get_uses_reg(R12)) {
			emit_PUSH(R12);
			emit_PUSH(R13);
		}
		if (rtc_current_method_get_uses_reg(R14)) {
			emit_PUSH(R14);
			emit_PUSH(R15);
		}
		if (rtc_current_method_get_uses_reg(R16)) {
			emit_PUSH(R16);
			emit_PUSH(R17);
		}
		emit_PUSH(R28);
		emit_PUSH(R29);
		emit_MOVW(R28, R24);
		emit_MOVW(R26, R22);
	}
}

void rtc_emit_epilogue() {
	if (rtc_current_method_is_lightweight()) {
		// Push the return address back on the stack for lightweight methods.
		emit_PUSH(R19);
		emit_PUSH(R18);
	} else {
		emit_POP(R29);
		emit_POP(R28);
		if (rtc_current_method_get_uses_reg(R16)) {
			emit_POP(R17);
			emit_POP(R16);
		}
		if (rtc_current_method_get_uses_reg(R14)) {
			emit_POP(R15);
			emit_POP(R14);
		}
		if (rtc_current_method_get_uses_reg(R12)) {
			emit_POP(R13);
			emit_POP(R12);
		}
		if (rtc_current_method_get_uses_reg(R10)) {
			emit_POP(R11);
			emit_POP(R10);
		}
		if (rtc_current_method_get_uses_reg(R8)) {
			emit_POP(R9);
			emit_POP(R8);
		}
		if (rtc_current_method_get_uses_reg(R6)) {
			emit_POP(R7);
			emit_POP(R6);
		}
		if (rtc_current_method_get_uses_reg(R4)) {
			emit_POP(R5);
			emit_POP(R4);
		}
		if (rtc_current_method_get_uses_reg(R2)) {
			emit_POP(R3);
			emit_POP(R2);
		}
	}
	emit_RET();
}