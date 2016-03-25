import sys,os,time,cjson
sys.path.insert(0,os.path.abspath(os.path.join(os.path.dirname(__file__),'..')))
from udpwkpf import WuClass,Device
#from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor
#from twisted.web import resource,static,server
from twisted.internet.protocol import Factory,Protocol
from twisted.internet.endpoints import TCP4ServerEndpoint

def debug(*s,**kw):
    newline= kw.get('newline','\n')
    cols = [str(x) for x in s]
    line = '\t'.join(cols).encode('utf-8')+newline
    sys.stderr.write(line)

"""
class UIButton(WuClass):
    def __init__(self):
        self.loadClass('UIButton')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass

class UISlider(WuClass):
    def __init__(self):
        self.loadClass('UISlider')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass
class UIScene(WuClass):
    def __init__(self):
        self.loadClass('UIScene')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass
class HsremoteDevice(Device):
    def __init__(self,gtwip,selfip):
        Device.__init__(self,gtwip,selfip)
    def init(self):
        print "xxxxxx"
        cls = UIButton()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = UISlider()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = UIScene()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        self.ID_slider = cls.getWuClassID('UISlider')
        self.ID_scene = cls.getWuClassID('UIScene')
        pass
    def update(self,obj,pID,val):
        pass
    def findSlider(self,p,d):
        for i in range(0,len(self.objects)):
            obj = self.objects[i]
            if obj.cls.ID == self.ID_slider:
                print obj.port,obj.getProperty(1),obj.getProperty(2)
                if obj.getProperty(1) == p and obj.getProperty(2) == d:
                    return obj
        return None
    def findScene(self,p,d):
        for i in range(0,len(self.objects)):
            obj = self.objects[i]
            if obj.cls.ID == self.ID_scene:
                if obj.getProperty(1) == p and obj.getProperty(2) == d:
                    return obj
        return None

class SearchProtocol(DatagramProtocol):
    def datagramReceived(self, data, (host, port)):
        # \x01\x00\x00\xc0\x00\x00\x00\x00\x00\x01\xff\xff
        net = netifaces.ifaddresses(myinterface)
        mac = net[netifaces.AF_LINK]
        ip = map(int,net[netifaces.AF_INET][0]['addr'].split('.'))
        print ip
        mac = netifaces.ifaddresses(myinterface)[netifaces.AF_LINK][0]['addr'].split(':')
        print mac
        
        header = struct.pack("hBBIhh6B",1,0,0xc0,0,1,2,int(mac[0],16),int(mac[1],16),int(mac[2],16),int(mac[3],16),int(mac[4],16),int(mac[5],16))
        data = header+struct.pack("4BII", ip[0],ip[1],ip[2],ip[3],0,0)+"wukong"+"\0"*14
        self.transport.write(data,(host,port))
        print map(ord, data)
        print "receive data %r from %s:%d" % (data,host,port)
class Cgi(Protocol):
    isLeaf = True
    def __init__(self,factory):
        self.factory = factory
    def dataReceived(self,request):
        print request
        # /cgi-bin/proxy?cmd=/scene&p=1&d=113&serialno=,65535
        # GET /cgi-bin/proxy?cmd=/scene&p=1&d=113&serialno=,65535 HTTP/1.
        lines = request.split('\n')
        print lines[0]
        tokens = lines[0].split(' ')
        ulist = tokens[1].split('?')
        if len(ulist) >= 2:
            paths = ulist[0].split('/')
            pars = ulist[1].split('&')
            query = {}
            for p in pars:
                ff = p.split('=')
                if len(ff) != 2:
                    return self.render_none()
                query[ff[0]] = ff[1]
        else:
            paths = ulist[0].split('/')
            query={}
        print paths
        if tokens[0] == 'GET':
            if paths[1] == 'cgi-bin':
                if paths[2] == 'proxy':
                    print query
                    if query['cmd'] == '/scene':
                        self.render_scene(query)
                    elif query['cmd'][0:2] == '/r':
                        self.render_control(query['cmd'])
                    else:
                        self.render_none()
                else:
                    self.render_none()
            else:
                self.render_none()
        elif tokens[0] == 'POST':
            print 'POST method',paths
            if paths[1] == 'cgi-bin':
                if paths[2] == 'ssidlist':
                    print lines
                    for i in range(len(lines)):
                        if lines[i] == '' or lines[i] == '\r':
                            try:
                                obj = cjson.decode('\n'.join(lines[i+1:]))
                                print obj
                            except:
                                pass
            self.render_none()

    def doCommand_clear(self):
        if self.clear_obj:
            self.clear_obj.setProperty(0,7)
            self.clear_obj.setProperty(3,False)
            self.clear_obj = None
    def doCommand(self,d,p,v):
        obj = self.factory.device.findSlider(d,p)
        if obj:
            if v == 30: #up
                obj.setProperty(0,3)
            elif v == 31: #down
                obj.setProperty(0,4)
            elif v == 32: #left
                obj.setProperty(0,1)
            elif v == 33: #right
                obj.setProperty(0,2)
            elif v == 19: #O
                obj.setProperty(0,5)
            elif v == 21: #X
                obj.setProperty(0,6)
            elif v == 20: #Stop
                obj.setProperty(0,0)
            elif v == 39: #change mode
                obj.setProperty(3,True)
            if d == 11 or d == 12 or d == 13 or  d == 14:
                print "CMD: %d reset" % (d)
                self.clear_obj  = obj
                reactor.callLater(0.5, self.doCommand_clear)
    def doScene(self,s):
        obj = self.factory.device.findScene(1,s)
        if obj: obj.setProperty(0,v)
    def render_control(self,cmd):
        cmd = cmd[2:].split(',')
        print ' command %0x' % int(cmd[0])
        arg = int(cmd[0])
        v = (arg >> 16)&0xff
        d = (arg>>8)&0xff
        p = arg & 0xff
        print "CMD (%d,%d) = %d" % (d,p,v)
        self.doCommand(d,p,v)
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 0\n\n')
        self.transport.loseConnection()

    def render_scene(self,query):
        d = int(query['d'])
        p = int(query['p'])
        print 'render_scene'
        if p == 1 and (d>=113 & d <= 117):
            s = d-113+1
            print 'Do scene %d' % s
            self.doScene(s)
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 0\n\n')
        self.transport.loseConnection()

    def render_none(self):
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 1\n\n0')
        self.transport.loseConnection()
class CgiFactory(Factory):
    def __init__(self,device):
        self.device = device
    def buildProtocol(self, addr):
        return Cgi(self)
"""
class MessageQueue(object):
    def __init__(self):
        self.out_queue = []
        self.in_queue = []
        self.listeners = {}
        self._timer = None
        self._message_ttl = 3.0 ## 3 seconds
    def messageOut(self,obj,pID,val):
        signalType = {
            1:'boolean',
            2:'short',
            3:'string',
        }
        message = {
            'payload':{
                'topic':getattr(obj.cls,'topic'),
                'type':signalType[pID],
                'value':val
            },
            'ts':time.time(),
            'ack_count':0
        }
        #debug('add out message',message['payload'])
        self.out_queue.append(message)
        if self._timer is None:
            self._timer = reactor.callLater(0.5,self._notify)
    def _notify(self):
        self._timer = None

        ## do cleanup of the out_queue
        now = time.time()
        item_tobe_remove = []
        for item in self.out_queue:
            
            topic = item['payload']['topic']
            
            try:
                listeners = self.listeners[topic]
            except KeyError:
                listeners = []
                
            for listener in listeners:
                listener(item)
            
            if now - item['ts'] > self._message_ttl:
                item_tobe_remove.append(item)
                #debug('drop out message ',item['payload'])
            elif item['ack_count'] >= len(listeners):
                item_tobe_remove.append(item)
                #debug('cleanup out message',item['payload'])

        item_tobe_remove.reverse()
        for item in item_tobe_remove:
            self.out_queue.remove(item)
        #debug('out message length',len(self.out_queue),'listener number',len(self.listeners))

    def addListener(self,topic,_callable):
        try:
            self.listeners[topic].append(_callable)
        except KeyError:
            self.listeners[topic] = [_callable]
    def removeListener(self,_callable):
        for topic,callables in self.listeners.iteritems():
            if _callable in callables:
                callables.remove(_callable)

