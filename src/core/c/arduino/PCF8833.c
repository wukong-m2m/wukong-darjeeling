/*
 * PCF8833.c
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
 
/* PCF8833.c  -- c program for NOkia 6610 (Philips) LCD */
/* most of the source code are extracted from James Lynch's LCD application note */

#include <avr/io.h>
#include "PCF8833.h"
#include "FONT8x8.h"

#include "avr.h"
#include "spi.h"

void avr_lcdSendData(uint8_t data)
{
	avr_sspiSlaveSelectEnable();
	avr_sspiSendBit(1);
	avr_sspiSendByte(data);
	avr_sspiSlaveSelectDisable();
}

void avr_lcdSendColor(uint8_t color)
{
	avr_sspiSendBit(1);
	avr_sspiSendByte(color);
}

void avr_lcdSendCommand(uint8_t command)
{
	avr_sspiSlaveSelectEnable();
	avr_sspiSendBit(0);
	avr_sspiSendByte(command);
	avr_sspiSlaveSelectDisable();
}

void avr_lcdBacklight(uint8_t on)
{
	PORTH = on ? (PORTH|(1<<PH5)) : (PORTH&~(1<<PH5));
};

void avr_lcdReset(uint8_t value)
{
	PORTH = value ? (PORTH|(1<<PH6)) : (PORTH&~(1<<PH6));
};

void LCD_Init(void)
{
	// initialise software SPI (sets SPI pins to output)
	avr_sspiInit();

	// set pins for backlight and reset to output
	DDRH |= 0x30;

	// reset the LCD
	avr_sspiSlaveSelectDisable();
	avr_sspiClock(0);
	avr_sspiMoSi(0);

	avr_lcdReset(1);
	avr_delay(50);
	avr_lcdReset(0);
	avr_delay(50);
	avr_lcdReset(1);
	avr_delay(50);

	avr_sspiSlaveSelectEnable();
	avr_sspiClock(1);
	avr_sspiMoSi(1);

	// Turn screen on
	avr_lcdSendCommand(SWRESET);
	avr_delay(10);

	// Turn screen on
	avr_lcdSendCommand(SLEEPOUT); // Sleepout
	// avr_lcdSendCommand(INVON);  // Invert display mode
	// SendLcd(LCDCommand,BSTRON);  // BoostON
	avr_lcdSendCommand(MADCTL); // memory access control
	avr_lcdSendData(0xe0);

	// Set contrast
	avr_lcdSendCommand(SETCON);
	avr_lcdSendData(0x40);
	avr_delay(10);

	// display on
	avr_lcdSendCommand(DISPON);

	// set color mode to 12 bpp
	avr_lcdSendCommand(COLMOD);
	avr_lcdSendData(0x03);
	avr_lcdSendCommand(NOP);

}
void avr_lcdGotoXY(uint8_t x, uint8_t y)
{
	avr_lcdSendCommand(PASET); // page start/end ram
	avr_lcdSendData(x); // Start Page to display to
	avr_lcdSendData(x); // End Page to display to

	avr_lcdSendCommand(CASET); // column start/end ram
	avr_lcdSendData(y); // Start Column to display to
	avr_lcdSendData(y); // End Column to display to
}

void avr_lcdPutPixel(uint8_t x, uint8_t y, int color)
{
	avr_lcdGotoXY(x, y);
	avr_lcdSendCommand(RAMWR);
	avr_sspiSlaveSelectEnable();
	//  avr_lcdSendData((uint8_t)((color>>4)& 0xff) );
	//  avr_lcdSendData((uint8_t)((color&0xf)<<4)|0x00 );
	avr_lcdSendColor((uint8_t) ((color >> 4) & 0xff));
	avr_lcdSendColor((uint8_t) ((color & 0xf) << 4) | 0x00);
	avr_sspiSlaveSelectDisable();
	avr_lcdSendCommand(NOP);
}

