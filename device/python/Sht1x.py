'''
Created on Oct 5, 2012

@author: Luca Nobili

This modules reads Humidity and Temperature from a Sensirion SHT1x sensor. I has been tested
both with an SHT11 and an SHT15.

It is meant to be used in a Raspberry Pi and depends on this module (http://code.google.com/p/raspberry-gpio-python/).

The module raspberry-gpio-python requires root privileges, therefore, to run this module you need to run your script as root.


Example Usage:

>>> from sht1x.Sht1x import Sht1x as SHT1x
>>> sht1x = SHT1x(11,7)
>>> sht1x.read_temperature_C()
25.22
>>> sht1x.read_humidity()     
52.6564216

'''
import traceback
import sys
import time
import logging
import math

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    import RPi.GPIO as GPIO
except ImportError:
    logger.warning("Could not import the RPi.GPIO package (http://pypi.python.org/pypi/RPi.GPIO). Using a mock instead. Notice that this is useful only for the purpose of debugging this module, but will not give the end user any useful result.")
    import RPiMock.GPIO as GPIO
except:
    logger.warning("Could not import the RPi.GPIO package (http://pypi.python.org/pypi/RPi.GPIO). Using a mock instead. Notice that this is useful only for the purpose of debugging this module, but will not give the end user any useful result.")
    import RPiMock.GPIO as GPIO
    traceback.print_exc(file=sys.stdout)
 
#   Conversion coefficients from SHT15 datasheet
D1 = -40.0  # for 14 Bit @ 5V
D2 =  0.01 # for 14 Bit DEGC

C1 = -2.0468       # for 12 Bit
C2 =  0.0367       # for 12 Bit
C3 = -0.0000015955 # for 12 Bit
T1 =  0.01      # for 14 Bit @ 5V
T2 =  0.00008   # for 14 Bit @ 5V
    
class Sht1x(object):
    GPIO_BOARD = GPIO.BOARD
    GPIO_BCM = GPIO.BCM

    def __init__(self, dataPin, sckPin, gpioMode = GPIO_BOARD):
        self.dataPin = dataPin
        self.sckPin = sckPin
        GPIO.setmode(gpioMode)
        
#    I deliberately will not implement read_temperature_F because I believe in the
#    in the Metric System (http://en.wikipedia.org/wiki/Metric_system)

    def read_temperature_C(self):
        temperatureCommand = 0b00000011

        self.__sendCommand(temperatureCommand)
        self.__waitForResult()
        rawTemperature = self.__getData16Bit()
        self.__skipCrc()
#        GPIO.cleanup() # https://www.raspberrypi.org/forums/viewtopic.php?f=32&t=100684

        return rawTemperature * D2 + D1
        

    def read_humidity(self):
#        Get current temperature for humidity correction
        temperature = self.read_temperature_C()
        return self._read_humidity(temperature)
    
    def _read_humidity(self, temperature):
        humidityCommand = 0b00000101
        self.__sendCommand(humidityCommand)
        self.__waitForResult()
        rawHumidity = self.__getData16Bit()
        self.__skipCrc()
#        GPIO.cleanup()
#        Apply linear conversion to raw value
        linearHumidity = C1 + C2 * rawHumidity + C3 * rawHumidity * rawHumidity
#        Correct humidity value for current temperature
        return (temperature - 25.0 ) * (T1 + T2 * rawHumidity) + linearHumidity            

    def calculate_dew_point(self, temperature, humidity):
        if temperature > 0:
            tn = 243.12
            m = 17.62
        else:
            tn = 272.62
            m = 22.46
        return tn * (math.log(humidity / 100.0) + (m * temperature) / (tn + temperature)) / (m - math.log(humidity / 100.0) - m * temperature / (tn + temperature))

    def __sendCommand(self, command):
        #Transmission start
        GPIO.setup(self.dataPin, GPIO.OUT)
        GPIO.setup(self.sckPin, GPIO.OUT)
                
        GPIO.output(self.dataPin, GPIO.HIGH)
        self.__clockTick(GPIO.HIGH)
        GPIO.output(self.dataPin, GPIO.LOW)
        self.__clockTick(GPIO.LOW)
        self.__clockTick(GPIO.HIGH)
        GPIO.output(self.dataPin, GPIO.HIGH)
        self.__clockTick(GPIO.LOW)

        for i in range(8):
            GPIO.output(self.dataPin, command & (1 << 7 - i))
            self.__clockTick(GPIO.HIGH)
            self.__clockTick(GPIO.LOW)     
        
        self.__clockTick(GPIO.HIGH)
        
        GPIO.setup(self.dataPin, GPIO.IN)
        
        ack = GPIO.input(self.dataPin)
        logger.debug("ack1: %s", ack)
        if ack != GPIO.LOW:
            logger.error("nack1")
        
        self.__clockTick(GPIO.LOW)
        
        ack = GPIO.input(self.dataPin)
        logger.debug("ack2: %s", ack)
        if ack != GPIO.HIGH:
            logger.error("nack2")        
            
    def __clockTick(self, value):
        GPIO.output(self.sckPin, value)
