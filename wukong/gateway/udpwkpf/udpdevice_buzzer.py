from udpwkpf import WuClass, Device
import sys
import mraa
from twisted.internet import reactor

if __name__ == "__main__":
    class Buzzer(WuClass):
        def __init__(self, pin):
            self.ID = 2004
            self.buzzer_pwm = mraa.Pwm(pin)
            print "Buzzer init success"

        def update(self,obj,pID,val):
            on_off = obj.getProperty(0)
            freq = obj.getProperty(1)
            freq = freq if freq > 0 else 1
            duty_cycle = obj.getProperty(2)
 
            usecPeriod = 1000000/freq
            duty_cycle_ratio = (duty_cycle/100.0)

            if pID == 0:
                if on_off == True:
                    self.buzzer_pwm.enable(True)
                    self.buzzer_pwm.period_us(usecPeriod)
                    self.buzzer_pwm.write(duty_cycle_ratio)
                    print "Buzzer state: %r, Freq: %d Hz, Duty_cycle_ratio: %.2f" % (on_off, freq, duty_cycle_ratio) 
                else:
                    # force it to stop
                    self.buzzer_pwm.period_us(0)
                    self.buzzer_pwm.write(0)
                    self.buzzer_pwm.enable(False)
                    self.buzzer_pwm.enable(True)
                    self.buzzer_pwm.period_us(0)
                    self.buzzer_pwm.write(0)
                    self.buzzer_pwm.enable(False)
                    print "Buzzer off"
            elif pID == 1:
                self.buzzer_pwm.period_us(usecPeriod)
                print "Buzzer state: %r, Freq: %d Hz, Duty_cycle_ratio: %.2f" % (on_off, freq, duty_cycle_ratio) 
            elif pID == 2:
                self.buzzer_pwm.write(duty_cycle_ratio)
                print "Buzzer state: %r, Freq: %d Hz, Duty_cycle_ratio: %.2f" % (on_off, freq, duty_cycle_ratio) 
            else:
                print "Buzzer garbage message"

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            Buzzer_Pin = 6 
            cls = Buzzer(Buzzer_Pin)
            self.addClass(cls,0)
            self.obj_buzzer = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
