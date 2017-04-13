#include "rtc.h"
#include "asm.h"
#include "parse_infusion.h"

bool rtc_current_method_is_lightweight() {
	return dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_LIGHTWEIGHT;
}

void rtc_emit_prologue() {
	if (rtc_current_method_is_lightweight()) {
		// Lightweight methods are directly CALLed, so the return pointer is on the stack, hiding the JVM operand stack.
		// For these methods we won't use R18:R19 for stack caching, but use them to pop the return value into. The reduced
		// number of registers available for stack caching probably won't be a problem since they are typically small methods.

		// We also don't need to PUSH the call saved registers that will be used in this method. (this would also hide the
		// operands) We don't follow the normal avr-gcc ABI here. Lightweight methods can only be called from the context
		// of an AOT compiled method, and the INVOKELIGHT instruction will guarantee all registers are not used.
#if defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
		// Baseline uses hardcoded registers, which uses R18:R19 in some cases.
		// Use R4:R5 for return address instead
		emit_POP(R4);
		emit_POP(R5);
#else
		emit_POP(R18);
		emit_POP(R19);
#endif
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
#if defined (AOT_STRATEGY_BASELINE)  || defined (AOT_STRATEGY_IMPROVEDPEEPHOLE)
		// Baseline uses hardcoded registers, which uses R18:R19 in some cases.
		// Use R4:R5 for return address instead
		emit_PUSH(R5);
		emit_PUSH(R4);
#else
		emit_PUSH(R19);
		emit_PUSH(R18);
#endif
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