global message_queue
message_queue = MessageQueue()

class NodeREDSignalSender(WuClass):
    """
    Receive signal from WuKong, then pass to NodeRED
    """
    def __init__(self):
        print 'NodeREDSignalSender __init__()'
        WuClass.__init__(self)
        self.loadClass('NodeREDSignalSender')
        
    def update(self,obj,pID,val):
        #print 'NodeREDSignalSender update',[obj,pID,val]
        if pID==0:
            #
            # This is setting the topic, doesn't need to send to remote, 
            #
            setattr(obj.cls,'topic',val)
        else:
            global message_queue
            message_queue.messageOut(obj,pID,val)

    def init(self):
        print 'NodeREDSignalSender init'
    def __repr__(self):
        return '<NodeREDSignalSender>'

class NodeREDSignalReceiver(WuClass):
    """
    Receive signal from NodeRED, then pass to WuKong
    """
    def __init__(self):
        WuClass.__init__(self)
        print 'NodeREDSignalReceiver __init__()'
        self.loadClass('NodeREDSignalReceiver')
    def update(self,obj,pID,val):
        print 'NodeREDSignalReceiver update',[obj,pID,val]
    def init(self):
        print 'NodeREDSignalReceiver init'
    def __repr__(self):
        return '<NodeREDSignalReceiver>'

