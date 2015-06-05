/*
 * PCF8833.h
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
// 12-bit color definitions
#ifndef PCF8833_H
#define PCF8833_H

#define WHITE		0xFFF
#define BLACK		0x000
#define RED			0xF00
#define GREEN		0x0F0
#define BLUE		0x00F
#define CYAN		0x1FF
#define MAGENTA		0xF0F
#define YELLOW		0xFF0
#define GRAY		0x222
#define LIGHTBLUE	0xADE
// #define PINK		0xF6A NR 20130103: conflicts with PINK (as in: PIN K) in the AVR header files

/* 8x8 font */
#define FONT_HEIGHT 8

#define LCDCommand      0
#define LCDData         1

/*Fill a box or not */
#define NOFILL	0
#define FILL 1

#define LCD_CS(x)           PORTB = (x)? (PORTB|(1<<PB4)) : (PORTB&~(1<<PB4))
#define LCD_DATA(x)         PORTB = (x)? (PORTB|(1<<PB5)) : (PORTB&~(1<<PB5))
#define LCD_CLK(x)          PORTB = (x)? (PORTB|(1<<PB7)) : (PORTB&~(1<<PB7))

#define LCD_RESET(x)        PORTH = (x)? (PORTH|(1<<PH6)) : (PORTH&~(1<<PH6))
#define LCD_BACKLIGHT(x)    PORTH = (x)? (PORTH|(1<<PH5)) : (PORTH&~(1<<PH5))

void SendLcd(uint8_t type, uint8_t dat);

void LCD_Init(void);
void avr_lcdGotoXY(uint8_t x, uint8_t y);
void LCD_ClearScreen(void);
void LCD_Pixel(uint8_t x, uint8_t y, int color);
void LCD_Fill(uint8_t xs, uint8_t ys, uint8_t w, uint8_t h, int color);
void LCD_Box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill, int color);
void LCD_Char(char ch, uint8_t x, uint8_t y, int fcolor, int bcolor);
void LCD_String(char *the_string, uint8_t x, uint8_t y, int fcolor, int bcolor);
void avr_lcdDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, int color);
void LCD_Circle(uint8_t center_x, uint8_t center_y, uint8_t rad, int color);
void LCD_Bitmap(uint8_t start_x, uint8_t start_y, uint8_t h_size, uint8_t v_size, uint8_t *bitmap_data);

void avr_lcdBacklight(uint8_t on);

// Philips PCF8833 LCD controller command codes
#define NOP			0x00  // nop
#define SWRESET		0x01  // software reset
#define BSTROFF		0x02  // booster voltage OFF
#define BSTRON		0x03  // booster voltage ON
#define RDDIDIF		0x04  // read display identification
#define RDDST		0x09  // read display status
#define SLEEPIN		0x10  // sleep in
#define SLEEPOUT	0x11  // sleep out
#define PTLON		0x12  // partial display mode
#define NORON		0x13  // display normal mode
#define INVOFF		0x20  // inversion OFF
#define INVON		0x21  // inversion ON
#define DALO		0x22  // all pixel OFF
#define DAL			0x23  // all pixel ON
#define SETCON		0x25  // write contrast
#define DISPOFF		0x28  // display OFF
#define DISPON		0x29  // display ON
#define CASET		0x2A  // column address set
#define PASET		0x2B  // page address set
#define RAMWR		0x2C  // memory write
#define RGBSET		0x2D  // colour set
#define PTLAR		0x30  // partial area
#define VSCRDEF		0x33  // vertical scrolling definition
#define TEOFF		0x34  // test mode
#define TEON		0x35  // test mode
#define MADCTL		0x36  // memory access control
#define SEP			0x37  // vertical scrolling start address
#define IDMOFF		0x38  // idle mode OFF
#define IDMON		0x39  // idle mode ON
#define COLMOD		0x3A  // interface pixel format
#define SETVOP		0xB0  // set Vop
#define BRS			0xB4  // bottom row swap
#define TRS			0xB6  // top row swap
#define DISCTR		0xB9  // display control
#define DOR			0xBA  // data order
#define TCDFE		0xBD  // enable/disable DF temperature compensation
#define TCVOPE		0xBF  // enable/disable Vop temp comp
#define EC			0xC0  // internal or external  oscillator
#define SETMUL		0xC2  // set multiplication factor
#define TCVOPAB		0xC3  // set TCVOP slopes A and B
#define TCVOPCD		0xC4  // set TCVOP slopes c and d
#define TCDF		0xC5  // set divider frequency
#define DF8COLOR	0xC6  // set divider frequency 8-color mode
#define SETBS		0xC7  // set bias system
#define RDTEMP		0xC8  // temperature read back
#define NLI			0xC9  // n-line inversion
#define RDID1		0xDA  // read ID1
#define RDID2		0xDB  // read ID2
#define RDID3		0xDC  // read ID3
#define RGB_8BIT	2
#define RGB_12BIT	3

#endif
