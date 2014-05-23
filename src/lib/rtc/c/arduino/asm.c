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

uint16_t avr_asm (uint16_t opop, ... ) {
    va_list operands_list;
    va_start(operands_list, opop);

    uint16_t opcode = va_arg(operands_list, uint16_t);
    uint16_t instruction;

    switch (opcode) {
    	case OP_PUSH:
    	case OP_POP:
    		instruction = opcode
    					 + (va_arg(operands_list, uint16_t) << 4);
			break;
    	case OP_LDD:
    		instruction = opcode
    					 + (va_arg(operands_list, uint16_t) << 4) // dest reg
    					 + (va_arg(operands_list, uint16_t) << 3) // Y or Z
    					 + makeLDDoffset(va_arg(operands_list, uint16_t)); // offset from Y or Z
			break;
    	case OP_LDI:
    		instruction = opcode
    					 + ((va_arg(operands_list, uint16_t) - 16)<< 4) // dest reg
    					 + makeLDIconstant(va_arg(operands_list, uint16_t)); // constant
			break;
    	case OP_ADD:
    	case OP_ADC:
    		instruction = opcode
    					 + ((va_arg(operands_list, uint16_t) - 16)<< 4) // dest reg
    					 + makeSourceRegister(va_arg(operands_list, uint16_t)); // source reg
			break;
		case OP_RET:
			instruction = opcode;
    	default:
			DEBUG_LOG(DBG_DARJEELING, "Unimplemented native opcode %d\n", opcode);
			dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
			break;
    }

    va_end ( operands_list );
    return instruction;
}