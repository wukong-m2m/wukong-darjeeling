#ifndef RTC_BRANCHES_H
#define RTC_BRANCHES_H

#include <stdint.h>

#define rtc_is_branchtag(opcode) (opcode == OPCODE_BREAK)
#define rtc_branch_table_size(methodimpl) (dj_di_methodImplementation_getNumberOfBranchTargets(methodimpl)*SIZEOF_RJMP)
#define rtc_branch_target_table_address(table_start, i) ((table_start) + 2*i)

void emit_x_branchtag(uint16_t opcode, uint16_t target);
void rtc_patch_branches(dj_di_pointer branch_target_table_start_ptr, dj_di_pointer end_of_safe_region, uint16_t branchTableSize, dj_di_pointer end_of_method);

#endif // RTC_BRANCHES_H