import re, os, socket
from threading import Thread
import serial
import netifaces as ni

class TestDevice:
    def __init__(self):
        pass

    def download(self):
        pass

    def deviceLearn(self):
        pass

    def deviceReset(self):
        pass

    def waitDeviceReady(self, timeout=20):
        pass

    def startLog(self):
        pass

    def stopLog(self):
        pass



class WuDevice_Zwave(TestDevice):
    def __init__(self, binary, usbport):
        TestDevice.__init__(self)
        self.binary = binary
        self.usbport = usbport
        self.node_id = None # Will be set from test_environment_device.py, which is a bit ugly
        self.console = serial.Serial(self.usbport, baudrate=115200)
        self.console.timeout = 1

    def download(self):
        os.system("avrdude -p atmega2560 -c wiring -P %s -U flash:w:%s" % (self.usbport, self.binary))

    def deviceLearn(self):
        self.console.write("$l")

    def deviceReset(self):
        self.console.write("$r")

    def waitDeviceReady(self, timeout=20):
        while timeout > 0:
            timeout = timeout - 1
            l = self.console.readline()
            if l.find("ready") != -1: break

    def startLog(self):
        def func(self, console, filename):
            f = open(filename, 'w')

            while not self.log_done:
                s = console.readline()
                f.write(s)
                f.flush()
            f.close()
        filename = 'reports/node_%d.txt' % (self.node_id)
        self.log_done = False
        self.log_thread = Thread(target=func, args=(self, self.console, filename))
        self.log_thread.start()

    def stopLog(self):
        time.sleep(1)
        self.log_done = True
        self.log_thread.join() # Not sure why this is necessary, but this code is just copied from test_environment_device.py



class Galileo_NetworkServer(TestDevice):
    def getMyIP(self):
        if 'eth0' in ni.interfaces():
            return ni.ifaddresses('eth0')[2][0]['addr']
        elif 'en0' in ni.interfaces():
            return ni.ifaddresses('en0')[2][0]['addr']
        else:
            raise Error('Cant determine IP address')

    def __init__(self, binary, ipaddress):
        TestDevice.__init__(self)
        self.binary = binary
        self.ipaddress = ipaddress
        self.node_id = int(re.findall(r'\b25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?\.25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?\.25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?\.25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?\b', ipaddress)[3])

    def download(self):
        os.system("ssh root@%s 'rm -rf /darjeeling'" % (self.ipaddress))
        os.system("ssh root@%s 'mkdir /darjeeling'" % (self.ipaddress))
        os.system("scp -r %s/darjeeling.elf %s/*.dja %s/install_service.sh %s/service root@%s:/darjeeling" % (self.binary, self.binary, self.binary, self.binary, self.ipaddress))
        local_networkserver_ip = self.getMyIP()
        print local_networkserver_ip
        os.system("ssh root@%s 'cd /darjeeling; ./install_service.sh \"-i %s    -s %s\"'" % (self.ipaddress, self.node_id, local_networkserver_ip))

    def deviceLearn(self):
        pass

    def deviceReset(self):
        pass

    def waitDeviceReady(self, timeout=20):
        pass

    def startLog(self):
        pass

    def stopLog(self):
        filename = 'reports/node_%d.txt' % (self.node_id)
        os.system("scp -r root@%s:/var/log/darjeeling.log %s" % (self.ipaddress, filename))