class NodeREDBroker(Device):
    def __init__(self,gtwip,selfip):
        Device.__init__(self,gtwip,selfip)
    def init(self):
        print "NodeRedBroker init()"
        cls = NodeREDSignalReceiver()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = NodeREDSignalSender()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
    def update(self,obj,pID,val):
        print 'NodeREDBroker.update()',[obj,pID,val]

def parseArguments():
    from optparse import OptionParser,OptionGroup
    parser = OptionParser(usage="""
  $python %prog -g 192.168.56.101 -p 192.168.56.101:5100
  or,
  $sh run.sh -g 192.168.56.101 -p 192.168.56.101:5100
    """,
    description="This is the NodeRED Broker for WuKong",
    )
    parser.add_option('-g','--gateway',
        help='IP of the gateway')
    parser.add_option('-p','--ip',
        help='ip:port of this device, ex. -p 192.168.56.101:5100')
    parser.add_option('-d','--develop',action="store_true",
        help='use developing paramters: -g 192.168.56.101 -p 192.168.56.101:5100')

    (options, args) = parser.parse_args()

    return options,args,parser

#
# Communication to NodeRED
#
class WuKongNodeREDProtocol(Protocol):
    boundary = '||'
    def __init__(self):
        self.last_message_out_ts = 0
        self.last_message_in_ts = 0
        self.subscribed_topic = None
        self.data = ''
    def dataReceived(self, data):
        #self.transport.write(data)
        self.data += data
        chunks = self.data.split(WuKongNodeREDProtocol.boundary)
        residue = []
        if len(chunks) > 1:
            global message_queue
            for chunk in chunks:
                if not chunk: continue
                try:
                    obj = cjson.decode(chunk)
                    debug('message in %s' % obj)
                    
                    if obj.has_key('subscribe') and ((self.subscribed_topic is None) or (self.subscribed_topic !=obj['subscribe'])):
                        global message_queue
                        debug('subscribe topic %s' % obj['topic'])
                        if self.subscribed_topic is not None:
                            message_queue.removeListener(self.onMessageUpdated)
                        self.subscribed_topic = obj['topic']
                        message_queue.addListener(obj['topic'],self.onMessageUpdated)
                    ##
                    ## drop the residue (un-handled chunk) if we can succeed to parse a chunk
                    ## this is kind of cleanup, mostly the newline might be cleaned
                    del residue[:]
                except cjson.DecodeError:
                    residue.append(chunk)
        self.data = WuKongNodeREDProtocol.boundary.join(residue)
        debug('received:%s, data buffer:%s' % (data,self.data))
    def connectionMade(self):
        debug('Connected')
    def connectionLost(self,reason):
        debug('Disconnected')
        global message_queue
        message_queue.removeListener(self.onMessageUpdated)
    def onMessageUpdated(self,item):
        if self.transport is None: return
        #global message_queue
        #
        #for item in message_queue.out_queue:
        if self.last_message_out_ts==0 or item['ts'] > self.last_message_out_ts:
            content = WuKongNodeREDProtocol.boundary+cjson.encode(item['payload'])+WuKongNodeREDProtocol.boundary+'\n'
            self.transport.write(content)
            self.last_message_out_ts = item['ts']
            item['ack_count'] += 1
            debug('sending %s' % content)
        

