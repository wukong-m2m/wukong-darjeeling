#include "execution.h"
#include "array.h"
#include "jlib_bm_motetrack.h"
#include "RefSignature.h"
#include "SignatureDB.h"
#include "RefSignatureDB.h"

int16_t MoteTrack_DB_index;
BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RefSignature * MoteTrack_DB_dest_refsignature;
uint16_t MoteTrack_DB_returnaddress;

void javax___read_refsignature_from_flash() {
    RefSignature refSig;
    RefSignatureDB_get_JVM(&refSig, MoteTrack_DB_index);

    BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Point * dest_location = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Point *)REF_TO_VOIDP(MoteTrack_DB_dest_refsignature->location);
    dest_location->x = refSig.location.x;
    dest_location->y = refSig.location.y;
    dest_location->z = refSig.location.z;

    BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Signature * dest_sig = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Signature *)REF_TO_VOIDP(MoteTrack_DB_dest_refsignature->sig);
    dest_sig->id = refSig.sig.id;

    ref_t * dest_rfsignals = ((dj_ref_array *)REF_TO_VOIDP(dest_sig->rfSignals))->refs;
    for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
        BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RFSignal * dest_rfsignal = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RFSignal *)REF_TO_VOIDP(dest_rfsignals[i]);
        dest_rfsignal->sourceID = refSig.sig.rfSignals[i].sourceID;
        dest_rfsignal->rssi_0 = refSig.sig.rfSignals[i].rssi_0;
        dest_rfsignal->rssi_1 = refSig.sig.rfSignals[i].rssi_1;
    }
}

void javax___read_signature_from_flash() {
    RefSignature refSig;
    SignatureDB_get_JVM(&refSig, MoteTrack_DB_index);

    BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Point * dest_location = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Point *)REF_TO_VOIDP(MoteTrack_DB_dest_refsignature->location);
    dest_location->x = refSig.location.x;
    dest_location->y = refSig.location.y;
    dest_location->z = refSig.location.z;

    BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Signature * dest_sig = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_Signature *)REF_TO_VOIDP(MoteTrack_DB_dest_refsignature->sig);
    dest_sig->id = refSig.sig.id;

    ref_t * dest_rfsignals = ((dj_ref_array *)REF_TO_VOIDP(dest_sig->rfSignals))->refs;
    for (uint8_t i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
        BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RFSignal * dest_rfsignal = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RFSignal *)REF_TO_VOIDP(dest_rfsignals[i]);
        dest_rfsignal->sourceID = refSig.sig.rfSignals[i].sourceID;
        dest_rfsignal->rssi_0 = refSig.sig.rfSignals[i].rssi_0;
        dest_rfsignal->rssi_1 = refSig.sig.rfSignals[i].rssi_1;
    }
}


#ifndef NO_LIGHTWEIGHT_METHODS
void javax_rtcbench_DB_void_refSignature_get_javax_rtcbench_RefSignature_short() {
    asm volatile("   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  MoteTrack_DB_returnaddress+1, r18" "\n\r"
                 "   sts  MoteTrack_DB_returnaddress, r19" "\n\r"
                 "   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  MoteTrack_DB_index, r18" "\n\r"
                 "   sts  MoteTrack_DB_index+1, r19" "\n\r"
                 "   ld   r18, -x" "\n\r"
                 "   ld   r19, -x" "\n\r"
                 "   sts  MoteTrack_DB_dest_refsignature+1, r18" "\n\r"
                 "   sts  MoteTrack_DB_dest_refsignature, r19" "\n\r"
                 "   push r26" "\n\r"
                 "   push r27" "\n\r"
                 "   call javax___read_refsignature_from_flash" "\n\r"
                 "   pop  r27" "\n\r"
                 "   pop  r26" "\n\r"
                 "   lds  r19, MoteTrack_DB_returnaddress" "\n\r"
                 "   lds  r18, MoteTrack_DB_returnaddress+1" "\n\r"
                 "   push r19" "\n\r"
                 "   push r18" "\n\r"
             :: );
}
void javax_rtcbench_DB_void_signature_get_javax_rtcbench_RefSignature_short() {
    asm volatile("   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  MoteTrack_DB_returnaddress+1, r18" "\n\r"
                 "   sts  MoteTrack_DB_returnaddress, r19" "\n\r"
                 "   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  MoteTrack_DB_index, r18" "\n\r"
                 "   sts  MoteTrack_DB_index+1, r19" "\n\r"
                 "   ld   r18, -x" "\n\r"
                 "   ld   r19, -x" "\n\r"
                 "   sts  MoteTrack_DB_dest_refsignature+1, r18" "\n\r"
                 "   sts  MoteTrack_DB_dest_refsignature, r19" "\n\r"
                 "   push r26" "\n\r"
                 "   push r27" "\n\r"
                 "   call javax___read_signature_from_flash" "\n\r"
                 "   pop  r27" "\n\r"
                 "   pop  r26" "\n\r"
                 "   lds  r19, MoteTrack_DB_returnaddress" "\n\r"
                 "   lds  r18, MoteTrack_DB_returnaddress+1" "\n\r"
                 "   push r19" "\n\r"
                 "   push r18" "\n\r"
             :: );
}
#else
void javax_rtcbench_DB_void_refSignature_get_javax_rtcbench_RefSignature_short() {
    MoteTrack_DB_index = dj_exec_stackPopShort();
    MoteTrack_DB_dest_refsignature = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RefSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());
    javax___read_refsignature_from_flash();
}
void javax_rtcbench_DB_void_signature_get_javax_rtcbench_RefSignature_short() {
    MoteTrack_DB_index = dj_exec_stackPopShort();
    MoteTrack_DB_dest_refsignature = (BM_MOTETRACK_UOP_STRUCT_javax_rtcbench_RefSignature *)REF_TO_VOIDP(dj_exec_stackPopRef());
    javax___read_signature_from_flash();
}
#endif

