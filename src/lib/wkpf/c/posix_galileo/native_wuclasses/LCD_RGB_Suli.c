/*
  LCD_RGB_Suli.cpp

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

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include "mraa/mraa.h"
#include "LCD_RGB_Suli.h"

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

uint8_t _initialized;

uint8_t _numlines;
uint8_t _currline;


mraa_i2c_context __I2C_Device;

void i2c_send_byte(uint8_t dta)
{
    mraa_result_t rv;
    rv = mraa_i2c_address(__I2C_Device, LCD_ADDRESS);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_address: %d\n\r", rv);        
    }
    rv = mraa_i2c_write_byte(__I2C_Device, dta);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_write_byte: %d\n\r", rv);        
    }
//    suli_i2c_write(__I2C_Device, LCD_ADDRESS, &dta, 1);
}

void i2c_send_byteS(uint8_t *dta, uint8_t len)
{
    mraa_result_t rv;
    rv = mraa_i2c_address(__I2C_Device, LCD_ADDRESS);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_address: %d\n\r", rv);        
    }
    rv = mraa_i2c_write(__I2C_Device, dta, len);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_write: %d\n\r", rv);        
        printf("mraa_i2c_write errno: %s \n\r", strerror(errno));
    }
//    suli_i2c_write(__I2C_Device, LCD_ADDRESS, dta, len);
}

void rgb_lcd_set_reg(uint8_t addr, uint8_t data)
{
    mraa_result_t rv;
    uint8_t dta[] = {addr, data};
    rv = mraa_i2c_address(__I2C_Device, RGB_ADDRESS);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_address: %d\n\r", rv);        
    }
    rv = mraa_i2c_write(__I2C_Device, dta, 2);
    if (rv != MRAA_SUCCESS) {
        printf("mraa_i2c_write: %d\n\r", rv);        
        printf("mraa_i2c_write errno: %s \n\r", strerror(errno));
    }
//    suli_i2c_write(__I2C_Device, RGB_ADDRESS, dta, 2);
}

void rgb_lcd_init2(mraa_i2c_context i2c_dev, uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
    __I2C_Device = i2c_dev;

    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    usleep(50000);

    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set rgb_lcd_command sequence
    rgb_lcd_command(LCD_FUNCTIONSET | _displayfunction);
    usleep(4500);  // wait more than 4.1ms

    // second try
    rgb_lcd_command(LCD_FUNCTIONSET | _displayfunction);
    usleep(150);

    // third go
    rgb_lcd_command(LCD_FUNCTIONSET | _displayfunction);


    // finally, set # lines, font size, etc.
    rgb_lcd_command(LCD_FUNCTIONSET | _displayfunction);

    // turn the rgb_lcd_display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    rgb_lcd_display();

    // rgb_lcd_clear it off
    rgb_lcd_clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    rgb_lcd_command(LCD_ENTRYMODESET | _displaymode);
    
    
    // backlight init
    rgb_lcd_set_reg(0, 0);
    rgb_lcd_set_reg(1, 0);
    rgb_lcd_set_reg(0x08, 0xAA);     // all led control by pwm
    
    rgb_lcd_setCursor(0, 0);
    // setColorWhite();

}
void rgb_lcd_init(int bus) {
    mraa_i2c_context dev = mraa_i2c_init(bus);
    if (dev == NULL) {
        printf("mraa_i2c_init failed\n");
        exit(1);
    }
    rgb_lcd_init2(dev, 16, 2, LCD_5x8DOTS);
    rgb_lcd_setRGB(255, 255, 255);
}

/********** high level commands, for the user! */
void rgb_lcd_clear()
{
    rgb_lcd_command(LCD_CLEARDISPLAY);        // rgb_lcd_clear rgb_lcd_display, set cursor position to zero
    usleep(2000);          // this rgb_lcd_command takes a long time!
}

void rgb_lcd_home()
{
    rgb_lcd_command(LCD_RETURNHOME);        // set cursor position to zero
    usleep(2000);        // this rgb_lcd_command takes a long time!
}

void rgb_lcd_setCursor(uint8_t col, uint8_t row)
{
    col = (row == 0 ? col|0x80 : col|0xc0);
    uint8_t dta[2] = {0x80, col};
    i2c_send_byteS(dta, 2);
}

// Turn the rgb_lcd_display on/off (quickly)
void rgb_lcd_noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd_display() {
    _displaycontrol |= LCD_DISPLAYON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void rgb_lcd_noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd_cursor() {
    _displaycontrol |= LCD_CURSORON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void rgb_lcd_noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd_blink()
{
    _displaycontrol |= LCD_BLINKON;
    rgb_lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the rgb_lcd_display without changing the RAM
void rgb_lcd_scrollDisplayLeft(void)
{
    rgb_lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void rgb_lcd_scrollDisplayRight(void)
{
    rgb_lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void rgb_lcd_leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    rgb_lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rgb_lcd_rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    rgb_lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void rgb_lcd_autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    rgb_lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void rgb_lcd_no_autoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    rgb_lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void rgb_lcd_create_char(uint8_t location, uint8_t charmap[])
{

    location &= 0x7; // we only have 8 locations 0-7
    rgb_lcd_command(LCD_SETCGRAMADDR | (location << 3));
    
    
    uint8_t dta[9];
    dta[0] = 0x40;
    int i;
    for(i=0; i<8; i++)
    {
        dta[i+1] = charmap[i];
    }
    i2c_send_byteS(dta, 9);
}

/*********** mid level commands, for sending data/cmds */

// send rgb_lcd_command
void rgb_lcd_command(uint8_t value)
{
    uint8_t dta[2] = {0x80, value};
    i2c_send_byteS(dta, 2);
}

// send data
void rgb_lcd_write(uint8_t value)
{
    uint8_t dta[2] = {0x40, value};
    i2c_send_byteS(dta, 2);
}

void rgb_lcd_print(char *str)
{
    while(*str)
    {
        rgb_lcd_write(*str);
        str++;
    }
}

void rgb_lcd_setRGB(uint8_t r, uint8_t g, uint8_t b)
{
    rgb_lcd_set_reg(REG_RED, r);
    rgb_lcd_set_reg(REG_GREEN, g);
    rgb_lcd_set_reg(REG_BLUE, b);
}

void rgb_lcd_setPwm(uint8_t color, uint8_t pwm)
{
    rgb_lcd_set_reg(color, pwm);
}

const uint8_t color_define[4][3] = 
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

void rgb_lcd_setColor(uint8_t color)
{
    if(color > 3)return ;
    rgb_lcd_setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

