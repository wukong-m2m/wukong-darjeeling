#include "RFSignalAvgHT.h"

void printRFSignalAvgHT(RFSignalAvgHT *h) {
    avroraPrintInt16(h->size);
    avroraPrintInt16(h->capacity);
    for (int i=0; i<h->capacity; i++) {
        avroraRTCRuntimeBeep(i);
        avroraPrintInt16(h->htData[i].sourceID);
        avroraPrintInt16(h->htData[i].rssiSum[0][0]);
        avroraPrintInt16(h->htData[i].nbrSamples[0][0]);
        avroraPrintInt16(h->htData[i].rssiSum[1][0]);
        avroraPrintInt16(h->htData[i].nbrSamples[1][0]);
    }
    asm volatile ("break");
}
