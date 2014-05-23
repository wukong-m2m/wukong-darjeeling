#include "types.h"
#include "panic.h"
#include "debug.h"
#include "parse_infusion.h"
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

void emit(uint16_t opop, ...) {
	uint16_t instruction = avr_asm(opop);
	DEBUG_LOG(DBG_RTC, "[rtc]      %x %x\n", instruction%0xFF, (instruction>>8)&0xFF);
    wkreprog_write(2, (uint8_t *)&instruction);
    rtc_compiled_code_next_free_byte += 2;
}

void rtc_compile_lib(dj_infusion *infusion) {
	// uses 512bytes on the stack... maybe optimise this later
	native_method_function_t rtc_method_start_addresses[256];

	rtc_compiled_code_next_free_byte = (dj_di_pointer)rtc_compiled_code_buffer;
	wkreprog_open_raw(rtc_compiled_code_next_free_byte);

	uint16_t number_of_methodimpls = dj_di_getListSize(infusion->methodImplementationList);
	for (uint16_t i=0; i<number_of_methodimpls; i++) {
		dj_di_pointer methodimpl = dj_di_getListElement(infusion->methodImplementationList, i);
		if (infusion->native_handlers[i] != NULL)
			continue; // Skip native or already rtc compiled methods

		DEBUG_LOG(DBG_RTC, "[rtc] compiling method %d\n", i);

		// store the starting address for this method;
		rtc_method_start_addresses[i] = (native_method_function_t)rtc_compiled_code_next_free_byte;

		// translate the method
		dj_di_pointer code = dj_di_methodImplementation_getData(methodimpl);
		uint16_t method_length = dj_di_methodImplementation_getLength(methodimpl);
		for (uint16_t pc=0; pc<method_length; pc++) {
			uint8_t opcode = dj_di_getU8(code + pc);
			switch (opcode) {
				case JVM_SLOAD_0:
					emit(OP_LDD, R18, Y, offset_for_intlocal(methodimpl, 0));
					emit(OP_PUSH, R18);
					emit(OP_LDD, R18, Y, offset_for_intlocal(methodimpl, 0)+1);
					emit(OP_PUSH, R18);
					break;
				case JVM_SCONST_2:
					emit(OP_LDI, R18, 2);
					emit(OP_PUSH, R18);
					emit(OP_LDI, R18, 0);
					emit(OP_PUSH, R18);
					break;
				case JVM_SADD:
					emit(OP_POP, R21);
					emit(OP_POP, R20);
					emit(OP_POP, R19);
					emit(OP_POP, R18);
					emit(OP_ADD, R18, R20);
					emit(OP_ADC, R19, R21);
					emit(OP_PUSH, R18);
					emit(OP_PUSH, R19);
					break;
				case JVM_RETURN:
					emit(OP_RET);
					break;
				case JVM_SRETURN:
					emit(OP_POP, R25);
					emit(OP_POP, R24);
					emit(OP_RET);
					break;
				case JVM_IRETURN:
					emit(OP_POP, R25);
					emit(OP_POP, R24);
					emit(OP_POP, R23);
					emit(OP_POP, R22);
					emit(OP_RET);
					break;
				default:
					DEBUG_LOG(DBG_DARJEELING, "Unimplemented Java opcode %d at pc=%d\n", opcode, pc);
					dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
					break;
			}
		}
	}

	wkreprog_close();

	// At this point, the addresses in the rtc_method_start_addresses are 0
	// for the native methods, while the handler table is 0 for the java methods.
	// We need to fill in the addresses in rtc_method_start_addresses in the
	// empty slots in the handler table.
	rtc_update_method_pointers(infusion, rtc_method_start_addresses);

	// Mark the infusion as translated (how?)
}

