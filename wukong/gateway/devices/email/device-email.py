from udpwkpf import WuClass,Device
from twisted.internet import reactor
from twisted.web import resource,static,server
from autobahn.twisted.websocket import WebSocketServerFactory,WebSocketServerProtocol
from autobahn.twisted.resource import WebSocketResource,HTTPChannelHixie76Aware
import __builtin__
import cjson
import smtplib
from email.mime.text import MIMEText
global site

class EMail(WuClass):
    PICTURE=0
    def __init__(self):
        self.loadClass('Email')
    def update(self,obj,pID,value):
        print "send email" 
        f = open('email.txt')
        fields = f.readline().split(',')
        msg = MIMEText(f.read())
        f.close()
        msg['Subject'] = fields[0]
        msg['From'] = fields[1]
        msg['To'] = fields[2]

        s = smtplib.SMTP(fields[3])
        s.set_debuglevel(True)
        print fields
        s.login(fields[1], fields[4])
        try:
            s.sendmail(fields[1], [fields[2]], msg.as_string())
        except:
            traceback.print_exc()
        finally:
            s.close()
        pass
    def init(self):
        pass
class EMailDevice(Device):
    def __init__(self):
        Device.__init__(self,'127.0.0.1','127.0.0.1:4002')
    def init(self):
        cls = EMail()
        self.addClass(cls,0)
        self.obj = self.addObject(cls.ID)

d = EMailDevice()
root=static.File("./www")
site=server.Site(root)
site.device = d
reactor.listenTCP(11004,site)
__builtin__.site = site
reactor.run()
