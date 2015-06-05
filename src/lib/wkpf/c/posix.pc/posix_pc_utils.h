#ifndef POSIX_PC_UTILSH
#define POSIX_PC_UTILSH

#include "../common/wkpf_wuclasses.h"
#include "../common/wkpf_wuobjects.h"

extern void posix_property_put(wuobject_t *wuobject, char *property, int value);
extern int posix_property_get(wuobject_t *wuobject, char *property);

#endif // POSIX_PC_UTILSH