//  *************************************************************************************************
//
//  Draws a line in the specified color from (x0,y0) to (x1,y1)
//
// Inputs:   x     =   row address (0 .. 131)
//      y     =   column address  (0 .. 131)
//      color =   12-bit color value  rrrrggggbbbb
//     rrrr = 1111 full red
//          :
//      0000 red is off
//
//     gggg = 1111 full green
//          :
//      0000 green is off
//
//     bbbb = 1111 full blue
//          :
//      0000 blue is off
//
//  Returns:   nothing
//
//  Note:  good write-up on this algorithm in Wikipedia (search for Bresenham's line algorithm)
//    see lcd.h for some sample color settings
//
//  Authors:   Dr. Leonard McMillan, Associate Professor UNC
//      Jack Bresenham IBM, Winthrop University (Father of this algorithm, 1962)
//
//      Note: taken verbatim from Professor McMillan's presentation:
//            http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
//
//  *************************************************************************************************

void avr_lcdDrawLine(uint8_t x0, uint8_t y0, uint8_t x1,
		uint8_t y1, int color) {

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}

	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx


	avr_lcdPutPixel(x0, y0, color);

	if (dx > dy) {
		int fraction = dy - (dx >> 1); // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			x0 += stepx;

			fraction += dy; // same as fraction -= 2*dy
			avr_lcdPutPixel(x0, y0, color);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			avr_lcdPutPixel(x0, y0, color);
		}
	}

}

//  *****************************************************************************************
//
//  Draws a rectangle in the specified color from (x1,y1) to (x2,y2)
//  Rectangle can be filled with a color if desired
//
// Inputs:   x     =   row address (0 .. 131)
//      y     =   column address  (0 .. 131)
//      fill  =   0=no fill, 1-fill entire rectangle
//      color =   12-bit color value for lines  rrrrggggbbbb
//   Returns:   nothing
//
//   Notes:
//
//    The best way to fill a rectangle is to take advantage of the "wrap-around" featute
//    built into the Philips PCF8833 controller. By defining a drawing box, the memory can
//    be simply filled by successive memory writes until all pixels have been illuminated.
//
//      1.  Given the coordinates of two opposing corners (x0, y0) (x1, y1)
//          calculate the minimums and maximums of the coordinates
//
//        xmin = (x0 <= x1) ? x0 : x1;
//        xmax = (x0 > x1) ? x0 : x1;
//        ymin = (y0 <= y1) ? y0 : y1;
//        ymax = (y0 > y1) ? y0 : y1;
//
//      2. Now set up the drawing box to be the desired rectangle
//
//     WriteSpiCommand(PASET);   // set the row boundaries
//     WriteSpiData(xmin);
//     WriteSpiData(xmax);
//     WriteSpiCommand(CASET);   // set the column boundaries
//     WriteSpiData(ymin);
//     WriteSpiData(ymax);
//
//      3. Calculate the number of pixels to be written divided by 2
//
//        NumPixels = ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1)
//
//        You may notice that I added one pixel to the formula.
//        This covers the case where the number of pixels is odd and we
//        would lose one pixel due to rounding error. In the case of
//        odd pixels, the number of pixels is exact.
//        in the case of even pixels, we have one more pixel than
//        needed, but it cannot be displayed because it is outside
//     the drawing box.
//
//        We divide by 2 because two pixels are represented by three bytes.
//        So we work through the rectangle two pixels at a time.
//
//      4.  Now a simple memory write loop will fill the rectangle
//
//        for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1); i++) {
//      WriteSpiData((color >> 4) & 0xFF);
//          WriteSpiData(((color & 0xF) << 4) | ((color >> 8) & 0xF));
//      WriteSpiData(color & 0xFF);
//     }
//
//    In the case of an unfilled rectangle, drawing four lines with the Bresenham line
//    drawing algorithm is reasonably efficient.
//
//    Author:  James P Lynch      July 7, 2007
//  *****************************************************************************************


