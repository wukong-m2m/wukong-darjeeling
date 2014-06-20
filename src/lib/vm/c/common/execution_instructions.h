#include "types.h"

ref_t DO_LDS(dj_local_id localStringId);
void DO_INVOKEVIRTUAL(dj_local_id dj_local_id, uint8_t nr_ref_args);
ref_t DO_NEW(dj_local_id dj_local_id);
ref_t DO_ANEWARRAY(dj_local_id localId, uint16_t size);

