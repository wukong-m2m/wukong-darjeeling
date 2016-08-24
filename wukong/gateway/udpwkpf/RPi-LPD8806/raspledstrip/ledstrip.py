#!/usr/bin/env python
from color import Color, ColorHSV
from LPD8806 import LPD8806

#Not all LPD8806 strands are created equal.
#Some, like Adafruit's use GRB order and the other common order is GRB
#Library defaults to GRB but you can call strand.setChannelOrder(ChannelOrder)
#to set the order your strands use
class ChannelOrder:
    RGB = [0,1,2] #Probably not used, here for clarity

    GRB = [1,0,2] #Strands from Adafruit and some others (default)
    BRG = [1,2,0] #Strands from many other manufacturers


class LEDStrip:

    def __init__(self, leds, use_py_spi = False, dev="/dev/spidev0.0", driver="LPD8806"):
        #Variables:
        #	leds -- strand size
        #	dev -- spi device

        #no alternate drivers for now. Here so they can be added later
        self.driver = LPD8806(leds, use_py_spi, dev)

        self.c_order = ChannelOrder.GRB
        self.leds = leds
        self.lastIndex = self.leds - 1
        self.gamma = bytearray(256)
        self.buffer = [0 for x in range(self.leds + 1)]

        self.masterBrightness = 1.0

        for led in range(self.leds):
            self.buffer[led] = bytearray(3)
        for i in range(256):
            # Color calculations from
            # http://learn.adafruit.com/light-painting-with-raspberry-pi
            self.gamma[i] = 0x80 | int(
                pow(float(i) / 255.0, 2.5) * 127.0 + 0.5
            )

    def update(self):
        self.driver.update(self.buffer)

    #Allows for easily using LED strands with different channel orders
    def setChannelOrder(self, order):
        self.c_order = order

    #Set the master brightness for the LEDs 0.0 - 1.0
    def setMasterBrightness(self, bright):
        if(bright > 1.0 or bright < 0.0):
            raise ValueError('Brightness must be between 0.0 and 1.0')
        self.masterBrightness = bright

    #Fill the strand (or a subset) with a single color using a Color object
    def fill(self, color, start=0, end=-1):
        if start < 0:
            start = 0
        if end < 0 or end > self.lastIndex:
            end = self.lastIndex
        for led in range(start, end + 1): #since 0-index include end in range
            self.__set_internal(led, color)

    #Fill the strand (or a subset) with a single color using RGB values
    def fillRGB(self, r, g, b, start=0, end=-1):
        self.fill(Color(r, g, b), start, end)

    #Fill the strand (or a subset) with a single color using HSV values
    def fillHSV(self, h, s, v, start=0, end=-1):
        self.fill(ColorHSV(h, s, v).get_color_rgb(), start, end)

    #Fill the strand (or a subset) with a single color using a Hue value.
    #Saturation and Value components of HSV are set to max.
    def fillHue(self, hue, start=0, end=-1):
        self.fill(ColorHSV(hue).get_color_rgb(), start, end)

    def fillOff(self, start=0, end=-1):
        self.fillRGB(0, 0, 0, start, end)

    #internal use only. sets pixel color
    def __set_internal(self, pixel, color):
        if(pixel < 0 or pixel > self.lastIndex):
            return; #don't go out of bounds

        self.buffer[pixel][self.c_order[0]] = self.gamma[int(color.r * self.masterBrightness)]
        self.buffer[pixel][self.c_order[1]] = self.gamma[int(color.g * self.masterBrightness)]
        self.buffer[pixel][self.c_order[2]] = self.gamma[int(color.b * self.masterBrightness)]

    #Set single pixel to Color value
    def set(self, pixel, color):
        self.__set_internal(pixel, color)

    #Set single pixel to RGB value
    def setRGB(self, pixel, r, g, b):
        color = Color(r, g, b)
        self.set(pixel, color)

    #Set single pixel to HSV value
    def setHSV(self, pixel, h, s, v):
        self.set(pixel, ColorHSV(h, s, v).get_color_rgb())

    #Set single pixel to Hue value.
    #Saturation and Value components of HSV are set to max.
    def setHue(self, pixel, hue):
        self.set(pixel, ColorHSV(hue).get_color_rgb())

    #turns off the desired pixel
    def setOff(self, pixel):
        self.setRGB(pixel, 0, 0, 0)

    #Turn all LEDs off.
    def all_off(self):
        self.fillOff()
        self.update()
        self.fillOff()
        self.update()
