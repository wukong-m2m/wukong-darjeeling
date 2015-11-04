#ifndef WKPF_WUCLASSESH
#define WKPF_WUCLASSESH

#include "heap.h"
#include "types.h"

#define WKPF_IS_NATIVE_WUCLASS(x)               (x->update != NULL)
#define WKPF_IS_VIRTUAL_WUCLASS(x)              (x->update == NULL)

#define WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE		1

struct wuobject_t;
typedef void (*setup_function_t)(struct wuobject_t *);
typedef void (*update_function_t)(struct wuobject_t *);

typedef struct wuclass_t {
    uint16_t wuclass_id;
    setup_function_t setup;   // Set for native wuclasses, NULL for virtual wuclasses
    update_function_t update; // Set for native wuclasses, NULL for virtual wuclasses
    uint8_t number_of_properties;
    uint8_t private_c_data_size;
    uint8_t flags;
    struct wuclass_t *next;
    uint8_t properties[];
} wuclass_t;

extern void wkpf_register_wuclass(wuclass_t *wuclass);
extern uint8_t wkpf_get_wuclass_by_id(uint16_t wuclass_id, wuclass_t **wuclass);
extern uint8_t wkpf_get_wuclass_by_index(uint8_t index, wuclass_t **wuclass);
extern uint8_t wkpf_get_number_of_wuclasses();

#endif // WKPF_WUCLASSESH
