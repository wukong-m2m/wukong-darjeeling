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
	DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
	uint16_t native_handlers_address = (uint16_t)infusion->native_handlers;
	wkreprog_open_raw(native_handlers_address);

	uint16_t number_of_methodimpls = dj_di_getListSize(infusion->methodImplementationList);

	for (uint16_t i=0; i<number_of_methodimpls; i++) {
		dj_di_pointer methodimpl = dj_infusion_getMethodImplementation(infusion, i);
		native_method_function_t handler;
		if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_NATIVE) {
			// Copy existing pointer
			const DJ_PROGMEM native_method_function_t *native_handlers = infusion->native_handlers;
			handler = native_handlers[i];
			DEBUG_LOG(DBG_RTC, "[rtc] method %d is native, copying native handler: %p\n", i, handler);
		} else {
			// Fill in address of RTC compiled method
			handler = rtc_method_start_addresses[i];
			DEBUG_LOG(DBG_RTC, "[rtc] method %d is not native, filling in address from rtc buffer: %p\n", i, handler);
		}
		wkreprog_write(2, (uint8_t *)&handler);
	}

	wkreprog_close();
}

void emit(uint16_t instruction) {
	uint8_t *instructiondata = (uint8_t *)&instruction;
	DEBUG_LOG(DBG_RTC, "[rtc]    %x  (%x %x)\n", instruction, instructiondata[0], instructiondata[1]);
    wkreprog_write(2, (uint8_t *)&instruction);
    rtc_compiled_code_next_free_byte += 2;
}

uint16_t rtc_frame_locals_start;

void rtc_compile_lib(dj_infusion *infusion) {
	// uses 512bytes on the stack... maybe optimise this later
	native_method_function_t rtc_method_start_addresses[256];
	for (uint16_t i=0; i<256; i++)
		rtc_method_start_addresses[i] = 0;

	rtc_compiled_code_next_free_byte = (dj_di_pointer)rtc_compiled_code_buffer;
	wkreprog_open_raw(rtc_compiled_code_next_free_byte);

	uint16_t number_of_methodimpls = dj_di_getListSize(infusion->methodImplementationList);
	DEBUG_LOG(DBG_RTC, "[rtc] infusion contains %d methods\n", number_of_methodimpls);

	const DJ_PROGMEM native_method_function_t *handlers = infusion->native_handlers;
	DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", infusion->native_handlers);
	DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %p\n", handlers);
	DEBUG_LOG(DBG_RTC, "[rtc] handler list is at %x\n", (uint16_t)handlers);
	for (uint16_t i=0; i<number_of_methodimpls; i++) {		
		DEBUG_LOG(DBG_RTC, "[rtc] (compile) pointer for method %i %p\n", i, infusion->native_handlers[i]);	
		DEBUG_LOG(DBG_RTC, "[rtc] (compile) pointer for method %i %p\n", i, handlers[i]);
		DEBUG_LOG(DBG_RTC, "[rtc] (compile) pointer for method %i %x\n", i, dj_di_getU16(((uint16_t)handlers)+2*i));	

		dj_di_pointer methodimpl = dj_infusion_getMethodImplementation(infusion, i);
		if (dj_di_methodImplementation_getFlags(methodimpl) & FLAGS_NATIVE) {
			DEBUG_LOG(DBG_RTC, "[rtc] skipping native method %d\n", i);
			continue;
		}

		if (handlers[i] != NULL) {
			DEBUG_LOG(DBG_RTC, "[rtc] should skip already compiled method %d with pointer %p, but won't for now\n", i, handlers[i]);
			// continue; // Skip native or already rtc compiled methods
		}

		// TMPRTC
		if (i!=1) {
			DEBUG_LOG(DBG_RTC, "[rtc] skipping method %i for now\n", i);
			continue;
		}
		
		DEBUG_LOG(DBG_RTC, "[rtc] compiling method %d\n", i);

		// store the starting address for this method;
		// IMPORTANT!!!! the PC in AVR stores WORD addresses, so we need to divide the address
		// of a function by 2 in order to get a function pointer!
		rtc_method_start_addresses[i] = (native_method_function_t)(rtc_compiled_code_next_free_byte/2);

		// prologue (is this the right way?)
		emit( asm_PUSH(R28) ); // Push Y
		emit( asm_PUSH(R29) );
		emit( asm_LDI(R26, ((uint16_t)&rtc_frame_locals_start)&0xFF) ); // Load address OF rtc_frame_locals_start in X
		emit( asm_LDI(R27, (((uint16_t)&rtc_frame_locals_start)>>8)&0xFF) );
		emit( asm_LDXINC(R28) ); // Load the address STORED IN rtc_frame_locals_start in Y
		emit( asm_LDXINC(R29) );

		// translate the method
		dj_di_pointer code = dj_di_methodImplementation_getData(methodimpl);
		uint16_t method_length = dj_di_methodImplementation_getLength(methodimpl);
		DEBUG_LOG(DBG_RTC, "[rtc] method length %d\n", method_length);

		uint8_t jvm_operand_byte0;
		for (uint16_t pc=0; pc<method_length; pc++) {
			uint8_t opcode = dj_di_getU8(code + pc);
			DEBUG_LOG(DBG_RTC, "[rtc] JVM opcode %d\n", opcode);
			switch (opcode) {
				case JVM_SLOAD_0:
					emit( asm_LDD(R18, Y, offset_for_intlocal(methodimpl, 0)) );
					emit( asm_PUSH(R18) );
					emit( asm_LDD(R18, Y, offset_for_intlocal(methodimpl, 0)+1) );
					emit( asm_PUSH(R18) );
					break;
				case JVM_SCONST_2:
					emit( asm_LDI(R18, 2) );
					emit( asm_PUSH(R18) );
					emit( asm_LDI(R18, 0) );
					emit( asm_PUSH(R18) );
					break;
				case JVM_BSPUSH:
					jvm_operand_byte0 = dj_di_getU8(code + ++pc);
					emit( asm_LDI(R18, jvm_operand_byte0) );
					emit( asm_PUSH(R18) );
					emit( asm_LDI(R18, 0) );
					emit( asm_PUSH(R18) );
					break;
				case JVM_SADD:
					emit( asm_POP(R21) );
					emit( asm_POP(R20) );
					emit( asm_POP(R19) );
					emit( asm_POP(R18) );
					emit( asm_ADD(R18, R20) );
					emit( asm_ADC(R19, R21) );
					emit( asm_PUSH(R18) );
					emit( asm_PUSH(R19) );
					break;
				case JVM_RETURN:
					// epilogue (is this the right way?)
					emit( asm_POP(R29) ); // Pop Y
					emit( asm_POP(R28) );
					emit( asm_RET );
					break;
				case JVM_SRETURN:
					emit( asm_POP(R25) );
					emit( asm_POP(R24) );

					// epilogue (is this the right way?)
					emit( asm_POP(R29) ); // Pop Y
					emit( asm_POP(R28) );
					emit( asm_RET );
					break;
				case JVM_IRETURN:
					emit( asm_POP(R25) );
					emit( asm_POP(R24) );
					emit( asm_POP(R23) );
					emit( asm_POP(R22) );

					// epilogue (is this the right way?)
					emit( asm_POP(R29) ); // Pop Y
					emit( asm_POP(R28) );
					emit( asm_RET );
					break;
				default:
					DEBUG_LOG(DBG_RTC, "Unimplemented Java opcode %d at pc=%d\n", opcode, pc);
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

