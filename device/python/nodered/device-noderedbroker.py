#
#
# 2016/4/25
# Change:
#   1. replace "Topic" with "Subject"
#   2. Set the type of Subject to be string, (Topic is integer)
#
import sys,os,time,cjson,traceback,weakref
sys.path.insert(0,os.path.abspath(os.path.join(os.path.dirname(__file__),'..')))
from udpwkpf import WuClass,Device
#from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor
#from twisted.web import resource,static,server
from twisted.internet.protocol import Factory,Protocol
from twisted.internet.endpoints import TCP4ServerEndpoint

debug = None

def verbose(*s,**kw):
    newline= kw.get('newline','\n')
    cols = [str(x) for x in s]
    line = '\t'.join(cols).encode('utf-8')+newline
    sys.stderr.write(line)
def silent(*s,**kw):
    #sys.stderr.write('SILENT\n')
    pass

class MessageQueue(object):
    def __init__(self):
        self.queue = []
        self.listeners = {}
        self._timer = None
        self._message_ttl = 3.0 ## 3 seconds
    def addMessage(self,message):
        debug('add out message',message['payload'])
        self.queue.append(message)
        if self._timer is None:
            self._timer = reactor.callLater(0.5,self._notify)
    def _notify(self):
        #
        # Consume the outgoing queue
        # do cleanup of the queue at the same time
        #
        now = time.time()
        item_tobe_remove = []
        for item in self.queue:

            subject = item['payload']['subject']

            if now - item['ts'] > self._message_ttl:
                item_tobe_remove.append(item)
                continue
                debug('drop out message ',item['payload'])

            try:
                listeners = self.listeners[subject]
            except KeyError:
                listeners = []

            for listener in listeners:
                try:
                    listener(item)
                except:
                    debug('NodeRedBroker Listener Error:%s' % traceback.format_exc())

            item_tobe_remove.append(item)
            debug('cleanup out message',item['payload'])

        item_tobe_remove.reverse()
        for item in item_tobe_remove:
            self.queue.remove(item)
        debug('%s message removed' % len(item_tobe_remove),', queue length',len(self.queue),',listener length',len(self.listeners))
        self._timer = None

    def addListener(self,subject,_callable):
        try:
            self.listeners[subject].append(_callable)
        except KeyError:
            self.listeners[subject] = [_callable]
    def removeListener(self,_callable):
        remove_subject = False
        for subject,callables in self.listeners.iteritems():
            if _callable in callables:
                callables.remove(_callable)
                remove_subject = len(callables)==0
                break
        if remove_subject:
            del self.listeners[subject]
        debug('after removed listetners:%s' % len(self.listeners))
global message_out_queue
message_out_queue = MessageQueue()

class NodeREDSignalSender(WuClass):
    """
    Receive signal from WuKong, then pass to NodeRED
    """
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('NodeREDSignalSender')
    def update(self,obj,pID,val):
        debug('NodeREDSignalSender update',[obj,pID,val])
        if pID==0:
            #
            # This is setting the subject, doesn't need to send to remote,
            #
            setattr(obj,'subject',str(val))
        else:
            global message_out_queue
            message_out_queue.addMessage(self.createMessage(obj,pID,val))

    def init(self):
        print 'NodeREDSignalSender init'
    def __repr__(self):
        return '<NodeREDSignalSender>'

    def createMessage(self,obj,pID,val):
        signalType = {
            1:'boolean',
            2:'short',
            3:'string',
        }
        message = {
            'payload':{
                'subject':getattr(obj,'subject'),
                'type':signalType[pID],
                'value':val
            },
            'ts':time.time()
        }
        return message


