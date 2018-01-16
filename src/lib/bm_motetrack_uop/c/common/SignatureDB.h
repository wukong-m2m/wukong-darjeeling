#ifndef SIGNATUREDB_H
#define SIGNATUREDB_H
#include <stdint.h>
#include <avr/pgmspace.h>
#include "RefSignature.h"

// ===================== signatureDB Database ===================================
#define SIGNATUREDB_SIZE 74

extern const RefSignature signatureDB[] PROGMEM;

static inline void SignatureDB_get(RefSignature *refSigPtr, uint16_t indexDB) 
{ 
        memcpy_P(refSigPtr, (RefSignature*) &signatureDB[indexDB], sizeof(RefSignature)); 
}

// Made a copy of this function so when the AOT version of MoteTrack calls it, it doesn't get counted for the C version's cycles
static inline void SignatureDB_get_JVM(RefSignature *refSigPtr, uint16_t indexDB) 
{ 
        memcpy_P(refSigPtr, (RefSignature*) &signatureDB[indexDB], sizeof(RefSignature)); 
}

#endif
