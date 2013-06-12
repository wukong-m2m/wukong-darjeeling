#ifndef WKPF_MAINH
#define WKPF_MAINH

#include "types.h"
#include "wkpf_wuobjects.h"

extern void wkpf_initLinkTableAndComponentMap(dj_di_pointer archive);
extern void wkpf_initLocalObjectAndInitValues(dj_di_pointer archive);
extern wuobject_t *wkpf_mainloop();
extern void wkpf_picokong(dj_di_pointer archive);

#endif // WKPF_MAINH
