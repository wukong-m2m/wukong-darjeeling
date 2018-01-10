#include "execution.h"
#include "array.h"
#include "jlib_bm_motetrack.h"
#include "RefSignature.h"
#include "RefSignatureDB.h"

void javax_rtcbench_NewSignature_void_initNewSignature_javax_rtcbench_NewSignature_short() {
	avroraRTCRuntimeBeep(16);
	int16_t index = dj_exec_stackPopShort();
	BM_MOTETRACK_STRUCT_javax_rtcbench_NewSignature * dest_signature = (BM_MOTETRACK_STRUCT_javax_rtcbench_NewSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());

	RefSignature refSig;
	// avroraRTCRuntimeBeep(17);
	RefSignatureDB_get(&refSig, index);
	// avroraRTCRuntimeBeep(18);

	dest_signature->x = refSig.location.x;
	dest_signature->y = refSig.location.y;
	dest_signature->z = refSig.location.z;
	dest_signature->sig_id = refSig.sig.id;

	// dj_int_array* signals_source_ID = REF_TO_VOIDP(refSig.signals_source_ID);
	// dj_int_array* signals_rssi1 = REF_TO_VOIDP(refSig.signals_rssi1);
	// dj_int_array* signals_rssi2 = REF_TO_VOIDP(refSig.signals_rssi2);

	int16_t* dest_signals_source_ID = ((dj_int_array *)REF_TO_VOIDP(dest_signature->signals_source_ID))->data.shorts;
	int8_t* dest_signals_rssi1      = ((dj_int_array *)REF_TO_VOIDP(dest_signature->signals_rssi1))->data.bytes;
	int8_t* dest_signals_rssi2      = ((dj_int_array *)REF_TO_VOIDP(dest_signature->signals_rssi2))->data.bytes;

	// avroraPrintDJHeap();
	// avroraPrintPtr(dest_signature);
	// avroraPrintPtr(dest_signals_source_ID);
	// avroraPrintPtr(dest_signals_rssi1);
	// avroraPrintPtr(dest_signals_rssi2);

	for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
		dest_signals_source_ID[i] = refSig.sig.rfSignals[i].sourceID;
		dest_signals_rssi1[i] = refSig.sig.rfSignals[i].rssi[0][0];
		dest_signals_rssi2[i] = refSig.sig.rfSignals[i].rssi[1][0];
	}
	avroraRTCRuntimeBeep(19);
}
