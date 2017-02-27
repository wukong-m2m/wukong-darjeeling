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
    uint8_t current_method_used_call_saved_reg; // Used to generate the method prologue/epilogue
#if defined(AOT_OPTIMISE_CONSTANT_SHIFTS)
    uint8_t do_CONST_SHIFT_optimisation;
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS
#ifdef AOT_STRATEGY_POPPEDSTACKCACHE
    uint16_t current_instruction_pc; // We may need this later, after the instruction already forwarded pc to skip over arguments
    uint16_t current_instruction_valuetag;
#endif // AOT_STRATEGY_POPPEDSTACKCACHE
#ifdef AOT_STRATEGY_MARKLOOP
    uint16_t current_instruction_pc; // We may need this later, after the instruction already forwarded pc to skip over arguments
    uint16_t current_instruction_valuetag;
    uint8_t current_instruction_opcode;
    uint8_t current_instruction_opcodetype;
    uint16_t pinned_reg_needs_store;
    uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_valuetags[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_age[RTC_STACKCACHE_MAX_IDX];
    uint16_t rtc_stackcache_pinned;
    bool may_use_RZ;
#endif // AOT_STRATEGY_MARKLOOP
} rtc_translationstate;

// Store a global pointer to the translation state. This will point to a big struct on the heap that
// will contain all the state necessary for AOT translation.
extern rtc_translationstate *rtc_ts;

#endif // RTC_H
