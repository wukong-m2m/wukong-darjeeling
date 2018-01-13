#ifndef REFSIGNATUREDB_H
#define REFSIGNATUREDB_H
#include <stdint.h>
#include <avr/pgmspace.h>
#include "RefSignature.h"

// ===================== refSignatureDB Database ===================================
#define REFSIGNATUREDB_SIZE 257

extern const RefSignature refSignatureDB[] PROGMEM;

static inline void RefSignatureDB_get(RefSignature *refSigPtr, uint16_t indexDB) 
{ 
        memcpy_P(refSigPtr, (RefSignature*) &refSignatureDB[indexDB], sizeof(RefSignature)); 
}

// Made a copy of this function so when the AOT version of MoteTrack calls it, it doesn't get counted for the C version's cycles
static inline void RefSignatureDB_get_JVM(RefSignature *refSigPtr, uint16_t indexDB) 
{ 
        memcpy_P(refSigPtr, (RefSignature*) &refSignatureDB[indexDB], sizeof(RefSignature)); 
}

#endif
