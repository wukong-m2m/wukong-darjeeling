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
		if (dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_USES_SIMUL_OR_INVOKESTATIC) {
			// The method calling this lightweight method will have reserved space for this method
			// and moved Y to point at the start of it. If this method uses SIMUL or INVOKELIGHT,
			// we cannot use R18:R19 to store the return address since it would be overwritten
			// by those instructions.
			// Instead we will store it at the beginning of the locals, and move Y forward another
			// two bytes. The calling method will have reserved the extra two bytes for this.
			emit_ST_YINC(R4);
			emit_ST_YINC(R5);
		}
#else
		emit_POP(R18);
		emit_POP(R19);
		if (dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_USES_SIMUL_OR_INVOKESTATIC) {
			// The method calling this lightweight method will have reserved space for this method
			// and moved Y to point at the start of it. If this method uses SIMUL or INVOKELIGHT,
			// we cannot use R18:R19 to store the return address since it would be overwritten
			// by those instructions.
			// Instead we will store it at the beginning of the locals, and move Y forward another
			// two bytes. The calling method will have reserved the extra two bytes for this.
			emit_ST_YINC(R18);
			emit_ST_YINC(R19);
		}
#endif
	} else {
		for (uint8_t reg=R2; reg<=R16; reg+=2) {
			if (rtc_current_method_get_uses_reg(reg)) {
				emit_PUSH(reg);
				emit_PUSH(reg+1);
				if(reg==R2) {
					uint16_t rtc_statics_start = (uint16_t)rtc_ts->infusion->staticReferenceFields;
					emit_LDI(R20, rtc_statics_start&0xFF);
					emit_LDI(R21, (rtc_statics_start>>8)&0xFF);
					emit_MOVW(R2, R20);
				}
			}
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
		if (dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_USES_SIMUL_OR_INVOKESTATIC) {
			// See comment in prologue
			emit_LD_DECY(R5);
			emit_LD_DECY(R4);
		}
		emit_PUSH(R5);
		emit_PUSH(R4);
#else
		if (dj_di_methodImplementation_getFlags(rtc_ts->methodimpl) & FLAGS_USES_SIMUL_OR_INVOKESTATIC) {
			// See comment in prologue
			emit_LD_DECY(R19);
			emit_LD_DECY(R18);
		}
		emit_PUSH(R19);
		emit_PUSH(R18);
#endif
	} else {
		emit_POP(R29);
		emit_POP(R28);


		for (uint8_t reg=R16; reg>=R2; reg-=2) {
			if (rtc_current_method_get_uses_reg(reg)) {
				emit_POP(reg+1);
				emit_POP(reg);
			}
		}
	}
	emit_RET();
}