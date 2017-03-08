#include "rtc.h"
#include "asm.h"

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

bool rtc_current_method_get_uses_reg(uint8_t reg) {
#ifdef AOT_STRATEGY_MARKLOOP	
	return (rtc_ts->current_method_used_call_saved_reg & 1<<((reg-2)/2)) != 0;
#else // TODO: implement this optimisation for other strategies as well
	return true;
#endif
}

uint8_t rtc_current_method_prologue_size() {
	// Maximum prologue if all registers and local statics are used:
	// PUSH(R2),
	// PUSH(R3),
	// PUSH(R4),
	// PUSH(R5),
	// PUSH(R6),
	// PUSH(R7),
	// PUSH(R8),
	// PUSH(R9),
	// PUSH(R10),
	// PUSH(R11),
	// PUSH(R12),
	// PUSH(R13),
	// PUSH(R14),
	// PUSH(R15),
	// PUSH(R16),
	// PUSH(R17),
	// PUSH(R28),
	// PUSH(R29),
	// MOVW(R28, R24),
	// MOVW(R26, R22),
	// MOVW(R2, R20),
	return rtc_current_method_get_uses_reg(R2)*10 // 10 instead of 4 because we also need to emit the LDIs and MOVW
		 + rtc_current_method_get_uses_reg(R4)*4
		 + rtc_current_method_get_uses_reg(R6)*4
		 + rtc_current_method_get_uses_reg(R8)*4
		 + rtc_current_method_get_uses_reg(R10)*4
		 + rtc_current_method_get_uses_reg(R12)*4
		 + rtc_current_method_get_uses_reg(R14)*4
		 + rtc_current_method_get_uses_reg(R16)*4
		 + 4; // for the MOVW to R28 and R26
}

void rtc_emit_prologue() {
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

void rtc_emit_epilogue() {
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
	emit_RET();
}