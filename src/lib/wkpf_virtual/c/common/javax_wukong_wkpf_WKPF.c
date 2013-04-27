#include "types.h"
#include "panic.h"
#include "debug.h"
#include "array.h"
#include "hooks.h"
#include "execution.h"
#include "heap.h"
#include "djarchive.h"
#include "core.h"
#include "jlib_base.h"

#include "wkpf.h"
#include "wkpf_gc.h"
#include "wkpf_wuclasses.h"
#include "wkpf_wuobjects.h"
#include "wkpf_properties.h"
#include "wkpf_links.h"
#include "wkpf_comm.h"
#include "wkpf_main.h"
#include "wkpf_virtual_wuclasses.h"

uint8_t wkpf_error_code = WKPF_OK;

void javax_wukong_wkpf_WKPF_byte_getErrorCode()
{
	dj_exec_stackPushShort(wkpf_error_code);
}

void javax_wukong_wkpf_WKPF_void_registerWuClass_short_byte__()
{
	dj_int_array *byteArrayProperties = REF_TO_VOIDP(dj_exec_stackPopRef());
	// check null
	if (byteArrayProperties==nullref){
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	}
	uint16_t wuclass_id = (uint16_t)dj_exec_stackPopShort();
	DEBUG_LOG(DBG_WKPF, "WKPF: Registering virtual wuclass with id %x\n", wuclass_id);
	wkpf_error_code = wkpf_register_virtual_wuclass(wuclass_id, NULL, byteArrayProperties->array.length, (uint8_t *)byteArrayProperties->data.bytes);
}

void javax_wukong_wkpf_WKPF_void_createWuObject_short_byte_javax_wukong_wkpf_VirtualWuObject()
{
	dj_object *java_instance_reference = REF_TO_VOIDP(dj_exec_stackPopRef());
	uint8_t port_number = (uint8_t)dj_exec_stackPopShort();
	uint16_t wuclass_id = (uint16_t)dj_exec_stackPopShort();
	DEBUG_LOG(DBG_WKPF, "WKPF: Creating wuobject for virtual wuclass with id %x at port %x (ref: %p)\n", wuclass_id, port_number, java_instance_reference);
	wkpf_error_code = wkpf_create_wuobject(wuclass_id, port_number, java_instance_reference);
}

void javax_wukong_wkpf_WKPF_void_destroyWuObject_byte()
{
	uint8_t port_number = (uint8_t)dj_exec_stackPopShort();
	DEBUG_LOG(DBG_WKPF, "WKPF: Removing wuobject at port %x\n", port_number);
	wkpf_error_code = wkpf_remove_wuobject(port_number);
}

void javax_wukong_wkpf_WKPF_short_getPropertyShort_javax_wukong_wkpf_VirtualWuObject_byte()
{
	uint8_t property_number = (uint8_t)dj_exec_stackPopShort();
	dj_object *java_instance_reference = REF_TO_VOIDP(dj_exec_stackPopRef());
	wuobject_t *wuobject;
	wkpf_error_code = wkpf_get_wuobject_by_java_instance_reference(java_instance_reference, &wuobject);
	if (wkpf_error_code == WKPF_OK) {
		int16_t value;
		wkpf_error_code = wkpf_internal_read_property_int16(wuobject, property_number, &value);
		dj_exec_stackPushShort(value);
	}
}

void javax_wukong_wkpf_WKPF_void_setPropertyShort_javax_wukong_wkpf_VirtualWuObject_byte_short() {
	int16_t value = (int16_t)dj_exec_stackPopShort();
	uint8_t property_number = (uint8_t)dj_exec_stackPopShort();
	dj_object *java_instance_reference = REF_TO_VOIDP(dj_exec_stackPopRef());
	wuobject_t *wuobject;
	wkpf_error_code = wkpf_get_wuobject_by_java_instance_reference(java_instance_reference, &wuobject);
	if (wkpf_error_code == WKPF_OK) {
		wkpf_error_code = wkpf_internal_write_property_int16(wuobject, property_number, value);
	}
}

void javax_wukong_wkpf_WKPF_boolean_getPropertyBoolean_javax_wukong_wkpf_VirtualWuObject_byte() {
	uint8_t property_number = (uint8_t)dj_exec_stackPopShort();
	dj_object *java_instance_reference = REF_TO_VOIDP(dj_exec_stackPopRef());
	wuobject_t *wuobject;
	wkpf_error_code = wkpf_get_wuobject_by_java_instance_reference(java_instance_reference, &wuobject);
	if (wkpf_error_code == WKPF_OK) {
		bool value;
		wkpf_error_code = wkpf_internal_read_property_boolean(wuobject, property_number, &value);
		dj_exec_stackPushShort(value);
	}
}
void javax_wukong_wkpf_WKPF_void_setPropertyBoolean_javax_wukong_wkpf_VirtualWuObject_byte_boolean() {
	bool value = (int16_t)dj_exec_stackPopShort();
	uint8_t property_number = (uint8_t)dj_exec_stackPopShort();
	dj_object *java_instance_reference = REF_TO_VOIDP(dj_exec_stackPopRef());
	wuobject_t *wuobject;
	wkpf_error_code = wkpf_get_wuobject_by_java_instance_reference(java_instance_reference, &wuobject);
	if (wkpf_error_code == WKPF_OK) {
		wkpf_error_code = wkpf_internal_write_property_boolean(wuobject, property_number, value);
	}
}

void javax_wukong_wkpf_WKPF_void_appInitLinkTableAndComponentMap() {
	dj_vm *vm = dj_exec_getVM();
	wkpf_initLinkTableAndComponentMap(vm->di_app_infusion_archive_data);
}

void javax_wukong_wkpf_WKPF_void_appInitLocalObjectAndInitValues() {
	dj_vm *vm = dj_exec_getVM();
	wkpf_initLocalObjectAndInitValues(vm->di_app_infusion_archive_data);
}

void javax_wukong_wkpf_WKPF_javax_wukong_wkpf_VirtualWuObject_select() {
	// Will call update() for native profiles directly,
	// and return only true for virtual profiles requiring an update.
	wuobject_t *wuobject = wkpf_mainloop();
	dj_exec_stackPushRef(VOIDP_TO_REF(wuobject->java_instance_reference));
	DEBUG_LOG(DBG_WKPF, "WKPF: WKPF.select returning wuclass at port %x.\n", wuobject->port_number);
	return;
}

void javax_wukong_wkpf_WKPF_byte_getPortNumberForComponent_short() {
	uint16_t component_id = (uint16_t)dj_exec_stackPopShort();
	wkcomm_address_t node_id;
	uint8_t port_number;
	wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
	dj_exec_stackPushShort(port_number);
}

void javax_wukong_wkpf_WKPF_boolean_isLocalComponent_short() {
	uint16_t component_id = (int16_t)dj_exec_stackPopShort();
	wkcomm_address_t node_id;
	uint8_t port_number;
	wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
	dj_exec_stackPushShort(wkpf_error_code == WKPF_OK && node_id == wkcomm_get_node_id());
}

void javax_wukong_wkpf_WKPF_short_getMyNodeId() {
	dj_exec_stackPushShort(wkcomm_get_node_id());
}