class NodeREDSignalReceiver(WuClass):
    """
    Receive signal from NodeRED, then pass to WuKong
    """
    subscribers = weakref.WeakSet()
    def __init__(self):
        WuClass.__init__(self)
        self.subject = '0' ## default subject
        self.loadClass('NodeREDSignalReceiver')
    def update(self,obj,pID,val):
        debug('NodeREDSignalReceiver update',[obj,pID,val])
        #WuClass.update(self,obj,pID,val)
        if pID==0:
            #
            # This call want to set the subject of the object
            #
            setattr(obj,'subject',str(val))

            if not obj in NodeREDSignalReceiver.subscribers:
                NodeREDSignalReceiver.subscribers.add(obj)
            return
    def init(self):
        pass
    def __repr__(self):
        return '<NodeREDSignalReceiver>'
    @classmethod
    def dispatchMessage(cls,obj):
        #
        # prepare value
        #
        signalIndexOfType = {
            'boolean':1,
            'short':2,
            'string':3,
        }

        #
        # convert the subject to string
        #
        subject = str(obj['subject'])

        pID = signalIndexOfType.get(obj['type'],None)
        if pID is None:
            pID = signalIndexOfType['string']
            value = str(obj)
        else:
            value = obj['value']
        #
        # notify subscribers
        #
        debug('dispatching subject:%s property index:%s value:%s ' % (subject,pID,value))
        for subscriber in cls.subscribers:
            if subscriber.subject == subject:
                subscriber.setProperty(pID,value)
        return

class NodeREDBroker(Device):
    def __init__(self,gtwip,selfip):
        Device.__init__(self,gtwip,selfip)
    def init(self):
        cls = NodeREDSignalReceiver()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = NodeREDSignalSender()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
    def update(self,obj,pID,val):
        debug('NodeREDBroker.update()',[obj,pID,val])
        pass


#
# Communication to NodeRED
#
class WuKongNodeREDProtocol(Protocol):
    boundary = '||'
    def __init__(self):
        self.last_message_out_ts = 0
        self.last_message_in_ts = 0
        self.subscribed_subject = None
        self.data = ''
    def dataReceived(self, data):
        #self.transport.write(data)
        self.data += data
        chunks = self.data.split(WuKongNodeREDProtocol.boundary)
        residue = []
        if len(chunks) > 1:
            for chunk in chunks:
                if not chunk: continue
                try:
                    obj = cjson.decode(chunk)
                    debug('message in %s' % obj)

                    if obj.has_key('subscribe') and ((self.subscribed_subject is None) or (self.subscribed_subject !=obj['subscribe'])):
                        #
                        # This is a request to subscribe some subject
                        #
                        global message_out_queue
                        debug('subscribe subject %s' % obj['subject'])
                        if self.subscribed_subject is not None:
                            message_out_queue.removeListener(self.onMessageUpdated)
                        self.subscribed_subject = int(obj['subject'])
                        message_out_queue.addListener(obj['subject'],self.onMessageUpdated)
                    elif obj.has_key('subject'):
                        #
                        # This is an input data
                        #
                        reactor.callLater(0,NodeREDSignalReceiver.dispatchMessage,obj)
                    else:
                        #
                        # Unknown
                        #
                        debug('Message Unhandled:%s' % obj)
                    #
                    # drop the content of the residue (un-handled chunk) buffer
                    # if we can succeed to parse a chunk.
                    # this is kind of cleanup, mostly the newline might be cleaned
                    #
                    del residue[:]
                except cjson.DecodeError:
                    #
                    # suppose there is not enough data been received
                    #
                    residue.append(chunk)
        self.data = WuKongNodeREDProtocol.boundary.join(residue)
        debug('received:%s, data buffer:%s' % (data,self.data))
    def connectionMade(self):
        debug('Connected')
    def connectionLost(self,reason):
        debug('Disconnected')
        global message_out_queue
        message_out_queue.removeListener(self.onMessageUpdated)
    def onMessageUpdated(self,item):
        if self.transport is None: return
        #global message_out_queue
        #
        #for item in message_out_queue.queue:
        if self.last_message_out_ts==0 or item['ts'] > self.last_message_out_ts:
            content = WuKongNodeREDProtocol.boundary+cjson.encode(item['payload'])+WuKongNodeREDProtocol.boundary+'\n'
            self.transport.write(content)
            self.last_message_out_ts = item['ts']
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
def main(gtwip,selfip,options):

    global noderedBroker
    noderedBroker = NodeREDBroker(gtwip,selfip)

    startServerForNodeRED(noderedBroker,port=5101)

    if options.simulator:
        reactor.callLater(1.5,simulator)

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
    parser.add_option('-s','--simulator',action="store_true",
        help='run the simulator')
    parser.add_option('-v','--verbose',action="store_true",
        help='verbosely dumping message')

    (options, args) = parser.parse_args()

    return options,args,parser

