#include "string.h"

#include "jstring.h"
#include "jlib_base.h"

#include "execution.h"
#include "array.h"

dj_object * dj_jstring_create(dj_vm *vm, int length)
{
	dj_object * jstring;

	// create java String object
	uint8_t runtime_id = dj_vm_getSysLibClassRuntimeId(vm, BASE_CDEF_java_lang_String);
	dj_di_pointer classDef = dj_vm_getRuntimeClassDefinition(vm, runtime_id);
	jstring = dj_object_create(runtime_id, dj_di_classDefinition_getNrRefs(classDef), dj_di_classDefinition_getOffsetOfFirstReference(classDef));

	// throw OutOfMemoryError if dj_object_create returns NULL
	if (jstring == NULL) return NULL;

	// add the string pointer to the safe memory pool to avoid it becoming invalid in case dj_int_array_create triggers a GC
	dj_mem_addSafePointer((void**)&jstring);

	// create charArray
	dj_int_array * charArray = dj_int_array_create(T_CHAR, length);

	// throw OutOfMemoryError
	if (charArray == NULL)
	{
		dj_mem_free(jstring);
		return NULL;
	} else
	{
		BASE_STRUCT_java_lang_String * stringObject = (BASE_STRUCT_java_lang_String*)jstring;

		stringObject->offset = 0;
		stringObject->count = length;
		stringObject->value = VOIDP_TO_REF(charArray);
	}

	// Remove the string object pointer from the safe memory pool.
	dj_mem_removeSafePointer((void**)&jstring);

	return jstring;
}

dj_object * dj_jstring_createFromStr(dj_vm *vm, char * str)
{
	uint16_t i;

	BASE_STRUCT_java_lang_String * jstring = (BASE_STRUCT_java_lang_String*)dj_jstring_create(vm, strlen(str));
	dj_int_array * charArray = REF_TO_VOIDP(jstring->value);

	// Copy ASCII from program space to the array
	for (i = 0; i < strlen(str); i++) charArray->data.bytes[i] = str[i];

	return (dj_object *)jstring;
}

dj_object * dj_jstring_createFromGlobalId(dj_vm *vm, dj_global_id stringId)
{
	uint16_t i;

	// get pointer to the ASCII string in program memory and the string length
	dj_di_pointer stringBytes = dj_di_stringtable_getElementBytes(stringId.infusion->stringTable, stringId.entity_id);
	uint16_t stringLength = dj_di_stringtable_getElementLength(stringId.infusion->stringTable, stringId.entity_id);

	BASE_STRUCT_java_lang_String * jstring = (BASE_STRUCT_java_lang_String*)dj_jstring_create(vm, stringLength);
	dj_int_array * charArray = REF_TO_VOIDP(jstring->value);

	// Copy ASCII from program space to the array
	for (i = 0; i < stringLength; i++)
		charArray->data.bytes[i] = dj_di_getU8(stringBytes++);

	return (dj_object *)jstring;
}
