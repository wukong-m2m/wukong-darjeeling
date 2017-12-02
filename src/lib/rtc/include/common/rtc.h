#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "config.h"

#define RTC_START_OF_COMPILED_CODE_SPACE (GET_FAR_ADDRESS(rtc_start_of_compiled_code_marker))
#define RTC_END_OF_COMPILED_CODE_SPACE ((uint32_t)122880)

#define RTC_CODEBUFFER_SIZE 96

extern uint16_t rtc_start_of_next_method;
#define RTC_SET_START_OF_NEXT_METHOD(addr) do { rtc_start_of_next_method = (uint16_t)(addr/2); } while(0)
#define RTC_GET_START_OF_NEXT_METHOD()     ( ((uint32_t)rtc_start_of_next_method)*2 )

void rtc_compile_lib(dj_infusion *);

#ifdef AVRORA
#define AVRORATRACE_DISABLE()    avroraTraceDisable();
#define AVRORATRACE_ENABLE()     avroraTraceEnable();
#else
#define AVRORATRACE_DISABLE()
#define AVRORATRACE_ENABLE()
#endif

uint16_t rtc_offset_for_static_ref(dj_infusion *infusion_ptr, uint8_t variable_index);
uint16_t rtc_offset_for_static_byte(dj_infusion *infusion_ptr, uint8_t variable_index);
uint16_t rtc_offset_for_static_short(dj_infusion *infusion_ptr, uint8_t variable_index);
uint16_t rtc_offset_for_static_int(dj_infusion *infusion_ptr, uint8_t variable_index);
uint16_t rtc_offset_for_static_long(dj_infusion *infusion_ptr, uint8_t variable_index);
uint16_t rtc_offset_for_referenced_infusion(dj_infusion *infusion_ptr, uint8_t ref_inf);
uint8_t offset_for_intlocal_short(dj_di_pointer methodimpl, uint8_t local);
uint8_t offset_for_intlocal_int(dj_di_pointer methodimpl, uint8_t local);
uint8_t offset_for_intlocal_long(dj_di_pointer methodimpl, uint8_t local);
uint8_t offset_for_reflocal(dj_di_pointer methodimpl, uint8_t local);
uint8_t rtc_number_of_operandbytes_for_opcode(uint8_t opcode);
void rtc_current_method_set_uses_reg(uint8_t reg);
void rtc_current_method_set_uses_reg_used_in_lightweight_invoke(uint8_t lightweightmethod_id);
bool rtc_current_method_get_uses_reg(uint8_t reg);
bool rtc_method_get_uses_reg(uint8_t method, uint8_t reg);

void emit_load_local_16bit(uint8_t *regs, uint16_t offset);
void emit_load_local_32bit(uint8_t *regs, uint16_t offset);
#define emit_load_local_ref(regs, offset) emit_load_local_16bit(regs, offset)

void emit_store_local_16bit(uint8_t *regs, uint16_t offset);
void emit_store_local_32bit(uint8_t *regs, uint16_t offset);
#define emit_store_local_ref(regs, offset) emit_store_local_16bit(regs, offset)

#define RTC_STACKCACHE_MAX_IDX             16 // 16 because we only keep track of pairs
#define REG_TO_ARRAY_INDEX(reg)            ((reg)/2)
#define ARRAY_INDEX_TO_REG(idx)            ((idx)*2)

typedef struct _rtc_translationstate {
	dj_infusion *infusion;
	dj_di_pointer methodimpl;
    dj_di_pointer jvm_code_start;
    uint16_t pc;
    uint16_t method_length;
	uint_farptr_t branch_target_table_start_ptr;
    uint16_t branch_target_count; // Keep track of how many branch targets we've seen
    uint16_t codebuffer[RTC_CODEBUFFER_SIZE];
    uint16_t *rtc_codebuffer;
    uint16_t *rtc_codebuffer_position; // A pointer to somewhere within the buffer
    uint8_t current_method_index;
    uint8_t *call_saved_registers_used_per_method; // Used to generate the method prologue/epilogue and by INVOKELIGHT inside of a MARKLOOP loop to determine which pinned registers to save/restore.
    native_method_function_t *method_start_addresses; // Used by INVOKELIGHT to find the address of lightweight methods in the current infusion.
    uint8_t flags;
#ifdef AOT_STRATEGY_SIMPLESTACKCACHE
    uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];
#endif // AOT_STRATEGY_SIMPLESTACKCACHE
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE
    uint16_t current_instruction_pc; // We may need this later, after the instruction already forwarded pc to skip over arguments
    uint16_t current_instruction_valuetag;
    uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_valuetags[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_age[RTC_STACKCACHE_MAX_IDX];
#endif // AOT_STRATEGY_POPPEDSTACKCACHE
#ifdef AOT_STRATEGY_MARKLOOP
    uint16_t current_instruction_pc; // We may need this later, after the instruction already forwarded pc to skip over arguments
    uint16_t current_instruction_valuetag;
    uint8_t current_instruction_opcode;
    uint8_t current_instruction_opcodetype;
    uint16_t pinned_reg_needs_load;
    uint16_t pinned_reg_needs_store;
    uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_valuetags[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_age[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_pinned;
    bool may_use_RZ;
#endif // AOT_STRATEGY_MARKLOOP
#ifdef AOT_SAFETY_CHECKS
    uint8_t jvm_operand_byte0;
    uint8_t jvm_operand_byte1;
    uint8_t jvm_operand_byte2;
    uint8_t jvm_operand_byte3;
    uint8_t jvm_operand_byte4;
    uint8_t pre_instruction_int_stack;
    uint8_t pre_instruction_ref_stack;
    uint8_t post_instruction_int_stack;
    uint8_t post_instruction_ref_stack;
    uint8_t current_opcode;
#endif //AOT_SAFETY_CHECKS
} rtc_translationstate;

#ifdef RTC_TS_HARDCODED_AT_END_OF_HEAP
// Hardcode rtc_ts to be the end of the heap. This will reduce code size by about 1KB because the address
// is known at compile time so we don't need to follow the pointer.
extern unsigned char mem[HEAPSIZE];
#define rtc_ts ((rtc_translationstate *) (((void *)mem) + HEAPSIZE - sizeof(rtc_translationstate)))
#else
// Store a global pointer to the translation state. This will point to a big struct on the heap that
// will contain all the state necessary for AOT translation.
extern rtc_translationstate *rtc_ts;
#endif

#endif // RTC_H
