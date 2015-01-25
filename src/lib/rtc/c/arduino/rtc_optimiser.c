#include <string.h>
#include "panic.h"
#include "asm.h"

#define IS_PUSH(x)			(((x) & 0xFE0F) == OPCODE_PUSH)
#define IS_POP(x)			(((x) & 0xFE0F) == OPCODE_POP)

#define GET_1REG_OPERAND(x)	((x) & 0x01F0)

#define OPCODE_PUSH                     0x920F
#define asm_PUSH(reg)                   opcodeWithSingleRegOperand(OPCODE_PUSH, reg)

// POP                                  1001 000d dddd 1111
#define OPCODE_POP                      0x900F

void rtc_optimise_drop_2_instructions(uint16_t *first_instr, uint16_t **code_end) {
	void *address = (void *)first_instr;

	// avroraPrintStr("rtc_optimise_drop_2_instructions\n\r");
	// avroraPrintPtr(first_instr);
	// avroraPrintPtr(*code_end);
	// avroraPrintPtr(address);
	// avroraPrintPtr((address+4));
	// avroraPrintHex32((((void *)*code_end) - (address+4)));

	memcpy (address, address+4, ((void *)*code_end) - (address+4));
	*code_end -= 2;
}

void rtc_optimise(uint16_t *buffer, uint16_t **code_end) {
	// PUSH                                 1001 001d dddd 1111, with d=source register
	// POP                                  1001 000d dddd 1111
	bool found;

	do {
		found = false;
		for (uint16_t *p = buffer; p < *code_end-1; p++) {
			uint16_t inst1 = *(p);
			uint16_t inst2 = *(p+1);
			if (IS_PUSH(inst1)
					&& IS_POP(inst2)
					&& GET_1REG_OPERAND(inst1) == GET_1REG_OPERAND(inst2)) {
				// PUSH rX, POP rX -> remove
				rtc_optimise_drop_2_instructions(p, code_end);
				found = true;
				break;
			}
		}
	} while (found);
}