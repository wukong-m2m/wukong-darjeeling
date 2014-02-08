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
#ifdef USE_INT_AS_BUTTON	
    EICRA |= (0x01 & 0x03);//falling endge interrupt mode
    EIMSK |=_BV(0);//enable INT0
#endif    

	// PE4: D2
    DDRE &= ~_BV(4); 
    PORTE |= _BV(4);	//pull high
	DDRG &= ~_BV(5);
	PORTG |= _BV(5);
	PORTK = 0xf;

}

static uint32_t zwave_pg5_press=0;
static uint32_t zwave_pe4_press=0;
static uint32_t zwave_led_time=0;
void radio_zwave_platform_dependent_poll(void) {
	if (zwave_pg5_press==0 && zwave_pe4_press==0) {
		if ((PING&_BV(5))==0) {
			zwave_pg5_press = dj_timer_getTimeMillis();
			zwave_time_btn_push=dj_timer_getTimeMillis();
			zwave_btn_is_push = true;
			PORTK &= ~_BV(0);
			DEBUG_LOG(DBG_WKCOMM,"PG5 is pressed");
		}
		if ((PINE&_BV(4))==0) {
			zwave_pe4_press = dj_timer_getTimeMillis();
			zwave_time_btn_push=dj_timer_getTimeMillis();
			zwave_btn_is_push = true;
			PORTK &= ~_BV(0);
			DEBUG_LOG(true,"PE4 0\n");
		}
	} else if (zwave_pe4_press != 0) {
		if ((PINE&_BV(4))) {
			if (dj_timer_getTimeMillis() > zwave_pe4_press + 100) {
				zwave_btn_is_release = 1;
				zwave_time_btn_release=dj_timer_getTimeMillis();
				zwave_pe4_press = 0;
				zwave_btn_is_push=false;
				zwave_btn_is_release=true;
				zwave_led_time = zwave_time_btn_release;
				PORTK &= ~_BV(0);
				DEBUG_LOG(true,"PE4 1");
			}
		}
	} else {
		if ((PING&_BV(5))) {
			if (dj_timer_getTimeMillis() > zwave_pg5_press + 100) {
				zwave_btn_is_release = 1;
				zwave_time_btn_release=dj_timer_getTimeMillis();
				zwave_pg5_press = 0;
				zwave_btn_is_push=false;
				zwave_btn_is_release=true;
				zwave_led_time = zwave_time_btn_release;
				PORTK &= ~_BV(0);
			}
		}
	}
	if ( zwave_led_time > 0 && dj_timer_getTimeMillis() > zwave_led_time) {
		PORTK |= _BV(0);
		zwave_led_time = 0;
	}
#ifdef USE_INT_AS_BUTTON	
    if( (EIMSK&0x01) ==0 )//INT0 is disable
    {
	    if( (dj_timer_getTimeMillis()-zwave_time_btn_interrupt)>100 )//wait 100ms for button debounce, enable interrupt again
	    {
		    EIFR |=_BV(0);//clear INT0 flag
		    EIMSK |=_BV(0);//enable INT0
	    }
    }
#endif    
}
#ifdef USE_INT_AS_BUTTON	
ISR(INT0_vect)
{
    EIMSK &=~_BV(0);//disable INT0
	if (zwave_pg5_press) return;
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
			PORTK |= _BV(0);
			zwave_led_time = zwave_time_btn_release + 500;
		}
    }
}
#endif
#endif // RADIO_USE_ZWAVE
