from twisted.web.client import FileBodyProducer
from twisted.protocols import basic
from twisted.internet import reactor
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

from udpwkpf import WuClass, Device
import sys
import json

class AWS_Publisher(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('AWS_Publisher')
        self.id = 0
        self.myMQTTClient = AWSIoTMQTTClient("")
        self.myMQTTClient.configureEndpoint("a1trumz0n7avwt.iot.us-west-2.amazonaws.com", 8883)
        self.myMQTTClient.configureCredentials("AWS/root.crt", "AWS/private.key", "AWS/cert.crt")
        self.myMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
        self.myMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
        self.myMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
        self.myMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec
        self.myMQTTClient.connect()
        print "aws init success"

    def update(self,obj,pID=None,val=None):
        print "location", obj.getLocation()
        location = (int)(obj.getProperty(1))
        val = (int)(obj.getProperty(0))
        if location:
            print "change fire state at", location, "to", val
            mess = json.dumps({'fire': val, 'location': location})
            self.myMQTTClient.publish('fireMessage', mess, 0)

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            self.m = AWS_Publisher()
            self.addClass(self.m,0)
            self.addObject(self.m.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
