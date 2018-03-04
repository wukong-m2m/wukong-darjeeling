#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"


// Split into separate function to avoid the compiler just optimising away the whole test.
void compress(int16_t ri, int16_t ri_1, uint8_t **stream, uint8_t *stream_bits_used_in_current_byte);
void encode(int16_t di, uint32_t *bsi, uint8_t *bsi_length);
uint8_t computeBinaryLog(uint16_t di);

uint8_t __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[], uint8_t output[]) {
    rtc_startBenchmarkMeasurement_Native();

    uint8_t *stream = output;
    uint8_t stream_bits_used_in_current_byte = 0;

    int16_t ri_1 = 0;
    for (uint16_t i=0; i<NUMNUMBERS; i++) {
        int16_t ri = numbers[i];
        compress(ri, ri_1, &stream, &stream_bits_used_in_current_byte);

        ri_1 = ri;
    }

    rtc_stopBenchmarkMeasurement();

    return stream-output+1; // The number of bytes in the output array
}

uint16_t si_tbl[] = {
    0b00,
    0b010,
    0b011,
    0b100,
    0b101,
    0b110,
    0b1110,
    0b11110,
    0b111110,
    0b1111110,
    0b11111110,
    0b111111110,
    0b1111111110,
    0b11111111110,
    0b111111111110,
    0b1111111111110,
    0b11111111111110,
};

uint8_t si_length_tbl[] = {
    2,
    3,
    3,
    3,
    3,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,    
};

// pseudo code from the paper:

// compress(ri, ri_1, stream)
//     // compute difference di
//     SET di TO ri - ri_1
//     // encode difference di
//     CALL encode() with di RETURNING bsi
//     // append bsi to stream
//     SET stream TO <<stream,bsi>>
//     RETURN stream
void compress(int16_t ri, int16_t ri_1, uint8_t **stream, uint8_t *stream_bits_used_in_current_byte) {
    // compute difference di
    int16_t di = ri - ri_1;
    // encode difference di
    uint32_t bsi;
    uint8_t bsi_length;
    encode(di, &bsi, &bsi_length);
    // append bsi to stream
    uint8_t bits_left_current_in_byte = 8 - *stream_bits_used_in_current_byte;
    while (bsi_length > 0) {
        if (bsi_length > bits_left_current_in_byte) {
            // Not enough space to store all bits

            // Calculate bits to write to current byte
            uint8_t bits_to_add_to_current_byte = bsi >> (bsi_length - bits_left_current_in_byte);

            // Add them to the current byte
            **stream |= bits_to_add_to_current_byte;
            // Remove those bits from the to-do list
            bsi_length -= bits_left_current_in_byte;

            // Advance the stream to the next byte
            (*stream)++;
            // Whole new byte for the next round
            bits_left_current_in_byte = 8;
        } else {
            // Enough space to store all bits

            // After this we'll have -bsi_length bits left.
            bits_left_current_in_byte -= bsi_length;

            // Calculate bits to write to current byte
            uint8_t bits_to_add_to_current_byte = bsi << bits_left_current_in_byte;

            // Add them to the current byte
            **stream |= bits_to_add_to_current_byte;
            // Remove those bits from the to-do list
            bsi_length = 0;
        }
    }

    *stream_bits_used_in_current_byte = 8 - bits_left_current_in_byte;
    // Note that if we filled the last byte, stream_bits_used_in_current_byte will be 8,
    // which means in the next call to encode the first iteration of the while loop
    // won't do anything, except advance the stream pointer.
}

// encode(di)
//     // compute di category
//     IF di = 0
//         SET ni to 0
//     ELSE
//         SET ni to CEIL(log_2(|di|))
//     ENDIF
//     // extract si from Table
//     SET si TO Table[ni]
//     // build bsi
//     IF ni = 0 THEN
//         // ai is not needed
//         SET bsi to si
//     ELSE
//         // build ai
//         IF di > 0 THEN
//             SET ai TO (di)|ni
//         ELSE
//             SET ai TO (di-1)|ni
//         ENDIF
//         // build bsi
//         SET bsi TO <<si,ai>>
//     ENDIF
//     RETURN bsi
void encode(int16_t di, uint32_t *bsi, uint8_t *bsi_length) {
    // compute di category
    uint16_t di_abs;
    if (di < 0) {
        di_abs = -di;
    } else {
        di_abs = di;
    }
    uint8_t ni = computeBinaryLog(di_abs);
    // extract si from Table
    uint16_t si = si_tbl[ni];
    uint8_t si_length = si_length_tbl[ni];
    uint16_t ai = 0;
    uint8_t ai_length = 0;
    // build bsi
    if (ni == 0) {
        *bsi = si;
        *bsi_length = si_length;
    } else {
        // build ai
        if (di > 0) {
            ai = di;
            ai_length = ni;
        } else {
            ai = di-1;
            ai_length = ni;            
        }
        *bsi = (si << ai_length) | (ai & ((1 << ni) -1));
        *bsi_length = si_length + ai_length;
    }
}

// computeBinaryLog(di)
//     // CEIL(log_r|di|)
//     SET ni TO 0
//     WHILE di > 0
//         SET di TO di/2
//         SET ni to ni + 1
//     ENDWHILE
//     RETURN ni
uint8_t computeBinaryLog(uint16_t di) {
    uint8_t ni = 0;
    while (di > 0) {
        di >>= 1;
        ni++;
    }
    return ni;
}
