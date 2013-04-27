#include "types.h"
#include "debug.h"
#include "djtimer.h"
#include "wkpf.h"
#include "wkpf_properties.h"

void wkpf_update_status_after_property_write(wuobject_t *wuobject, wuobject_property_t *property, bool external_access) {
	// Propagate this property:
	property->status |= PROPERTY_STATUS_NEEDS_PUSH;
	// And remove any flags that indicate we were waiting for an initial value:
	property->status &= ~PROPERTY_STATUS_NEEDS_PULL;
	property->status &= ~PROPERTY_STATUS_NEEDS_PULL_WAITING;
	if (external_access) // Only call update() when someone else writes to the property, not for internal writes (==writes that are already coming from update())
		wkpf_set_need_to_call_update_for_wuobject(wuobject);
}

// Verifies the property exists for this wuobject, and the data and r/w access match
uint8_t wkpf_verify_property_access(wuobject_t *wuobject, uint8_t property_number, uint8_t access, bool external_access, uint8_t type) {
	if (wuobject->wuclass->number_of_properties <= property_number)
		return WKPF_ERR_PROPERTY_NOT_FOUND;
	uint8_t property = wuobject->wuclass->properties[property_number];
	if (external_access) {
		if (access == WKPF_PROPERTY_ACCESS_READONLY && WKPF_IS_WRITEONLY_PROPERTY(property))
			return WKPF_ERR_WRITE_ONLY;
		if (access == WKPF_PROPERTY_ACCESS_WRITEONLY && WKPF_IS_READONLY_PROPERTY(property))
			return WKPF_ERR_READ_ONLY;
	}
	if (type != WKPF_GET_PROPERTY_DATATYPE(property))
		return WKPF_ERR_WRONG_DATATYPE;
	return WKPF_OK;
}


uint8_t wkpf_read_property_int16(wuobject_t *wuobject, uint8_t property_number, bool external_access, int16_t *value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_SHORT);
	if (retval != WKPF_OK)
		return retval;
	int16_t *ptr = (int16_t *)wkpf_get_property(wuobject, property_number)->value;
	*value = *ptr;
	return WKPF_OK;
}
uint8_t wkpf_write_property_int16(wuobject_t *wuobject, uint8_t property_number, bool external_access, int16_t value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_SHORT);
	if (retval != WKPF_OK)
		return retval;
	wuobject_property_t *property = wkpf_get_property(wuobject, property_number);
	int16_t *ptr = (int16_t *)property->value;
	if (external_access || *ptr!=value) {
		*ptr = value;
		wkpf_update_status_after_property_write(wuobject, property, external_access);
	}
	return WKPF_OK;
}


uint8_t wkpf_read_property_boolean(wuobject_t *wuobject, uint8_t property_number, bool external_access, bool *value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);
	if (retval != WKPF_OK)
		return retval;
	bool *ptr = (bool *)wkpf_get_property(wuobject, property_number)->value;
	*value = *ptr;
	return WKPF_OK;
}
uint8_t wkpf_write_property_boolean(wuobject_t *wuobject, uint8_t property_number, bool external_access, bool value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);
	if (retval != WKPF_OK)
		return retval;
	wuobject_property_t *property = wkpf_get_property(wuobject, property_number);
	bool *ptr = (bool *)property->value;
	if (external_access || *ptr!=value) {
		*ptr = value;
		wkpf_update_status_after_property_write(wuobject, property, external_access);
	}
	return WKPF_OK;
}


