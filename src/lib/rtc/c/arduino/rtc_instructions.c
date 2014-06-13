#include "debug.h"
#include "types.h"
#include "global_id.h"
#include "execution.h"
#include "execution_instructions.h"

// TMPRTC
extern int16_t *intStack;
extern ref_t *refStack;


void RTC_INVOKEVIRTUAL(dj_local_id localId, uint8_t nr_ref_args) {
	DEBUG_LOG(DBG_RTC, "RTC_INVOKEVIRTUAL %d %d %d\n", localId.infusion_id, localId.entity_id, nr_ref_args);
	DEBUG_LOG(DBG_RTC, "RTC_INVOKEVIRTUAL %p %p\n", intStack, refStack);

	DO_INVOKEVIRTUAL(localId, nr_ref_args);
}

void RTC_INVOKESTATIC(dj_local_id localId) {
	DEBUG_LOG(DBG_RTC, "RTC_INVOKESTATIC %d %d\n", localId.infusion_id, localId.entity_id);
	DEBUG_LOG(DBG_RTC, "RTC_INVOKESTATIC %p %p\n", intStack, refStack);
	dj_global_id globalId = dj_global_id_resolve(dj_exec_getCurrentInfusion(),  localId);
	// call to callMethod in execution.c
	callMethod(globalId, false);
}
