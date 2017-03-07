#include "config.h"
#ifdef MRAA_LIBRARY

#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mraa.h>

mraa_gpio_context tv_onoff_in_gpio;  //pin 3
mraa_gpio_context tv_onoff_out_gpio; //pin 4
mraa_gpio_context tv_mute_out_gpio;  //pin 8

void wuclass_irremote_tv_setup(wuobject_t *wuobject) {
    tv_onoff_in_gpio = mraa_gpio_init(3);
    mraa_gpio_dir(tv_onoff_in_gpio, MRAA_GPIO_IN);
    
    tv_onoff_out_gpio = mraa_gpio_init(4);
    mraa_gpio_dir(tv_onoff_out_gpio, MRAA_GPIO_OUT);
    
    tv_mute_out_gpio = mraa_gpio_init(8);
    mraa_gpio_dir(tv_mute_out_gpio, MRAA_GPIO_OUT);
}

void wuclass_irremote_tv_update(wuobject_t *wuobject) {
    bool on_off;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_ON_OFF, &on_off);
    mraa_gpio_write(tv_onoff_out_gpio, on_off);		
   
    bool mute;
    wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_MUTE, &mute);
    mraa_gpio_write(tv_mute_out_gpio, mute);		

    bool on_off_state;
    int value_i;
    value_i = mraa_gpio_read(tv_onoff_in_gpio);
    on_off_state = (value_i != 0);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(IRremote TV): Sensed on_off_state: %d\n", value_i); 
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_IRREMOTE_TV_ON_OFF_STATE, on_off_state);
}
#endif