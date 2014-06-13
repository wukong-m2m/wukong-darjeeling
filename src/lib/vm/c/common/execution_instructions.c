#include "types.h"
#include "debug.h"
#include "execution.h"

// generated at infusion time
#include "jlib_base.h"

void DO_INVOKEVIRTUAL(dj_local_id dj_local_id, uint8_t nr_ref_args) {
	// peek the object on the stack
	dj_object *object = REF_TO_VOIDP(dj_exec_stackPeekDeepRef(nr_ref_args));

	// if null, throw exception
	if (object==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
		return;
	}

	// check if the object is still valid
	if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
	{
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
		return;
	}

	dj_global_id resolvedMethodDefId = dj_global_id_resolve(dj_exec_getCurrentInfusion(), dj_local_id);

	DEBUG_LOG(DBG_DARJEELING, ">>>>> invokevirtual METHOD DEF %p.%d\n", resolvedMethodDefId.infusion, resolvedMethodDefId.entity_id);

	// lookup the virtual method
	dj_global_id methodImplId = dj_global_id_lookupVirtualMethod(resolvedMethodDefId, object);

	DEBUG_LOG(DBG_DARJEELING, ">>>>> invokevirtual METHOD IMPL %p.%d\n", methodImplId.infusion, methodImplId.entity_id);

	// check if method not found, and throw an error if this is the case. else, invoke the method
	if (methodImplId.infusion==NULL)
	{
		DEBUG_LOG(DBG_DARJEELING, "methodImplId.infusion is NULL at INVOKEVIRTUAL %p.%d\n", resolvedMethodDefId.infusion, resolvedMethodDefId.entity_id);

		dj_exec_throwHere(dj_vm_createSysLibObject(dj_exec_getVM(), BASE_CDEF_java_lang_VirtualMachineError));
	} else
	{
		callMethod(methodImplId, true);
	}
}