uint8_t wkpf_read_property_refresh_rate(wuobject_t *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t *value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
	if (retval != WKPF_OK)
		return retval;
	wkpf_refresh_rate_t *ptr = (wkpf_refresh_rate_t *)wkpf_get_property(wuobject, property_number)->value;
	*value = *ptr;
	return WKPF_OK;
}
uint8_t wkpf_write_property_refresh_rate(wuobject_t *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t value) {
	uint8_t retval = wkpf_verify_property_access(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
	if (retval != WKPF_OK)
		return retval;
	wuobject_property_t *property = wkpf_get_property(wuobject, property_number);
	wkpf_refresh_rate_t *ptr = (wkpf_refresh_rate_t *)property->value;
	if (external_access || *ptr!=value) {
		*ptr = value;
	    wkpf_schedule_next_update_for_wuobject(wuobject);
		wkpf_update_status_after_property_write(wuobject, property, external_access);
	}
	return WKPF_OK;
}

uint8_t wkpf_get_property_status(wuobject_t *wuobject, uint8_t property_number, uint8_t *status) {
	wuobject_property_t *property = wkpf_get_property(wuobject, property_number);
	if (property) {
		DEBUG_LOG(DBG_WKPF, "WKPF: wkpf_get_property_status: (port 0x%x, property %d): 0x%x\n", wuobject->port_number, property_number, property->status);
		*status = property->status;
		return WKPF_OK;
	}
	return WKPF_ERR_PROPERTY_NOT_FOUND;
}

uint8_t wkpf_property_needs_initialisation_push(wuobject_t *wuobject, uint8_t property_number) {
	if (wuobject->wuclass->number_of_properties <= property_number)
		return WKPF_ERR_PROPERTY_NOT_FOUND;
	wuobject_property_t *property = wkpf_get_property(wuobject, property_number);
	if (property) {
			if (property->status & PROPERTY_STATUS_NEEDS_PULL || property->status & PROPERTY_STATUS_NEEDS_PULL_WAITING) {
				// A property in another WuObject needs this property as it's initial value,
				// but this property is also waiting for its initial value itself (a chain of unitialised properties)
				// So we only mark that the next push needs to be forced, and wait for the arrival of this
				// property's initial value before propagating.
				property->status |= PROPERTY_STATUS_FORCE_NEXT_PUSH;
			} else {
				// Otherwise (the property's value is already available), immediately schedule it to be propagated
				property->status = (PROPERTY_STATUS_NEEDS_PUSH | PROPERTY_STATUS_FORCE_NEXT_PUSH);
			}
			DEBUG_LOG(DBG_WKPF, "WKPF: wkpf_property_needs_initialisation_push: (port 0x%x, property %d): status %d\n", wuobject->port_number, property_number, property->status);
			return WKPF_OK;
	}
	return WKPF_ERR_SHOULDNT_HAPPEN;
}

bool inline wkpf_property_status_is_dirty(uint8_t status) {
	if (!((status & PROPERTY_STATUS_NEEDS_PUSH) || (status & PROPERTY_STATUS_NEEDS_PULL)))
		return false; // Doesn't need push or pull so skip.
	if ((status & 0x0C) == 0)
		return true; // Didn't fail, or only once (because bits 2,3 are 0): send message
	uint8_t timer_bit = (dj_timer_getTimeMillis() >> (status & PROPERTY_STATUS_FAILURE_COUNT_TIMES2_MASK /* failurecount*2*/)) & 1;
	return (status & 1) == timer_bit;
}


void wkpf_propagating_dirty_property_failed(wuobject_property_t *property) {
	uint8_t failure_count = (property->status & PROPERTY_STATUS_FAILURE_COUNT_TIMES2_MASK) / 2;  
	if (failure_count < 7) // increase if failure count < 7
		failure_count++;
	uint8_t timer_bit_number = failure_count*2; 
	uint8_t timer_bit_value = ((dj_timer_getTimeMillis() >> timer_bit_number) + 1) & 1; // Add one to flip bit 
	uint8_t new_status = (property->status & 0xF0) | (failure_count << 1) | timer_bit_value;
	property->status = new_status;
	DEBUG_LOG(DBG_WKPF, "WKPF: wkpf_propagating_dirty_property_failed!!!!! new status %d\n", new_status);
}

void wkpf_propagating_dirty_property_succeeded(wuobject_property_t *property) {
	if (property->status & PROPERTY_STATUS_NEEDS_PUSH) {
		property->status &= (~PROPERTY_STATUS_NEEDS_PUSH & ~PROPERTY_STATUS_FORCE_NEXT_PUSH & 0xF0);
	} else { // PROPERTY_STATUS_NEEDS_PULL: after sending the request, wait for the source node to send the value
		property->status &= (~PROPERTY_STATUS_NEEDS_PULL & 0xF0);
		property->status |= PROPERTY_STATUS_NEEDS_PULL_WAITING;
	}
}

void wkpf_set_property_status_needs_pull(wuobject_property_t *property) {
	property->status |= PROPERTY_STATUS_NEEDS_PULL;
}
