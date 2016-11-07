DEVICE_TYPE_MRAA = 0 # Edison, Galileo
DEVICE_TYPE_RPI  = 1 # Raspberry Pi
DEVICE_TYPE_GPI  = 2 # Grove Pi

PIN_TYPE_DIGITAL = 0
PIN_TYPE_ANALOG  = 1
PIN_TYPE_I2C     = 2

PIN_MODE_INPUT   = 0
PIN_MODE_OUTPUT  = 1

device_type = DEVICE_TYPE_GPI

if device_type == DEVICE_TYPE_MRAA:
    import mraa
    import pyupm_grove
    import pyupm_i2clcd as lcd
    import pyupm_lpd8806
elif device_type == DEVICE_TYPE_RPI:
    import RPi.GPIO as GPIO
    GPIO.setmode(GPIO.BOARD)
    import grove_rgb_lcd as lcd
    from Sht1x import Sht1x
    import sys
    sys.path.append("./RPi-LPD8806")
    from bootstrap import *
elif device_type == DEVICE_TYPE_GPI:
    import grovepi
    import sys
    sys.path.append("./RPi-LPD8806")
    from bootstrap import *
else:
    raise NotImplementedError


def pin_mode(pin, pin_type = PIN_TYPE_DIGITAL, pin_mode = PIN_MODE_INPUT, **kwargs):
    if device_type == DEVICE_TYPE_MRAA:
        if pin_type == PIN_TYPE_DIGITAL:
            if pin_mode == PIN_MODE_INPUT:
                obj = mraa.Gpio(pin)
                obj.dir(mraa.DIR_IN)
                return obj

            elif pin_mode == PIN_MODE_OUTPUT:
                obj = mraa.Gpio(pin)
                obj.dir(mraa.DIR_OUT)
                return obj

        elif pin_type == PIN_TYPE_ANALOG:
            obj = mraa.Aio(pin)
            return obj

        elif pin_type == PIN_TYPE_I2C:
            pass

    elif device_type == DEVICE_TYPE_RPI:
        if pin_type == PIN_TYPE_DIGITAL:
            if pin_mode == PIN_MODE_INPUT:
                GPIO.setup(pin, GPIO.IN, **kwargs)
                return pin

            elif pin_mode == PIN_MODE_OUTPUT:
                GPIO.setup(pin, GPIO.OUT, **kwargs)
                return pin

        else:
            pass

    elif device_type == DEVICE_TYPE_GPI:
        if pin_type == PIN_TYPE_DIGITAL:
            if pin_mode == PIN_MODE_INPUT:
                grovepi.pinMode(pin, "INPUT")
                return pin

            elif pin_mode == PIN_MODE_OUTPUT:
                grovepi.pinMode(pin, "OUTPUT")
                return pin

        elif pin_type == PIN_TYPE_ANALOG:
            grovepi.pinMode(pin, "INPUT")
            return pin
           
        else:
            pass

    raise NotImplementedError


def digital_read(pin_obj):
    if device_type == DEVICE_TYPE_MRAA:
        return pin_obj.read()

    elif device_type == DEVICE_TYPE_RPI:
        return GPIO.input(pin_obj)

    elif device_type == DEVICE_TYPE_GPI:
        return grovepi.digitalRead(pin_obj)

def digital_write(pin_obj, val):
    if device_type == DEVICE_TYPE_MRAA:
        pin_obj.write(val)

    elif device_type == DEVICE_TYPE_RPI:
        return GPIO.output(pin_obj, val)

    elif device_type == DEVICE_TYPE_GPI:
        return grovepi.digitalWrite(pin_obj, val)

def analog_read(pin_obj):
    if device_type == DEVICE_TYPE_MRAA:
        return pin_obj.read()

    elif device_type == DEVICE_TYPE_RPI:
        raise NotImplementedError

    elif device_type == DEVICE_TYPE_GPI:
        return grovepi.analogRead(pin_obj)

def grove_lcd(text_addr, rgb_addr):
    if device_type == DEVICE_TYPE_MRAA:
        return lcd.Jhd1313m1(0,text_addr,rgb_addr)
    elif device_type == DEVICE_TYPE_RPI:
        return lcd
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def grove_set_color(lcd, red, green, blue):
    if device_type == DEVICE_TYPE_MRAA:
        lcd.setColor(red, green, blue)
    elif device_type == DEVICE_TYPE_RPI:
        lcd.setRGB(red, green, blue)
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def grove_set_text(lcd, text):
    if device_type == DEVICE_TYPE_MRAA:
         lcd.setCursor(0,0)
         lcd.write(str(text))
    elif device_type == DEVICE_TYPE_RPI:
         lcd.setText(str(text))
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def grove_clear(lcd):
    if device_type == DEVICE_TYPE_MRAA:
         lcd.clear()
    elif device_type == DEVICE_TYPE_RPI:
         lcd.textCommand(0x01)
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def sht1x(dataPin, clkPin):
    if device_type == DEVICE_TYPE_MRAA:
        raise NotImplementedError
    elif device_type == DEVICE_TYPE_RPI:
        return Sht1x(dataPin, clkPin, Sht1x.GPIO_BOARD)
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def sht1x_read_temperature(temp_humid_obj):
    if device_type == DEVICE_TYPE_MRAA:
        raise NotImplementedError
    elif device_type == DEVICE_TYPE_RPI:
        return temp_humid_obj.read_temperature_C()
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError
    
def sht1x_read_humidity(temp_humid_obj):
    if device_type == DEVICE_TYPE_MRAA:
        raise NotImplementedError
    elif device_type == DEVICE_TYPE_RPI: 
        return temp_humid_obj.read_humidity()
    elif device_type == DEVICE_TYPE_GPI:
        raise NotImplementedError

def analog_write(pin_obj, val):
    if device_type == DEVICE_TYPE_MRAA:
        raise NotImplementedError

    elif device_type == DEVICE_TYPE_RPI:
        raise NotImplementedError

    elif device_type == DEVICE_TYPE_GPI:
        return grovepi.analogWrite(pin_obj, val)

def temp_read(pin_obj):
    if device_type == DEVICE_TYPE_MRAA:
        return pyupm_grove.GroveTemp(pin_obj).value()

    elif device_type == DEVICE_TYPE_RPI:
        raise NotImplementedError

    elif device_type == DEVICE_TYPE_GPI:
        return grovepi.temp(pin_obj,'1.1')

def device_cleanup():
    if device_type == DEVICE_TYPE_RPI:
        GPIO.cleanup()
    else:
        pass
