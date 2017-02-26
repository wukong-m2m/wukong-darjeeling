#ifndef RTC_BRANCHES_H
#define RTC_BRANCHES_H

#include <stdint.h>

#define rtc_is_branchtag(opcode) (opcode == OPCODE_BREAK)
// We allocate memory for two branch tables. One will contain the original offsets (in words), calculated based on 3 word branches.
// The second will contain optimised addresses, when it becomes clear how many instructions are really needed for each branch.
// Both tables have uint16_t entries.
#define rtc_branch_table_size(methodimpl) ((dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl)+1)*2*2) // +1 is for the extra branch target that will mark the address of the epilogue
#define rtc_branch_target_table_1_address(rtc_ts, i) ((rtc_ts->branch_target_table_start_ptr) + 2*i)

void rtc_mark_branchtarget();
void emit_x_branchtag(uint16_t opcode, uint16_t target);
void rtc_patch_branches(uint_farptr_t branch_target_table_start_ptr, uint_farptr_t end_of_method, uint16_t branchTableSize, uint8_t shift_because_of_smaller_prologue);

#endif // RTC_BRANCHES_H