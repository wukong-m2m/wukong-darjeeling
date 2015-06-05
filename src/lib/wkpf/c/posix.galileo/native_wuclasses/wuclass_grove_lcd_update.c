#include <stdint.h>
#include <stdio.h>
#include "native_wuclasses.h"
#include "LCD_RGB_Suli.h"
#include "config.h"

void wuclass_grove_lcd_setup(wuobject_t *wuobject) {
    #ifdef INTEL_GALILEO_GEN1
	rgb_lcd_init(0);
	rgb_lcd_clear();
	rgb_lcd_setRGB(255,0,0);
    #endif
    #ifdef INTEL_GALILEO_GEN2
	rgb_lcd_init(0);
	rgb_lcd_clear();
	rgb_lcd_setRGB(255,0,0);
    #endif
    #ifdef INTEL_EDISON
	rgb_lcd_init(6);
	rgb_lcd_clear();
	rgb_lcd_setRGB(255,0,0);
    #endif
}

void wuclass_grove_lcd_update(wuobject_t *wuobject) {
	int16_t value;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_GROVE_LCD_VALUE, &value);

    char buffer[128];
    snprintf(buffer, 128, "Valu:e%d", value);
    rgb_lcd_print(buffer);
}