void LCD_Box(
		uint8_t x0, uint8_t y0, uint8_t x1,
		uint8_t y1, uint8_t fill,
		int color)
{
	uint8_t xmin, xmax, ymin, ymax;
	int i;

	// check if the rectangle is to be filled
	if (fill == FILL)
	{
		// best way to create a filled rectangle is to define a drawing box
		// and loop two pixels at a time
		// calculate the min and max for x and y directions
		xmin = (x0 <= x1) ? x0 : x1;
		xmax = (x0 > x1) ? x0 : x1;
		ymin = (y0 <= y1) ? y0 : y1;
		ymax = (y0 > y1) ? y0 : y1;

		// specify the controller drawing box according to those limits
		// Row address set  (command 0x2B)
		avr_lcdSendCommand(PASET);
		avr_lcdSendData(xmin);
		avr_lcdSendData(xmax);
		// Column address set  (command 0x2A)
		avr_lcdSendCommand(CASET);
		avr_lcdSendData(ymin);
		avr_lcdSendData(ymax);

		// WRITE MEMORY
		avr_lcdSendCommand(RAMWR);

		// loop on total number of pixels / 2
		for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1); i++) {

			// use the color value to output three data bytes covering two pixels
			// SendLcd(LCDData,(color >> 4) & 0xFF );
			// SendLcd(LCDData,((color & 0xF) << 4) | ((color >> 8) & 0xF) );
			// avr_lcdSendData(color & 0xFF);
			avr_sspiSlaveSelectEnable();
			avr_lcdSendColor((color >> 4) & 0xFF);
			avr_lcdSendColor(((color & 0xF) << 4) | ((color >> 8) & 0xF));
			avr_lcdSendColor(color & 0xFF);
			avr_sspiSlaveSelectDisable();
		}

		avr_lcdSendCommand(NOP);
	} else {

		// best way to draw un unfilled rectangle is to draw four lines
		avr_lcdDrawLine(x0, y0, x1, y0, color);
		avr_lcdDrawLine(x0, y1, x1, y1, color);
		avr_lcdDrawLine(x0, y0, x0, y1, color);
		avr_lcdDrawLine(x1, y0, x1, y1, color);
	}
}

//  *************************************************************************************
//          LCD_Circle.c
//
//  Draws a line in the specified color at center (x0,y0) with radius
//
// Inputs:   x0     =   row address (0 .. 131)
//      y0     =   column address  (0 .. 131)
//      radius =   radius in pixels
//      color  =   12-bit color value  rrrrggggbbbb
//
//  Returns:   nothing
//
//  Author:    Jack Bresenham IBM, Winthrop University (Father of this algorithm, 1962)
//
//         Note: taken verbatim Wikipedia article on Bresenham's line algorithm
//          http://www.wikipedia.org
//
//  *************************************************************************************


void LCD_Circle(uint8_t x0, uint8_t y0, uint8_t radius,
		int color) {

	int f = 1 - radius;

	int ddF_x = 0;

	int ddF_y = -2 * radius;
	uint8_t x = 0;

	uint8_t y = radius;

	avr_lcdPutPixel(x0, y0 + radius, color);
	avr_lcdPutPixel(x0, y0 - radius, color);

	avr_lcdPutPixel(x0 + radius, y0, color);

	avr_lcdPutPixel(x0 - radius, y0, color);

	while (x < y) {

		if (f >= 0) {

			y--;
			ddF_y += 2;

			f += ddF_y;

		}

		x++;
		ddF_x += 2;

		f += ddF_x + 1;
		avr_lcdPutPixel(x0 + x, y0 + y, color);

		avr_lcdPutPixel(x0 - x, y0 + y, color);
		avr_lcdPutPixel(x0 + x, y0 - y, color);

		avr_lcdPutPixel(x0 - x, y0 - y, color);

		avr_lcdPutPixel(x0 + y, y0 + x, color);

		avr_lcdPutPixel(x0 - y, y0 + x, color);
		avr_lcdPutPixel(x0 + y, y0 - x, color);

		avr_lcdPutPixel(x0 - y, y0 - x, color);

	}
}

