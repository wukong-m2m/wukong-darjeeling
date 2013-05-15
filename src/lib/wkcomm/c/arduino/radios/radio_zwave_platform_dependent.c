#include "config.h" // To get RADIO_USE_ZWAVE

#ifdef RADIO_USE_ZWAVE

#include <avr/interrupt.h>

// Set from radio_zwave_learn_interrupt.c to start learn mode.
extern uint8_t zwave_learn_mode;

void radio_zwave_platform_dependent_init(void) {
    EICRA |= (0 & 0x03); //GND interrupt mode
    EIMSK |=_BV(0); //turn on INT0
}

ISR(INT0_vect)
{
    zwave_learn_mode = 1;
}

#endif // RADIO_USE_ZWAVE