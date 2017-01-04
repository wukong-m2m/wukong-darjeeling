#include "debug.h"
#include "wkreprog.h"
#include "rtc_emit.h"
#include "rtc_optimiser.h"

uint16_t *rtc_codebuffer;
uint16_t *rtc_codebuffer_position; // A pointer to somewhere within the buffer

void emit_init(uint16_t *codebuffer) {
    rtc_codebuffer = codebuffer;
    rtc_codebuffer_position = codebuffer;
}


void emit_raw_word(uint16_t word) {
    *(rtc_codebuffer_position++) = word;

    if (rtc_codebuffer_position >= rtc_codebuffer+RTC_CODEBUFFER_SIZE) { // Buffer full, do a flush.
        emit_flush_to_flash();
    }
}

void emit(uint16_t opcode) {
#ifdef AVRORA
    avroraRTCTraceSingleWordInstruction(opcode);
#endif
    emit_raw_word(opcode);
}

void emit2(uint16_t opcode1, uint16_t opcode2) {
#ifdef AVRORA
    avroraRTCTraceDoubleWordInstruction(opcode1, opcode2);
#endif
    emit_raw_word(opcode1);
    emit_raw_word(opcode2);
}

void emit_without_optimisation(uint16_t word) {
    emit_flush_to_flash();
    *(rtc_codebuffer_position++) = word;
    emit_flush_to_flash();
}

void emit_flush_to_flash() {
    if (rtc_codebuffer != rtc_codebuffer_position) {
        // Try to optimise the code currently in the buffer. This may affect rtc_codebuffer_position if we're able to compact the code.
        rtc_optimise(rtc_codebuffer, &rtc_codebuffer_position);

        uint8_t *instructiondata = (uint8_t *)rtc_codebuffer;
        uint16_t count = rtc_codebuffer_position - rtc_codebuffer;
#ifdef DARJEELING_DEBUG
        for (int i=0; i<count; i++) {
            DEBUG_LOG(DBG_RTCTRACE, "[rtc]    %x  (%x %x)\n", rtc_codebuffer[i], instructiondata[i*2], instructiondata[i*2+1]);
        }
#endif // DARJEELING_DEBUG
        // Write to flash buffer
        wkreprog_write(2*count, instructiondata);
        // Buffer is now empty
        rtc_codebuffer_position = rtc_codebuffer;
    }
}