void LCD_Bitmap(
		uint8_t start_x, uint8_t start_y,
		uint8_t h_size, uint8_t v_size,
		uint8_t *bitmap_data)
{
	int i;
	uint8_t *pBitmap;
	// specify the controller drawing box according to those limits
	// Row address set  (command 0x2B)
	avr_lcdSendCommand(PASET);
	avr_lcdSendData(start_x);
	avr_lcdSendData(start_x + h_size - 1);
	// Column address set  (command 0x2A)
	avr_lcdSendCommand(CASET);
	avr_lcdSendData(start_y);
	avr_lcdSendData(start_y + v_size - 1);

	// WRITE MEMORY
	avr_lcdSendCommand(RAMWR);

	pBitmap = bitmap_data;

	// loop on total number of pixels / 2
	for (i = 0; i < (h_size * v_size) >> 1; i++) {
		uint8_t bitmap;

		avr_sspiSlaveSelectEnable();

		bitmap = pgm_read_byte(pBitmap++);
		avr_lcdSendColor(bitmap);
		bitmap = pgm_read_byte(pBitmap++);
		avr_lcdSendColor(bitmap);
		bitmap = pgm_read_byte(pBitmap++);
		avr_lcdSendColor(bitmap);
		avr_sspiSlaveSelectDisable();
	}
	avr_lcdSendCommand(NOP);
}

void LCD_Char(char c, uint8_t x, uint8_t y, int fColor, int bColor) {

	int i, j;

	uint8_t nCols;

	uint8_t nRows;

	uint8_t nBytes;
	uint8_t PixelRow;

	uint8_t Mask;

	unsigned int Word0;

	uint8_t *pFont, *pChar;

	pFont = (uint8_t *) FONT8x8;

	// get the nColumns, nRows and nBytes
	nCols = pgm_read_byte(pFont);

	nRows = pgm_read_byte(pFont + 1);

	nBytes = pgm_read_byte(pFont + 2);

	// get pointer to the last byte of the desired character

	pChar = pFont + (nBytes * (c - 0x1F));

	// Row address set  (command 0x2B)

	/*SendLcd(LCDCommand,PASET);

	 SendLcd(LCDData,x);

	 SendLcd(LCDData,x + nRows - 1);

	 Serial.print(x + nRows - 1);

	 // Column address set  (command 0x2A)

	 SendLcd(LCDCommand,CASET);

	 SendLcd(LCDData,y);
	 SendLcd(LCDData,y + nCols - 1);

	 Serial.print(y + nCols -1);

	 // WRITE MEMORY

	 SendLcd(LCDCommand,RAMWR);
	 */

	// loop on each row, working backwards from the bottom to the top

	for (i = 0; i < nRows; i++) {

		// copy pixel row from font table and then decrement row

		PixelRow = pgm_read_byte(pChar++);

		Mask = 0x80;

		for (j = 0; j < nCols; j += 1) {

			// if pixel bit set, use foreground color; else use the background color

			// now get the pixel color for two successive pixels

			if ((PixelRow & Mask) == 0)
				Word0 = bColor;

			else

				Word0 = fColor;

			avr_lcdPutPixel(y + j, x + i, Word0);

			Mask = Mask >> 1;
			/*     if ((PixelRow & Mask) == 0)

			 Word1 = bColor;

			 else
			 Word1 = fColor;

			 Mask = Mask >> 1;



			 // use this information to output three data bytes
			 SendLcd(LCDCommand,(Word0 >> 4) & 0xFF);

			 SendLcd(LCDCommand,((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));

			 SendLcd(LCDCommand,Word1 & 0xFF);
			 */
		}
	}

	// terminate the Write Memory command
	avr_lcdSendCommand(NOP);
}

void LCD_String(char *str, uint8_t x, uint8_t y, int fColor,
		int bColor) {

	// loop until null-terminator is seen

	while (*str != 0x00) {

		// draw the character

		LCD_Char(*str++, x, y, fColor, bColor);

		// advance the y position

		y = y + 8;
		if (x > 131)
			break;

	}
}
