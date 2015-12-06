#ifndef RTC_COMPLEX_INSTRUCTIONS_H
#define RTC_COMPLEX_INSTRUCTIONS_H

#include "debug.h"
#include "types.h"
#include "global_id.h"
#include "execution.h"

void RTC_INVOKEVIRTUAL_OR_INTERFACE(dj_local_id localId, uint8_t nr_ref_args);
void RTC_INVOKESPECIAL(dj_local_id localId);
void RTC_INVOKESTATIC(dj_local_id localId);
ref_t RTC_NEW(dj_local_id localId);
ref_t RTC_LDS(dj_local_id localId);
ref_t RTC_ANEWARRAY(dj_local_id localId, uint16_t size);
int16_t RTC_INSTANCEOF(dj_local_id localClassId, ref_t ref);
void RTC_CHECKCAST(dj_local_id localId, ref_t ref);

#endif // RTC_COMPLEX_INSTRUCTIONS_H