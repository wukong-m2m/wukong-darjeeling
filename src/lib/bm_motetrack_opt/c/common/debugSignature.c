#include "RefSignature.h"

void printSignature(Signature *s) {
    avroraPrintInt16(s->id);
    for (int i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
        avroraRTCRuntimeBeep(i);
        avroraPrintInt16(s->rfSignals[i].sourceID);
        avroraPrintInt16(s->rfSignals[i].rssi[0]);
        avroraPrintInt16(s->rfSignals[i].rssi[1]);
    }
    asm volatile ("break");
}

void printRefSignature(RefSignature *r) {
    avroraPrintInt16(r->location.x);
    avroraPrintInt16(r->location.y);
    avroraPrintInt16(r->location.z);
    avroraPrintInt16(r->sig.id);
    for (int i=0; i<NBR_RFSIGNALS_IN_SIGNATURE; i++) {
        avroraRTCRuntimeBeep(i);
        avroraPrintInt16(r->sig.rfSignals[i].sourceID);
        avroraPrintInt16(r->sig.rfSignals[i].rssi[0]);
        avroraPrintInt16(r->sig.rfSignals[i].rssi[1]);
    }
    asm volatile ("break");
}
