#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
#include "../../../../vm/c/common/execution.h"
#include "../../common/native_wuclasses/GENERATEDwuclass_led.h"


#define output_low(port, pin) port &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define set_output(portdir, pin) portdir |= (1<<pin)

#ifdef ENABLE_WUCLASS_LED

void wuclass_led_setup(wuobject_t *wuobject) {
    set_output(DDRK, 0);
    set_output(DDRK, 1);
    set_output(DDRK, 2);
    set_output(DDRK, 3);
}

void set_led(uint8_t pin, bool onOff) {
    if (onOff)
        output_low(PORTK, pin);
    else
        output_high(PORTK, pin);
}

void wuclass_led_update(wuobject_t *wuobject) {
    bool p1, p2, p3, p4;

    bool *onOff;
    onOff = wuclass_led_getPrivateData(wuobject);
    *onOff = !(*onOff);

    DEBUG_LOG(DBG_WKPFUPDATE, "WKCOMMUPDATE: led %d\n", *onOff);

    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT1, &p1);
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT2, &p2);
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT3, &p3);
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LED_PORT4, &p4);

    if(p1==true)
        set_led(0, *onOff);
    else
        set_led(0, false);
    if(p2==true)
        set_led(1, *onOff);
    else
        set_led(1, false);
    if(p3==true)
        set_led(2, *onOff);
    else
        set_led(2, false);
    if(p4==true)
        set_led(3, *onOff);
    else
        set_led(3, false);
}

#endif // ENABLE_WUCLASS_LED
