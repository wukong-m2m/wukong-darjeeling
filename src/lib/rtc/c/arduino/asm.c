#include <stdint.h>
#include <stdbool.h>
#include "asm.h"

bool rtc_is_double_word_instruction(uint16_t instruction) {
    const uint16_t CALL_MASK            = 0xFE0E;
    const uint16_t LDS_MASK             = 0xFE0F;
    const uint16_t STS_MASK             = 0xFE0F;

    return (instruction & CALL_MASK)                == OPCODE_CALL         // 1001 010k kkkk 111k kkkk kkkk kkkk kkkk 
            || (instruction & LDS_MASK)             == OPCODE_LDS          // 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
            || (instruction & STS_MASK)             == OPCODE_STS;         // 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
}