def simulator():
    """
    Simulate the sending and receiving traffic
    """
    import random
    global noderedBroker
    debug("Simulator is running")

    # create a wuobject
    #NodeREDSignalSender_cls = noderedBroker.classes[21001]
    #NodeREDSignalReceiver_cls = noderedBroker.classes[21002]

    # 2 sender
    noderedBroker.addObject(21001)
    noderedBroker.addObject(21001)
    # 1 receiver
    noderedBroker.addObject(21002)

    SUBJECT_PROPERTY = 0
    BOOLEAN_SIGNAL_PROPERTY = 1
    INTEGER_SINGAL_PROPERTY = 2
    STRING_SIGNAL_PROPERTY = 3

    SIGNAL_SENDER = 1
    SIGNAL_SENDER1_SUBJECT_VALUE = 'A-Subject'
    SIGNAL_SENDER2_SUBJECT_VALUE = 'C-Subject'

    SIGNAL_RECEIVER = 2
    SIGNAL_RECEIVER_SUBJECT_VALUE = 'B-Subject'

    # does not work, because wkpf.components is empty []
    #noderedBroker.setProperty(SIGNAL_SENDER,TOPIC_PROPERTY,SIGNAL_SENDER_TOPIC_VALUE)
    #noderedBroker.setProperty(SIGNAL_RECEIVER,TOPIC_PROPERTY,SIGNAL_RECEIVER_TOPIC_VALUE)

    # set subject
    signal_sender1_object = noderedBroker.objects[0]
    signal_sender1_object.cls.update(signal_sender1_object,SUBJECT_PROPERTY,SIGNAL_SENDER1_SUBJECT_VALUE)

    signal_sender2_object = noderedBroker.objects[1]
    signal_sender2_object.cls.update(signal_sender2_object,SUBJECT_PROPERTY,SIGNAL_SENDER2_SUBJECT_VALUE)

    def send_signal():
        v = 0 # random.randint(0,100)%3
        def send1():
            signal_sender1_object = noderedBroker.objects[0]
            value = random.randint(0,100)
            signal_sender1_object.cls.update(signal_sender1_object,INTEGER_SINGAL_PROPERTY,value)
        def send2():
            signal_sender2_object = noderedBroker.objects[1]
            value = random.randint(0,100)
            signal_sender2_object.cls.update(signal_sender2_object,INTEGER_SINGAL_PROPERTY,value)
        if v==0:
            send1()
        elif v==1:
            send2()
        else:
            send1()
            send2()
        reactor.callLater(3,send_signal)

    reactor.callLater(1,send_signal)

    def receiving_signal():
        signal_receiver_object = noderedBroker.objects[2]
        signal_receiver_object.cls.update(signal_receiver_object,SUBJECT_PROPERTY,SIGNAL_RECEIVER_SUBJECT_VALUE)

    receiving_signal()

if __name__ == "__main__":
    #root=static.File("./www")
    #root.putChild("cgi-bin",Cgi(d))
    #site=server.Site(root)
    #site.device = d
    #endpoint = TCP4ServerEndpoint(reactor, 80)
    #endpoint.listen(CgiFactory(d))
    #reactor.listenUDP(8000, SearchProtocol())

    options,args,parser = parseArguments()
    if options.verbose:
        debug = verbose
    else:
        debug = silent
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
        debug('NodeRED Broker is running with:\n')
        debug('    Gateway:%s\n    Broker ip and port:%s\n' % (gtwip,selfip))
        reactor.callWhenRunning(main,gtwip,selfip,options)
        reactor.run()
