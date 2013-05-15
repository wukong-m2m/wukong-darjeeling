#include "config.h" // To get RADIO_USE_ZWAVE
#include "types.h"
#include "djtimer.h"
#include "debug.h"

#ifdef RADIO_USE_ZWAVE

#include <avr/interrupt.h>

// Set from radio_zwave_learn_interrupt.c to start learn mode.
extern uint32_t zwave_time_btn_interrupt;
extern uint32_t zwave_time_btn_push;
extern uint32_t zwave_time_btn_release;
extern bool zwave_btn_is_push;
extern bool zwave_btn_is_release;

void radio_zwave_platform_dependent_init(void) {
    EICRA |= (0x02 & 0x03);//falling endge interrupt mode
    EIMSK |=_BV(0);//enable INT0

    DDRD &= ~_BV(0); 
    PORTD |= _BV(0);	//pull high

}

ISR(INT0_vect)
{
    EIMSK &=~_BV(0);//disable INT0
    //DEBUG_LOG(DBG_ZWAVETRACE,"is_push %d,PIND=%d",zwave_btn_is_push,PIND);
    if(zwave_btn_is_push==false)
    {
	//DEBUG_LOG(DBG_ZWAVETRACE,"=======push=========\n");
	if( (PIND&0x01) !=0 )//INT0=pins[3] bit0, 0=button push, otherwise is noise
	{
		EIMSK |=_BV(0);//enable INT0
	}
	else
	{
		zwave_time_btn_interrupt=dj_timer_getTimeMillis();
		zwave_time_btn_push=dj_timer_getTimeMillis();
		zwave_btn_is_push=true;
	}
    }
    else
    {
	//DEBUG_LOG(DBG_ZWAVETRACE,"=====release=========\n");
	if( (PIND&0x01) ==0 )//INT0=pins[3] bit0, 0=button push, otherwise is noise
	{
		EIMSK |=_BV(0);//enable INT0
	}
	else
	{
		zwave_time_btn_interrupt=dj_timer_getTimeMillis();
		zwave_time_btn_release=dj_timer_getTimeMillis();
		zwave_btn_is_push=false;
		zwave_btn_is_release=true;
	}
    }
}

#endif // RADIO_USE_ZWAVE
