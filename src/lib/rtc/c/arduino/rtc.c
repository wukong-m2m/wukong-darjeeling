#include "types.h"
#include "panic.h"
#include "debug.h"
#include "parse_infusion.h"
#include "infusion.h"
#include "wkreprog.h"
#include "asm.h"
#include "opcodes.h"
#include "rtc.h"
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stddef.h>

const unsigned char PROGMEM __attribute__ ((aligned (SPM_PAGESIZE))) rtc_compiled_code_buffer[RTC_COMPILED_CODE_BUFFER_SIZE] = {};

uint8_t offset_for_intlocal(dj_di_pointer methodimpl, uint8_t local) {
	return (dj_di_methodImplementation_getReferenceLocalVariableCount(methodimpl) * sizeof(ref_t)) +
		   (local * sizeof(int16_t));
}

uint8_t offset_for_reflocal(dj_di_pointer methodimpl, uint8_t local) {
	return (local * sizeof(ref_t));
}

dj_di_pointer rtc_compiled_code_next_free_byte;

void rtc_update_method_pointers(dj_infusion *infusion, native_method_function_t *rtc_method_start_addresses) {
	wkreprog_open_raw((uint16_t)infusion->native_handlers);

	uint16_t number_of_methodimpls = dj_di_getListSize(infusion->methodImplementationList);
	for (uint16_t i=0; i<number_of_methodimpls; i++) {
		dj_di_pointer methodimpl = dj_di_getListElement(infusion->methodImplementationList, i);
		if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_NATIVE) {
			// Copy existing pointer
			wkreprog_write(sizeof(native_method_function_t), (uint8_t *)&(infusion->native_handlers[i]));
		} else {
			// Fill in address of RTC compiled method
			wkreprog_write(sizeof(native_method_function_t), (uint8_t *)&(rtc_method_start_addresses[i]));
		}
	}

	wkreprog_close();
}

#define emit0(opcode)			emit3(opcode, 0, 0, 0)
#define emit1(opcode, op1)		emit3(opcode, op1, 0, 0)
#define emit2(opcode, op1, op2)	emit3(opcode, op1, op2, 0)
void emit3(uint16_t opcode, uint16_t op1, uint16_t op2, uint16_t op3) {
	uint16_t instruction = avr_asm(opcode, op1, op2, op3);
	uint8_t *instructiondata = (uint8_t *)&instruction;
	DEBUG_LOG(DBG_RTC, "[rtc]    %x  (%x %x)\n", instruction, instructiondata[0], instructiondata[1]);
    wkreprog_write(2, (uint8_t *)&instruction);
    rtc_compiled_code_next_free_byte += 2;
}

uint16_t rtc_frame_locals_start;

void rtc_compile_lib(dj_infusion *infusion) {
	// uses 512bytes on the stack... maybe optimise this later
	native_method_function_t rtc_method_start_addresses[256];

	rtc_compiled_code_next_free_byte = (dj_di_pointer)rtc_compiled_code_buffer;
	wkreprog_open_raw(rtc_compiled_code_next_free_byte);

	uint16_t number_of_methodimpls = dj_di_getListSize(infusion->methodImplementationList);
	DEBUG_LOG(DBG_RTC, "[rtc] infusion contains %d methods\n", number_of_methodimpls);	
	for (uint16_t i=0; i<number_of_methodimpls; i++) {
		dj_di_pointer methodimpl = dj_infusion_getMethodImplementation(infusion, i);
		if (infusion->native_handlers[i] != NULL)
			continue; // Skip native or already rtc compiled methods

		// TMPRTC
		if (i!=1)
			continue;
		
		DEBUG_LOG(DBG_RTC, "[rtc] compiling method %d\n", i);

		// store the starting address for this method;
		rtc_method_start_addresses[i] = (native_method_function_t)rtc_compiled_code_next_free_byte;

		// prologue (is this the right way?)
		emit1(OP_PUSH, R28); // Push Y
		emit1(OP_PUSH, R29);
		emit2(OP_LDI, R26, ((uint16_t)&rtc_frame_locals_start)&0xFF); // Load address OF rtc_frame_locals_start in X
		emit2(OP_LDI, R27, (((uint16_t)&rtc_frame_locals_start)>>8)&0xFF);
		emit1(OP_LDXINC, R28); // Load the address STORED IN rtc_frame_locals_start in Y
		emit1(OP_LDXINC, R29);

		// translate the method
		dj_di_pointer code = dj_di_methodImplementation_getData(methodimpl);
		uint16_t method_length = dj_di_methodImplementation_getLength(methodimpl);
		DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", method_length);
		for (uint16_t pc=0; pc<method_length; pc++) {
			uint8_t opcode = dj_di_getU8(code + pc);
			DEBUG_LOG(DBG_RTC, "[rtc] opcode %d\n", opcode);
			switch (opcode) {
				case JVM_SLOAD_0:
					emit3(OP_LDD, R18, Y, offset_for_intlocal(methodimpl, 0));
					emit1(OP_PUSH, R18);
					emit3(OP_LDD, R18, Y, offset_for_intlocal(methodimpl, 0)+1);
					emit1(OP_PUSH, R18);
					break;
				case JVM_SCONST_2:
					emit2(OP_LDI, R18, 2);
					emit1(OP_PUSH, R18);
					emit2(OP_LDI, R18, 0);
					emit1(OP_PUSH, R18);
					break;
				case JVM_BSPUSH:
					emit2(OP_LDI, R18, dj_di_getU8(code + ++pc));
					emit1(OP_PUSH, R18);
				case JVM_SADD:
					emit1(OP_POP, R21);
					emit1(OP_POP, R20);
					emit1(OP_POP, R19);
					emit1(OP_POP, R18);
					emit2(OP_ADD, R18, R20);
					emit2(OP_ADC, R19, R21);
					emit1(OP_PUSH, R18);
					emit1(OP_PUSH, R19);
					break;
				case JVM_RETURN:
					emit0(OP_RET);
					break;
				case JVM_SRETURN:
					emit1(OP_POP, R25);
					emit1(OP_POP, R24);
					emit0(OP_RET);
					break;
				case JVM_IRETURN:
					emit1(OP_POP, R25);
					emit1(OP_POP, R24);
					emit1(OP_POP, R23);
					emit1(OP_POP, R22);
					emit0(OP_RET);
					break;
				default:
					DEBUG_LOG(DBG_DARJEELING, "Unimplemented Java opcode %d at pc=%d\n", opcode, pc);
					dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
					break;
			}
		}

		// epilogue (is this the right way?)
		emit1(OP_POP, R29); // Pop Y
		emit1(OP_POP, R28);
	}

	wkreprog_close();

	// At this point, the addresses in the rtc_method_start_addresses are 0
	// for the native methods, while the handler table is 0 for the java methods.
	// We need to fill in the addresses in rtc_method_start_addresses in the
	// empty slots in the handler table.
	rtc_update_method_pointers(infusion, rtc_method_start_addresses);

	// Mark the infusion as translated (how?)
}

