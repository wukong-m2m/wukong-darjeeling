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
	wkpf_error_code = wkpf_create_wuobject(wuclass_id, port_number, java_instance_reference, false);
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

void javax_wukong_wkpf_WKPF_void_appLoadInitLinkTableAndComponentMap() {
	dj_vm *vm = dj_exec_getVM();
	wkpf_initLinkTableAndComponentMap(vm->di_app_infusion_archive_data);
}

void javax_wukong_wkpf_WKPF_void_appInitCreateLocalObjectAndInitValues() {
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

void javax_wukong_wkpf_WKPF_boolean_updateMapping_short_short_byte_short_byte () {
	uint8_t new_port_number = (uint8_t)dj_exec_stackPopShort();
	uint16_t new_node_id = dj_exec_stackPopShort();
	uint8_t orig_port_number = (uint8_t)dj_exec_stackPopShort();
	uint16_t orig_node_id = dj_exec_stackPopShort();
	uint16_t component_id = dj_exec_stackPopShort();
	uint8_t rt_val = wkpf_update_map_in_flash(component_id, orig_node_id, orig_port_number, new_node_id, new_port_number);
	bool value = (bool)rt_val;
	dj_exec_stackPushShort(value);
}

void javax_wukong_wkpf_WKPF_boolean_updateLinking_short_short_byte_short_byte_short_byte_short_byte () {
	uint8_t new_dest_property_id = (uint8_t)dj_exec_stackPopShort();
	uint16_t new_dest_component_id = dj_exec_stackPopShort();
	uint8_t new_src_property_id = (uint8_t)dj_exec_stackPopShort();
	uint16_t new_src_component_id = dj_exec_stackPopShort();
	uint8_t orig_dest_property_id = (uint8_t)dj_exec_stackPopShort();
	uint16_t orig_dest_component_id = dj_exec_stackPopShort();
	uint8_t orig_src_property_id = (uint8_t)dj_exec_stackPopShort();
	uint16_t orig_src_component_id = dj_exec_stackPopShort();
	uint16_t setter_component_id = dj_exec_stackPopShort();
	
	DEBUG_LOG(true, "set msg orig_src_component_id:%u:%u, new_src_component_id:%u:%u, dest:%u:%u == %u:%u\n", orig_src_component_id, 
            orig_src_property_id, new_src_component_id, new_src_property_id,orig_dest_component_id,orig_dest_property_id, new_dest_component_id,new_dest_property_id);
	//identify node id and port through mapping table, component_id correspond to the seq no. of component in maptable
	wkcomm_address_t orig_src_node_id, new_src_node_id;
	uint8_t orig_src_port, new_src_port;
	//self_node_id = wkcomm_get_node_id();
	wkpf_get_node_and_port_for_component(orig_src_component_id, &orig_src_node_id, &orig_src_port);
	wkpf_get_node_and_port_for_component(new_src_component_id, &new_src_node_id, &new_src_port);
	DEBUG_LOG(true, "set msg orig_src_node_id:%d, new_src_node_id:%d\n", orig_src_node_id, new_src_node_id);
	//set_token spread out with the message of cancelling original link
	wkpf_set_token (setter_component_id, setter_component_id, orig_src_node_id);

	wkpf_send_set_linktable(orig_src_node_id, setter_component_id, orig_src_component_id, orig_src_component_id, orig_src_property_id, 
	                                orig_dest_component_id, orig_dest_property_id, new_src_component_id, 
	                                new_src_property_id, new_dest_component_id, new_dest_property_id);
	//release_token spread out with the message of adding new link
	wkpf_release_token (setter_component_id);
	wkpf_send_set_linktable(new_src_node_id, setter_component_id, new_src_component_id, orig_src_component_id, orig_src_property_id, 
	                                orig_dest_component_id, orig_dest_property_id, new_src_component_id, 
	                                new_src_property_id, new_dest_component_id, new_dest_property_id);
	wkpf_send_set_linktable_no_token(orig_src_node_id, setter_component_id, new_src_component_id, orig_src_component_id, orig_src_property_id, 
	                                orig_dest_component_id, orig_dest_property_id, new_src_component_id, 
	                                new_src_property_id, new_dest_component_id, new_dest_property_id);
	DEBUG_LOG(true, "about to update_link", orig_src_node_id, new_src_node_id);
	uint8_t rt_val = wkpf_update_link(orig_src_component_id, orig_src_property_id, orig_dest_component_id, orig_dest_property_id, 
	                                  new_src_component_id, new_src_property_id, new_dest_component_id, new_dest_property_id);


	bool value = ! ((bool)rt_val);		//rt_val == 0 (WKPF_OK)
	dj_exec_stackPushShort(value);

}