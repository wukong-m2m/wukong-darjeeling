/*
  LCD_RGB_Suli.h

  This is a Suly compatible Library
  
  2014 Copyright (c) Seeed Technology Inc.  All right reserved.
  
  Loovee
  2014-4-18

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __LCD_RGB_SULI_H__
#define __LCD_RGB_SULI_H__


// #include "Suli.h"


// Device I2C Arress
#define LCD_ADDRESS     (0x7c >> 1)
#define RGB_ADDRESS     (0xc4 >> 1)


// color define 
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#include <stdint.h>


void rgb_lcd_init();
//void rgb_lcd_init(void *i2c_dev, uint8_t cols, uint8_t lines, uint8_t dotsize);

void rgb_lcd_clear();
void rgb_lcd_home();
void rgb_lcd_setCursor(uint8_t col, uint8_t row);

// Turn the display on/off (quickly)
void rgb_lcd_noDisplay();
void rgb_lcd_display();
// Turns the underline cursor on/off
void rgb_lcd_noCursor();

void rgb_lcd_cursor();

// Turn on and off the blinking cursor
void rgb_lcd_noBlink();
void rgb_lcd_blink();
// These commands scroll the display without changing the RAM
void rgb_lcd_scrollDisplayLeft(void);
void rgb_lcd_scrollDisplayRight(void);

// This is for text that flows Left to Right
void rgb_lcd_leftToRight(void);
// This is for text that flows Right to Left
void rgb_lcd_rightToLeft(void);

// This will 'right justify' text from the cursor
void rgb_lcd_autoscroll(void);
// This will 'left justify' text from the cursor
void rgb_lcd_no_autoscroll(void);
// Allows us to fill the first 8 CGRAM locations
// with custom characters
void rgb_lcd_create_char(uint8_t location, uint8_t charmap[]);
/*********** mid level commands, for sending data/cmds */

// send command
void rgb_lcd_command(uint8_t value);
// send data
void rgb_lcd_write(uint8_t value);
void rgb_lcd_print(char *str);
void rgb_lcd_setRGB(uint8_t r, uint8_t g, uint8_t b);

void rgb_lcd_setPwm(uint8_t color, uint8_t pwm);


#endif 

