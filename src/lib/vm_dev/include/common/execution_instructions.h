#include "types.h"

ref_t DO_LDS(dj_local_id localStringId);
#ifdef AOT_SAFETY_CHECKS
void DO_INVOKEVIRTUAL(dj_global_id globalMethodDefId, rtc_safety_method_signature signature_info);
#else
void DO_INVOKEVIRTUAL(dj_global_id globalMethodDefId, uint8_t nr_ref_args);
#endif
ref_t DO_NEW(dj_local_id dj_local_id);
ref_t DO_ANEWARRAY(dj_local_id localId, uint16_t size);
int16_t DO_INSTANCEOF(dj_local_id localClassId, ref_t ref);

