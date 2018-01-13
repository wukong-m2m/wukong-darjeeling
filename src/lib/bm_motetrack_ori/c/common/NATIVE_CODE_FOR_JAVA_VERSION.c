#include "execution.h"
#include "array.h"
#include "jlib_bm_motetrack.h"
#include "RefSignature.h"
#include "SignatureDB.h"
#include "RefSignatureDB.h"

// void javax_rtcbench_NewSignature_void_newSignature_get_javax_rtcbench_NewSignature_short() {
// 	avroraRTCRuntimeBeep(16);
// 	int16_t index = dj_exec_stackPopShort();
// 	BM_MOTETRACK_STRUCT_javax_rtcbench_NewSignature * dest_newsignature = (BM_MOTETRACK_STRUCT_javax_rtcbench_NewSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());

// 	RefSignature refSig;
// 	RefSignatureDB_get(&refSig, index);

// 	dest_newsignature->x = refSig.location.x;
// 	dest_newsignature->y = refSig.location.y;
// 	dest_newsignature->z = refSig.location.z;
// 	dest_newsignature->sig_id = refSig.sig.id;

// 	// dj_int_array* signals_source_ID = REF_TO_VOIDP(refSig.signals_source_ID);
// 	// dj_int_array* signals_rssi1 = REF_TO_VOIDP(refSig.signals_rssi1);
// 	// dj_int_array* signals_rssi2 = REF_TO_VOIDP(refSig.signals_rssi2);

// 	int16_t* dest_signals_source_ID = ((dj_int_array *)REF_TO_VOIDP(dest_newsignature->signals_source_ID))->data.shorts;
// 	int8_t* dest_signals_rssi1      = ((dj_int_array *)REF_TO_VOIDP(dest_newsignature->signals_rssi1))->data.bytes;
// 	int8_t* dest_signals_rssi2      = ((dj_int_array *)REF_TO_VOIDP(dest_newsignature->signals_rssi2))->data.bytes;

// 	for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
// 		dest_signals_source_ID[i] = refSig.sig.rfSignals[i].sourceID;
// 		dest_signals_rssi1[i] = refSig.sig.rfSignals[i].rssi[0][0];
// 		dest_signals_rssi2[i] = refSig.sig.rfSignals[i].rssi[1][0];
// 	}
// 	avroraRTCRuntimeBeep(19);
// }

void javax_rtcbench_DB_void_refSignature_get_javax_rtcbench_RefSignature_short() {
	int16_t index = dj_exec_stackPopShort();
	BM_MOTETRACK_STRUCT_javax_rtcbench_RefSignature * dest_refsignature = (BM_MOTETRACK_STRUCT_javax_rtcbench_RefSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());

	RefSignature refSig;
	RefSignatureDB_get(&refSig, index);

	BM_MOTETRACK_STRUCT_javax_rtcbench_Point * dest_location = (BM_MOTETRACK_STRUCT_javax_rtcbench_Point *)REF_TO_VOIDP(dest_refsignature->location);
	dest_location->x = refSig.location.x;
	dest_location->y = refSig.location.y;
	dest_location->z = refSig.location.z;

	BM_MOTETRACK_STRUCT_javax_rtcbench_Signature * dest_sig = (BM_MOTETRACK_STRUCT_javax_rtcbench_Signature *)REF_TO_VOIDP(dest_refsignature->sig);
	dest_sig->id = refSig.sig.id;

	ref_t * dest_rfsignals = ((dj_ref_array *)REF_TO_VOIDP(dest_sig->rfSignals))->refs;
	for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
		BM_MOTETRACK_STRUCT_javax_rtcbench_RFSignal * dest_rfsignal = (BM_MOTETRACK_STRUCT_javax_rtcbench_RFSignal *)REF_TO_VOIDP(dest_rfsignals[i]);
		dest_rfsignal->sourceID = refSig.sig.rfSignals[i].sourceID;
		int8_t* dest_rssis = ((dj_int_array *)REF_TO_VOIDP(dest_rfsignal->rssi))->data.bytes;
		for (uint8_t j=0; j<NBR_FREQCHANNELS; j++) {
			dest_rssis[j] = refSig.sig.rfSignals[i].rssi[j][0];
		}
	}
}

void javax_rtcbench_DB_void_signature_get_javax_rtcbench_RefSignature_short() {
	int16_t index = dj_exec_stackPopShort();
	BM_MOTETRACK_STRUCT_javax_rtcbench_RefSignature * dest_refsignature = (BM_MOTETRACK_STRUCT_javax_rtcbench_RefSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());

	RefSignature refSig;
	SignatureDB_get(&refSig, index);

	BM_MOTETRACK_STRUCT_javax_rtcbench_Point * dest_location = (BM_MOTETRACK_STRUCT_javax_rtcbench_Point *)REF_TO_VOIDP(dest_refsignature->location);
	dest_location->x = refSig.location.x;
	dest_location->y = refSig.location.y;
	dest_location->z = refSig.location.z;

	BM_MOTETRACK_STRUCT_javax_rtcbench_Signature * dest_sig = (BM_MOTETRACK_STRUCT_javax_rtcbench_Signature *)REF_TO_VOIDP(dest_refsignature->sig);
	dest_sig->id = refSig.sig.id;

	ref_t * dest_rfsignals = ((dj_ref_array *)REF_TO_VOIDP(dest_sig->rfSignals))->refs;
	for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
		BM_MOTETRACK_STRUCT_javax_rtcbench_RFSignal * dest_rfsignal = (BM_MOTETRACK_STRUCT_javax_rtcbench_RFSignal *)REF_TO_VOIDP(dest_rfsignals[i]);
		dest_rfsignal->sourceID = refSig.sig.rfSignals[i].sourceID;
		int8_t* dest_rssis = ((dj_int_array *)REF_TO_VOIDP(dest_rfsignal->rssi))->data.bytes;
		for (uint8_t j=0; j<NBR_FREQCHANNELS; j++) {
			dest_rssis[j] = refSig.sig.rfSignals[i].rssi[j][0];
		}
	}
}