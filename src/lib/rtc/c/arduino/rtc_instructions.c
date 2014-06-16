#include "debug.h"
#include "types.h"
#include "global_id.h"
#include "execution.h"
#include "execution_instructions.h"

// TMPRTC
extern int16_t *intStack;
extern ref_t *refStack;


void RTC_INVOKEVIRTUAL_OR_INTERFACE(dj_local_id localId, uint8_t nr_ref_args) {
	DEBUG_LOG(DBG_RTC, "RTC_INVOKEVIRTUAL_OR_INTERFACE %d %d %d\n", localId.infusion_id, localId.entity_id, nr_ref_args);
	DEBUG_LOG(DBG_RTC, "RTC_INVOKEVIRTUAL_OR_INTERFACE %p %p\n", intStack, refStack);

	DO_INVOKEVIRTUAL(localId, nr_ref_args);

    // If we called a JVM method, run until it's done.
    while (!dj_exec_currentMethodIsRTCCompiled())
        dj_exec_run(RUNSIZE);
}

void RTC_INVOKESPECIAL(dj_local_id localId) {
	DEBUG_LOG(DBG_RTC, "RTC_INVOKESPECIAL %d %d\n", localId.infusion_id, localId.entity_id);
	DEBUG_LOG(DBG_RTC, "RTC_INVOKESPECIAL %p %p\n", intStack, refStack);
	dj_global_id globalId = dj_global_id_resolve(dj_exec_getCurrentInfusion(),  localId);
	callMethod(globalId, true); // call to callMethod in execution.c

    // If we called a JVM method, run until it's done.
    while (!dj_exec_currentMethodIsRTCCompiled())
        dj_exec_run(RUNSIZE);
}

void RTC_INVOKESTATIC(dj_local_id localId) {
    DEBUG_LOG(DBG_RTC, "RTC_INVOKESTATIC %d %d\n", localId.infusion_id, localId.entity_id);
    DEBUG_LOG(DBG_RTC, "RTC_INVOKESTATIC %p %p\n", intStack, refStack);
    dj_global_id globalId = dj_global_id_resolve(dj_exec_getCurrentInfusion(),  localId);
    callMethod(globalId, false); // call to callMethod in execution.c

    // If we called a JVM method, run until it's done.
    while (!dj_exec_currentMethodIsRTCCompiled())
        dj_exec_run(RUNSIZE);
}


ref_t RTC_NEW(dj_local_id localId) {
    DEBUG_LOG(DBG_RTC, "RTC_NEW %d %d\n", localId.infusion_id, localId.entity_id);
    return DO_NEW(localId);
}

ref_t RTC_LDS(dj_local_id localId) {
    DEBUG_LOG(DBG_RTC, "RTC_LDS %d %d\n", localId.infusion_id, localId.entity_id);
    return DO_LDS(localId);
}
