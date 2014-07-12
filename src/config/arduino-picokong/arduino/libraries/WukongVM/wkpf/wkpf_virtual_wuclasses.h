#ifndef WKPF_VIRTUAL_WUCLASSESH
#define WKPF_VIRTUAL_WUCLASSESH

#include "types.h"

extern uint8_t wkpf_register_virtual_wuclass(uint16_t wuclass_id, update_function_t update, uint8_t number_of_properties, uint8_t properties[]);

#endif // WKPF_VIRTUAL_WUCLASSESH
