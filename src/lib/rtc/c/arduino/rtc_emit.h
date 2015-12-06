#ifndef RTC_EMIT_H
#define RTC_EMIT_H

#include <stdint.h>
#define RTC_CODEBUFFER_SIZE 64

void emit_init(uint16_t *rtc_codebuffer);

void emit(uint16_t opcode);
void emit2(uint16_t opcode1, uint16_t opcode2);
void emit_raw_word(uint16_t word);
void emit_without_optimisation(uint16_t word);

void emit_flush_to_flash();

#endif // RTC_EMIT_H