class WuKongNodeREDProtocolFactory(Factory):
    def __init__(self,device):
        self.device = device
        self.protocol = WuKongNodeREDProtocol

def startServerForNodeRED(noderedBroker,port=5101):
    endpoint = TCP4ServerEndpoint(reactor,port)
    endpoint.listen(WuKongNodeREDProtocolFactory(noderedBroker))
    debug('Server is ready for NodeRED at port ',port)
#
# For simulator() to work, noderedBroker is defined to be global.
# This "global" can be removed in productive environment
#
global noderedBroker
def main(gtwip,selfip):
    
    global noderedBroker
    noderedBroker = NodeREDBroker(gtwip,selfip)
    
    startServerForNodeRED(noderedBroker,port=5101)
    
    print "NodeRed Broker is ready"
    reactor.callLater(1.5,simulator)

def simulator():
    """
    Simulate the sending and receiving traffic
    """
    import random
    global noderedBroker
    print "Simulator is running"
    
    # create a wuobject 
    #NodeREDSignalSender_cls = noderedBroker.classes[21001]
    #NodeREDSignalReceiver_cls = noderedBroker.classes[21002]
    noderedBroker.addObject(21001)
    noderedBroker.addObject(21002)
    
    TOPIC_PROPERTY = 0
    BOOLEAN_SIGNAL_PROPERTY = 1
    INTEGER_SINGAL_PROPERTY = 2
    STRING_SIGNAL_PROPERTY = 3

    SIGNAL_SENDER = 1
    SIGNAL_SENDER_TOPIC_VALUE = 1

    SIGNAL_RECEIVER = 2
    SIGNAL_RECEIVER_TOPIC_VALUE = 2

    # does not work, because wkpf.components is empty []
    #noderedBroker.setProperty(SIGNAL_SENDER,TOPIC_PROPERTY,SIGNAL_SENDER_TOPIC_VALUE)
    #noderedBroker.setProperty(SIGNAL_RECEIVER,TOPIC_PROPERTY,SIGNAL_RECEIVER_TOPIC_VALUE)

    # set topic
    signal_sender_object = noderedBroker.objects[0]
    signal_sender_object.cls.update(signal_sender_object,TOPIC_PROPERTY,SIGNAL_SENDER_TOPIC_VALUE)
    def send_signal():
        signal_sender_object = noderedBroker.objects[0]
        value = random.randint(0,100)
        signal_sender_object.cls.update(signal_sender_object,INTEGER_SINGAL_PROPERTY,value)
        reactor.callLater(10,send_signal)
    
    reactor.callLater(1,send_signal)
    
    def receiving_signal():
        signal_receiver_object = noderedBroker.objects[1]
        signal_receiver_object.cls.update(signal_receiver_object,TOPIC_PROPERTY,SIGNAL_RECEIVER_TOPIC_VALUE)


if __name__ == "__main__":
    #root=static.File("./www")
    #root.putChild("cgi-bin",Cgi(d))
    #site=server.Site(root)
    #site.device = d
    #endpoint = TCP4ServerEndpoint(reactor, 80)
    #endpoint.listen(CgiFactory(d))
    #reactor.listenUDP(8000, SearchProtocol())

    options,args,parser = parseArguments()
    if options.develop:
        gtwip = '192.168.56.101'
        selfip = gtwip+':5100'
    else:
        gtwip = options.gateway
        selfip = options.ip

    if gtwip is None or selfip is None:
        parser.print_help()
        sys.exit(1)
    else:
        sys.stderr.write('NodeRED Broker is running with:\n')
        sys.stderr.write('    Gateway:%s\n    Broker ip and port:%s\n' % (gtwip,selfip))
        reactor.callWhenRunning(main,gtwip,selfip)
        reactor.run()
