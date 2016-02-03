#ifndef RTC_BRANCHES_H
#define RTC_BRANCHES_H

#include <stdint.h>

#define rtc_is_branchtag(opcode) (opcode == OPCODE_BREAK)
// We allocate memory for two branch tables. One will contain the original offsets (in words), calculated based on 3 word branches.
// The second will contain optimised addresses, when it becomes clear how many instructions are really needed for each branch.
// Both tables have uint16_t entries.
#define rtc_branch_table_size(methodimpl) (dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl)*2*2)
#define rtc_branch_target_table_1_address(rtc_ts, i) ((rtc_ts->branch_target_table_start_ptr) + 2*i)

void emit_x_branchtag(uint16_t opcode, uint16_t target);
void rtc_patch_branches(dj_di_pointer branch_target_table_start_ptr, dj_di_pointer end_of_method, uint16_t branchTableSize);

#endif // RTC_BRANCHES_H