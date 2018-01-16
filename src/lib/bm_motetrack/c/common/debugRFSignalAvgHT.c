#include "RFSignalAvgHT.h"

void printRFSignalAvgHT(RFSignalAvgHT *h) {
    avroraPrintInt16(h->size);
    avroraPrintInt16(h->capacity);
    for (int i=0; i<h->capacity; i++) {
        avroraRTCRuntimeBeep(i);
        avroraPrintInt16(h->htData[i].sourceID);
        avroraPrintInt16(h->htData[i].rssiSum_0);
        avroraPrintInt16(h->htData[i].nbrSamples_0);
        avroraPrintInt16(h->htData[i].rssiSum_1);
        avroraPrintInt16(h->htData[i].nbrSamples_1);
    }
    asm volatile ("break");
}
