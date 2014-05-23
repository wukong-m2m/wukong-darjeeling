#include <stdint.h>
#include <stdarg.h>
#include "debug.h"
#include "panic.h"
#include "wkreprog.h"
#include "asm.h"

uint16_t makeLDDoffset(uint16_t offset) {
	// 6 bit offset q has to be inserted in the opcode like this:
	// 00q0 qq00 0000 0qqq
	return     (offset & 0x07)
			+ ((offset & 0x18) << 7)
			+ ((offset & 0x20) << 8);
}

uint16_t makeLDIconstant(uint16_t constant) {
	// 0000 KKKK 0000 KKKK
	return     (constant & 0x0F)
			+ ((constant & 0xF0) << 8);
}

uint16_t makeSourceRegister(uint16_t src_register) {
	// 0000 00r0 0000 rrrr, with d=dest register, r=source register
	return     (src_register & 0x0F)
			+ ((src_register & 0x10) << 5);
}

uint16_t avr_asm (uint16_t opcode, uint16_t op1, uint16_t op2, uint16_t op3) {
    uint16_t instruction = 0;
    DEBUG_LOG(DBG_RTC, "[rtc] asm %x %x %x %x\n", opcode, op1, op2, op3);

    switch (opcode) {
    	case OP_PUSH:
    	case OP_POP:
        case OP_LDXINC:
    		instruction = opcode
    					 + (op1 << 4);
			break;
    	case OP_LDD:
    		instruction = opcode
    					 + (op1 << 4) // dest reg
    					 + (op2 << 3) // Y or Z
    					 + makeLDDoffset(op3); // offset from Y or Z
			break;
    	case OP_LDI:
    		instruction = opcode
    					 + ((op1 - 16) << 4) // dest reg
    					 + makeLDIconstant(op2); // constant
			break;
    	case OP_ADD:
    	case OP_ADC:
    		instruction = opcode
    					 + (op1 << 4) // dest reg
    					 + makeSourceRegister(op2); // source reg
			break;
		case OP_RET:
			instruction = opcode;
            break;
    	default:
			DEBUG_LOG(DBG_DARJEELING, "Unimplemented native opcode %d\n", opcode);
			dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
			break;
    }

    return instruction;
}