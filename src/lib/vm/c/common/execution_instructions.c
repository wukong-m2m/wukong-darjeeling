#include "types.h"
#include "debug.h"
#include "execution.h"
#include "jstring.h"
#include "array.h"

// generated at infusion time
#include "jlib_base.h"

#include "rtc_safetychecks_vm_part.h"

ref_t DO_LDS(dj_local_id localStringId) {
	// resolve the string id
	dj_global_id globalStringId = dj_global_id_resolve(dj_exec_getCurrentInfusion(), localStringId);

	dj_object *string = dj_jstring_createFromGlobalId(dj_exec_getVM(), globalStringId);

	if (string==NULL) {
		dj_exec_createAndThrow(OUTOFMEMORY_ERROR);
		return 0;
	}

	return VOIDP_TO_REF(string);
}


void DO_INVOKEVIRTUAL(dj_global_id globalMethodDefId, uint8_t nr_ref_args) {
	// peek the object on the stack
	dj_object *object = REF_TO_VOIDP(dj_exec_stackPeekDeepRef(nr_ref_args));

	// if null, throw exception
	if (object==NULL)
	{
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
		return;
	}

	// check if the object is still valid
	if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
	{
		dj_exec_createAndThrow(CLASSUNLOADED_EXCEPTION);
		return;
	}

	DEBUG_LOG(DBG_DARJEELING, ">>>>> invokevirtual METHOD DEF %p.%d\n", resolvedMethodDefId.infusion, resolvedMethodDefId.entity_id);

	// lookup the virtual method
	dj_global_id methodImplId = dj_global_id_lookupVirtualMethod(globalMethodDefId, object);

// #ifdef AOT_SAFETY_CHECKS
// 	// Safety check
// 	dj_global_id anyMethodImplId = dj_global_id_lookupAnyVirtualMethod(globalMethodDefId);
// 	niels
// #endif


	DEBUG_LOG(DBG_DARJEELING, ">>>>> invokevirtual METHOD IMPL %p.%d\n", methodImplId.infusion, methodImplId.entity_id);

	// check if method not found, and throw an error if this is the case. else, invoke the method
	if (methodImplId.infusion==NULL)
	{
#ifdef AOT_SAFETY_CHECKS
		rtc_safety_abort_with_error(RTC_SAFETYCHECK_NO_IMPL_FOUND_FOR_INVOKEVIRTUAL);
#else // AOT_SAFETY_CHECKS
		DEBUG_LOG(DBG_DARJEELING, "methodImplId.infusion is NULL at INVOKEVIRTUAL %p.%d\n", resolvedMethodDefId.infusion, resolvedMethodDefId.entity_id);

		dj_exec_createAndThrow(VIRTUALMACHINE_ERROR);
#endif // AOT_SAFETY_CHECKS
	} else {
		callMethod(methodImplId, true);
	}
}

ref_t DO_NEW(dj_local_id dj_local_id) {
	dj_di_pointer classDef;
	dj_global_id dj_global_id = dj_global_id_resolve(dj_exec_getCurrentInfusion(), dj_local_id);

	// get class definition
	classDef = dj_global_id_getClassDefinition(dj_global_id);

	dj_object * object = dj_object_create(
			dj_global_id_getRuntimeClassId(dj_global_id),
			dj_di_classDefinition_getNrRefs(classDef),
			dj_di_classDefinition_getOffsetOfFirstReference(classDef)
			);

	// if create returns null, throw out of memory error
	if (object==NULL) {
		dj_exec_createAndThrow(OUTOFMEMORY_ERROR);
		return 0;
	}

	return VOIDP_TO_REF(object);
}

ref_t DO_ANEWARRAY(dj_local_id dj_local_id, uint16_t size) {
	dj_global_id dj_global_id = dj_global_id_resolve(dj_exec_getCurrentInfusion(), dj_local_id);
	uint16_t id = dj_global_id_getRuntimeClassId(dj_global_id);
	dj_ref_array *arr = dj_ref_array_create(id, size);

	if (arr==nullref) {
		dj_exec_createAndThrow(OUTOFMEMORY_ERROR);
		return 0;
	}
	else
		return VOIDP_TO_REF(arr);
}

int16_t DO_INSTANCEOF(dj_local_id localClassId, ref_t ref) {
	dj_object * object = REF_TO_VOIDP(ref);

    DEBUG_ENTER_NEST(DBG_DARJEELING, "INSTANCEOF()");
	// if the reference is null, result should be 0 (FALSE).
    // Else use dj_global_id_testType to dermine
	// if the ref on the stack is of the desired type
	if (ref==nullref)
    {
		return 0;
    } else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
	{
		dj_exec_createAndThrow(CLASSUNLOADED_EXCEPTION);
		return 0;
	}

	else if (dj_global_id_isJavaLangObject(dj_global_id_resolve(dj_exec_getCurrentInfusion(), localClassId)))
    {
        DEBUG_LOG(DBG_DARJEELING, "Ich bin a j.l.Object\n");
        // a   check  against   a  non-null   object   for  instanceof
        // java.lang.Object should always return true
        return 1;
    }
	else
    {
		return dj_global_id_testType(object, localClassId);
    }
    DEBUG_EXIT_NEST(DBG_DARJEELING, "INSTANCEOF()");
}