#       100 nanoseconds 
        time.sleep(.0000001)
        
    def __waitForResult(self):
        GPIO.setup(self.dataPin, GPIO.IN)

        for i in range(100):
#            10 milliseconds
            time.sleep(.01)
            ack = GPIO.input(self.dataPin)
            if ack == GPIO.LOW:
                break
        if ack == GPIO.HIGH:
            raise SystemError
            
    def __getData16Bit(self):
        GPIO.setup(self.dataPin, GPIO.IN)
        GPIO.setup(self.sckPin, GPIO.OUT)
#        Get the most significant bits
        value = self.__shiftIn(8)
        value *= 256
#        Send the required ack
        GPIO.setup(self.dataPin, GPIO.OUT)
        GPIO.output(self.dataPin, GPIO.HIGH)
        GPIO.output(self.dataPin, GPIO.LOW)
        self.__clockTick(GPIO.HIGH)
        self.__clockTick(GPIO.LOW)
#        Get the least significant bits
        GPIO.setup(self.dataPin, GPIO.IN)
        value |= self.__shiftIn(8)
        
        return value
    
    def __shiftIn(self, bitNum):
        value = 0
        for i in range(bitNum):
            self.__clockTick(GPIO.HIGH)
            value = value * 2 + GPIO.input(self.dataPin)
            self.__clockTick(GPIO.LOW)
        return value
     
    def __skipCrc(self):
#        Skip acknowledge to end trans (no CRC)
        GPIO.setup(self.dataPin, GPIO.OUT)
        GPIO.setup(self.sckPin, GPIO.OUT)
        GPIO.output(self.dataPin, GPIO.HIGH)
        self.__clockTick(GPIO.HIGH)
        self.__clockTick(GPIO.LOW)
    
    def __connectionReset(self):
        GPIO.setup(self.dataPin, GPIO.OUT)
        GPIO.setup(self.sckPin, GPIO.OUT)
        GPIO.output(self.dataPin, GPIO.HIGH)
        for i in range(10):
            self.__clockTick(GPIO.HIGH)
            self.__clockTick(GPIO.LOW)

class WaitingSht1x(Sht1x):
    def __init__(self, dataPin, sckPin):
        super(WaitingSht1x, self).__init__(dataPin, sckPin)
        self.__lastInvocationTime = 0

    def read_temperature_C(self):
        self.__wait()        
        return super(WaitingSht1x, self).read_temperature_C()
    
    def read_humidity(self):
        temperature = self.read_temperature_C()
        self.__wait()
        return super(WaitingSht1x, self)._read_humidity(temperature)
    
    def read_temperature_and_Humidity(self):
        temperature = self.read_temperature_C()
        self.__wait()
        humidity = super(WaitingSht1x, self)._read_humidity(temperature)
        return (temperature, humidity)
            
    def __wait(self):
        lastInvocationDelta = time.time() - self.__lastInvocationTime
#        if we queried the sensor less then a second ago, wait until a second is passed
        if lastInvocationDelta < 1:
            time.sleep(1 - lastInvocationDelta)
        self.__lastInvocationTime = time.time()
        
def main():
    sht1x = WaitingSht1x(11, 7)
    print(sht1x.read_temperature_C())
    print(sht1x.read_humidity())
    aTouple = sht1x.read_temperature_and_Humidity()
    print("Temperature: {} Humidity: {}".format(aTouple[0], aTouple[1]))
    print(sht1x.calculate_dew_point(20, 50))
    
if __name__ == '__main__':
    main()
