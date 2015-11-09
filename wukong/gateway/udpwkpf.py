import socket
import struct
import sys
import mptnUtils as MPTN
import uuid
import ipaddress
from twisted.internet import reactor
from twisted.internet.protocol import DatagramProtocol, ClientCreator, ReconnectingClientFactory
import cjson
import traceback
class WKPF(DatagramProtocol):
    GET_WUCLASS_LIST        = 0x90
    GET_WUCLASS_LIST_R      = 0x91
    GET_WUOBJECT_LIST       = 0x92
    GET_WUOBJECT_LIST_R     = 0x93
    READ_PROPERTY           = 0x94
    READ_PROPERTY_R         = 0x95
    WRITE_PROPERTY          = 0x96
    WRITE_PROPERTY_R        = 0x97
    REQUEST_PROPERTY_INIT   = 0x98
    REQUEST_PROPERTY_INIT_R = 0x99
    GET_LOCATION            = 0x9A
    GET_LOCATION_R          = 0x9B
    SET_LOCATION            = 0x9C
    SET_LOCATION_R          = 0x9D
    GET_FEATURES            = 0x9E
    GET_FEATURES_R          = 0x9F
    SET_FEATURE             = 0xA0
    SET_FEATURE_R           = 0xA1
    CHANGE_MAP              = 0xA2
    CHANGE_MAP_R            = 0xA3
    CHANGE_LINK             = 0xA4
    CHANGE_LINK_R           = 0xA5
    ERROR_R                 = 0xAF

    REPROG_OPEN             = 0x10
    REPROG_OPEN_R           = 0x11
    REPROG_WRITE            = 0x12
    REPROG_WRITE_R          = 0x13
    REPROG_COMMIT           = 0x14
    REPROG_COMMIT_R         = 0x15
    REPROG_REBOOT           = 0x16
    REPROG_REBOOT_R         = 0x17

    WKREPROG_OK             = 0
    WKREPROG_REQUEST_RETRANSMIT = 1
    WKREPROG_TOOLARGE       = 2
    WKREPROG_FAILED         = 3

    LIB_INFUSION            = 0
    APP_INFUSION            = 1
    LINK_TABLE              = 2
    COMPONENT_MAP           = 3
    INITVALUES_TABLE        = 4

    DATATYPE_SHORT          = 0
    DATATYPE_BOOLEAN        = 1
    DATATYPE_REFRESH        = 2


    WKCOMM_MESSAGE_PAYLOAD_SIZE=40
    OBJECTS_IN_MESSAGE               = (WKCOMM_MESSAGE_PAYLOAD_SIZE-3)/4

    def __init__(self,dev,host,port,gtwaddr):
        self.host = host
        self.port = port
        self.device = dev
        self._reactor = reactor
        self.gtwaddr = gtwaddr
        self.mptnaddr = 0
        self.nodeid=0
        self.location = 'Default'
        self.properties=[]
        self.tablebin=[]
        self.components=[]
        self.links=[]
        self.seq = 1000
        for i in range(0,4096):
            self.tablebin.append(0)
        self.load()
        self.init()
        pass
    def init(self):
        reactor.listenUDP(self.port, self)
        reactor.callWhenRunning(self.doInit)
    def load(self):
        try:
            f=open('udpwkpf-%d.json' % self.port)
            o = cjson.decode(f.read())
            print o
            self.location = o['location']
            self.uuid = o['uuid']
            self.nodeid = o['nodeid']
            self.components=o['components']
            self.links=o['links']
            self.mptnaddr = o['mptnaddr']
            f.close()
        except:
            self.uuid = map(ord,str(uuid.uuid4().bytes))
            self.save()
            return
        if o.has_key('uuid') == False:
            self.uuid = map(ord,str(uuid.uuid4().bytes))
            self.save()

    def save(self):
        try:
            o = {'location': self.location,'uuid':self.uuid,'nodeid':self.nodeid,'components':self.components, 'links':self.links,'mptnaddr':self.mptnaddr}
            f = open('udpwkpf-%d.json' % self.port,'w')
            f.write(cjson.encode(o))
            f.close()
        except:
            traceback.print_exc()
            pass
    def doInit(self):
        payload_length = 0
        p = struct.pack('11B', 0xAA,0x55,self.nodeid,self.host&0xff,(self.host>>8)&0xff,(self.host>>16)&0xff,(self.host>>24)&0xff,self.port%256,self.port/256,2,payload_length)
        self.transport.write(p,(self.gtwaddr,MPTN.MPTN_UDP_PORT))
        self.state = 'WAITID'
    def requestID(self):
        dest_id = MPTN.MASTER_ID
        src_id = 0xffffffff
        msg_type = MPTN.MPTN_MSGTYPE_IDREQ
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, ''.join(map(chr,self.uuid)))
        payload_length = len(message)
        address = self.host
        port = self.port
        p = struct.pack('11B', 0xAA,0x55,self.nodeid,address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,port%256,port/256,1,payload_length)
        p = p+message
        self.transport.write(p,(self.gtwaddr,MPTN.MPTN_UDP_PORT))
        self.state = 'WAITADDR'


    def datagramReceived(self, data, (host, port)):
        s = ''
        for d in data:
            s = s + '%02x '% ord(d)
        print self.state,s
        if self.state == 'WAITID':
            if ord(data[0]) == 0xAA and ord(data[1]) == 0x55:
                self.nodeid = ord(data[2])
                self.save()
                print 'get node id', self.nodeid
                self.send(0,'AAAA')
                self.state = 'INIT'
                self.requestID()
        elif self.state == 'WAITRSP':
            self.state = 'INIT'
            pass
        elif self.state == 'WAITADDR':
            dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(data[11:])
            if msg_type == MPTN.MPTN_MSGTYPE_IDACK and src_id == MPTN.MASTER_ID:
                src_id = dest_id
                print "Your ID is %d of which dotted format is %s" % (src_id, MPTN.ID_TO_STRING(src_id))
            self.state = 'INIT'
        elif self.state == 'INIT':
            dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(data[11:])
            print dest_id,src_id,msg_type
            if self.mptnaddr == 0:
                self.mptnaddr = dest_id
                self.save()
            if msg_type == 24:
                msg_id = ord(data[20])
                seq = ord(data[21])+ord(data[22])*256
                self.parseWKPF(src_id,msg_id,seq,data[23:])

    def parseWKPF(self,src_id,msgid,seq,payload):
        print 'WKPF ID %x %x %x'%( msgid,src_id,seq)
        if msgid == WKPF.GET_LOCATION:
            offset = ord(payload[0])
            s = self.location[offset:]
            if offset == 0:
                s = s[:WKPF.WKCOMM_MESSAGE_PAYLOAD_SIZE-4]
                msg = chr(len(self.location))+s
            else:
                s = s[:WKPF.WKCOMM_MESSAGE_PAYLOAD_SIZE-3]
                msg = s
            p = struct.pack('3B',WKPF.GET_LOCATION_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
            pass
        elif msgid == WKPF.SET_LOCATION:
            print map(ord,payload)
            offset = ord(payload[0])
            l = ord(payload[1])
            if offset == 0:
                self.location = payload[3:]
            else:
                self.location = self.location + payload[2:]
            self.save()
            p = struct.pack('3B',WKPF.SET_LOCATION_R,seq&255, (seq>>8)&255)+chr(0)
            self.send(src_id,p)

        elif msgid == WKPF.GET_WUCLASS_LIST:
            n_pack = ord(payload[0])
            total = 1
            n_item = len(self.device.classes.keys())
            msg = struct.pack('2B',total,n_item)
            p=struct.pack('3B',WKPF.GET_WUCLASS_LIST_R,seq&255, (seq>>8)&255)+msg
            end

            for CID in self.device.classes.keys():
                p = p + struct.pack('3B', (CID>>8)&0xff, CID&0xff, 0)
            self.send(src_id,p)
        elif msgid == WKPF.GET_WUOBJECT_LIST:
            n_pack = ord(payload[0])
            total = 1
            n_item = len(self.device.objects)
            msg = struct.pack('3B',0,total,n_item)
            p=struct.pack('3B',WKPF.GET_WUOBJECT_LIST_R,seq&255, (seq>>8)&255)+msg
            for i in range(0,len(self.device.objects)):
                obj = self.device.objects[i]
                CID = obj.getID()
                p = p + struct.pack('4B', i, (CID>>8)&0xff, CID&0xff, 0)

            print map(ord,p)
            self.send(src_id,p)
        elif msgid == WKPF.REPROG_OPEN:
            fielid = ord(payload[0])
            #self.openTable(fileid)
            msg = struct.pack('3B', WKPF.WKREPROG_OK, 1024 % 256, 1024/256)
            p=struct.pack('3B',WKPF.REPROG_OPEN_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
        elif msgid == WKPF.REPROG_WRITE:
            pos = ord(payload[0])+ord(payload[1])*256
            for i in range(2,len(payload)):
                self.tablebin[pos+i-2] = ord(payload[i])
            msg = chr(WKPF.WKREPROG_OK)
            self.size = pos + len(payload)-2
            p=struct.pack('3B',WKPF.REPROG_WRITE_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
        elif msgid == WKPF.REPROG_COMMIT:
            msg = chr(WKPF.WKREPROG_OK)
            p=struct.pack('3B',WKPF.REPROG_COMMIT_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
            s = ''
            for i in range(0,self.size):
                s = s + '%02x ' % self.tablebin[i]
            print s
            self.parseTables()
        elif msgid == WKPF.REPROG_REBOOT:
            msg = chr(WKPF.WKREPROG_OK)
            p=struct.pack('3B',WKPF.REPROG_REBOOT_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
        elif msgid == WKPF.WRITE_PROPERTY:
            print map(ord,payload)
            port = ord(payload[0])
            cID = ord(payload[1])*256 + ord(payload[2])
            if cID == 0:
                # The request from the Master will not have componentID in it. It will
                # use the port directly.
                cID = self.findComponentByPort(pport)
            pID = ord(payload[3])
            dtype = ord(payload[4])
            if dtype == WKPF.DATATYPE_SHORT or dtype == WKPF.DATATYPE_REFRESH:
                val = ord(payload[5])*256 + ord(payload[6])
            else:
                val  = ord(payload[5])

            p=struct.pack('7B',WKPF.WRITE_PROPERTY_R,seq&255, (seq>>8)&255, port, (cID>>8)&0xff, cID&0xff, pID)
            self.send(src_id,p)
            self.setProperty(port,pID, val)
        pass
    def parseTables(self):
        i = 0
        files={}

        while i < self.size:
            len = self.tablebin[i]+self.tablebin[i+1]*256
            type = self.tablebin[i+2]
            i += 3
            files[type] = self.tablebin[i:i+len]
            i += len
            print 'type %d  size %d' % (type,len)
            if type == WKPF.LINK_TABLE:
                self.parseLinkTable(files[type])
            elif type == WKPF.COMPONENT_MAP:
                self.parseComponentMap(files[type])
            elif type == WKPF.INITVALUES_TABLE:
                self.parseInitTable(files[type])
        self.save()

    def parseLinkTable(self,data):
        links = data[0]+data[1]*256
        print 'Links(%d):' % links
        self.links={}
        for i in range(0,links):
            p = 2 + 6 * i
            src_id = data[p]+data[p+1]*256
            s_pID = data[p+2]
            dest_id = data[p+3]+data[p+4]*256
            d_pID = data[p+5]
            print '    %d.%d ---> %d.%d' % (src_id,s_pID,dest_id,d_pID)
            if self.links.has_key('%d.%d' % (src_id,s_pID)):
                self.links['%d.%d'%(src_id,s_pID)].append([dest_id,d_pID])
            else:
                self.links['%d.%d'%(src_id,s_pID)]= [[dest_id,d_pID]]
    def addProperties(self,n):
        props = []
        for i in range(0,n): props.append({'value':0,'dirty':False})
        self.properties.append(props)
    def checkDirty(self,port,pID):
        for i in range(self.last_dirty_ptr,len(self.properties)):
            if self.properties[self.last_dirty_ptr]['dirty']:
                self.last_dirty_ptr = i + 1
                return self.properties[port][pID]
        self.last_dirty_ptr = 0
        return None
    def getProperty(self,port,pID):
        return self.properties[port][pID]['value']

    def setProperty(self,port,pID,val):
        print 'setProperty',port,pID,val
        try:
            if self.properties[port][pID]['value'] != val:
                self.properties[port][pID]['value'] = val
                self.properties[port][pID]['dirty'] = True
                self.propagateProperty(port,pID)
        except:
            self.properties[port][pID]['value'] = val
            self.propagateProperty(port,pID,val)
    def remoteSetProperty(self,dest_id,cls,port,pID,val):
        print "cls=",cls
        print "dest_id=",dest_id
        print "port=",port
        src_id = self.mptnaddr
        p = struct.pack('10B', WKPF.WRITE_PROPERTY, self.seq & 0xff, (self.seq >> 8) % 0xff, port, (cls >> 8) & 0xff, cls & 0xff, pID, WKPF.DATATYPE_SHORT, (val >> 8)&0xff, val & 0xff)
        msg_type = MPTN.MPTN_MSGTYPE_FWDREQ

        self.send(dest_id,p)
        self.seq = self.seq + 1
        pass
    def remoteGetProperty(self,addr,port,pID,cb):
        print 'remote get is not implemented yet'
        pass

    def findComponentByPort(self, port):
        for i in range(0,len(self.components)):
            c = self.components[i]
            for e in c['ports']:
                if e[1] == port:
                    return i
        return -1
    def getComponent(self,cid):
        return self.components[cid]
    def propagateProperty(self,port,pID,val):
        dirty_id = self.findComponentByPort(port)
        comp = self.getComponent(dirty_id)
        print ' check propagate', self.links
        for l in self.links:
            src_id,src_propertyID = l.split('.')
            src_id = int(src_id)
            src_propertyID = int(src_propertyID)
            print 'check link', src_id,dirty_id,pID,src_propertyID
            if src_id == dirty_id and pID == src_propertyID:
                target_links = self.links[l]
                print 'match link',target_links
                for target in target_links:
                    try:
                        print target
                        comp = self.getComponent(target[0])
                        print comp
			if comp['ports'][0][0] == self.mptnaddr:
                        	self.setProperty(comp['ports'][0][1],target[1],val)
			else:
                        	self.remoteSetProperty(comp['ports'][0][0],comp['ID'],comp['ports'][0][1], target[1],val)
                    except:
                        traceback.print_exc()
                        pass

    def parseInitTable(slef,data):
        pass
    def parseComponentMap(self,data):
        n_item = data[0] + data[1]*256
        self.components=[]

        for i in range(0,n_item):
            addr = data[2+i*2]+data[2+i*2+1]*256
            n_endpoints = data[addr]
            clsid = data[addr+1]+data[addr+2]*256
            print 'component class ID %d' % clsid
            com = {'ID':clsid, 'ports': []}
            for j in range(0,n_endpoints):
                mptnaddr = (data[addr+3+j*5+3]<<24) | (data[addr+3+j*5+2]<<16) | (data[addr+3+j*5+1]<<8) | (data[addr+3+j*5])
                port = data[addr+3+j*5+4]
                print '    addr %x at port %d' % (mptnaddr,port)
                com['ports'].append([mptnaddr,port])
            self.components.append(com)

    def send(self,dest_id,payload):
        src_id = self.mptnaddr
        msg_type = MPTN.MPTN_MSGTYPE_FWDREQ
        print src_id
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
        payload_length = len(message)
        p = struct.pack('11B', 0xAA,0x55,src_id&0xff,dest_id&0xff,(dest_id>>8)&0xff,(dest_id>>16)&0xff,(dest_id>>24)&0xff,self.port%256,self.port/256,1,payload_length)
        p = p+message
        self.transport.write(p,(self.gtwaddr,MPTN.MPTN_UDP_PORT))

class WuObject:
    def __init__(self,cls):
        self.cls = cls
        self.port = 0
    def getID(self):
        return self.cls.ID
    def setProperty(self,pID,val):
        self.cls.setProperty(self.port,pID,val)
    def getProperty(self,pID):
        return self.cls.getProperty(self.port,pID)

class WuClass:
    def __init__(self):
        self.ID = 0
        self.wkpf = None
    def update(self,port=-1,pID=-1,value=0):
        pass
    def newObject(self):
        return WuObject(self)
    def setProperty(self,port,pID,val):
        self.wkpf.setProperty(port,pID,val)
    def getProperty(self,port,pID):
        return self.wkpf.getProperty(port,pID)

class Device:
    def __init__(self,addr,localaddr):
        tcp_address = localaddr.split(":")
        address = MPTN.ID_FROM_STRING(tcp_address[0])
        port = int(tcp_address[1])
        self.wkpf = WKPF(self,address,port,addr)
        self.classes={}
        self.objects=[]
        self.init()
        reactor.callLater(1,self.updateTheNextDirtyObject)
        pass
    def updateTheNextDirtyObject(self):
        for obj in self.objects:
            for i in range(0,len(self.wkpf.properties[obj.port])):
                p = self.wkpf.properties[obj.port][i]
                if p['dirty'] == True:
                    p['dirty'] = False
                    try:
                        obj.cls.update(obj.port,i,p['value'])
                    except:
                        traceback.print_exc()
                        pass
        reactor.callLater(0.3, self.updateTheNextDirtyObject)

    def getPortClassID(self,port):
        return self.objects[port].cls.ID

    def addClass(self,cls):
        self.classes[cls.ID] = cls
        cls.wkpf = self.wkpf
    def addObject(self,ID):
        cls = self.classes[ID]
        self.wkpf.addProperties(3)
        if cls:
            obj = cls.newObject()
            obj.port = len(self.objects)
            self.objects.append(obj)
            return obj
        return None
    def setProperty(self,pID, val):
        self.wkpf.setProperty(pID,val)
    def getProperty(self,pID):
        return self.wkpf.setProperty(pID,val)

if __name__ == "__main__":
    class Magnetic(WuClass):
        def __init__(self):
            self.ID = 1007
        def update(self):
            pass
    class Threshold(WuClass):
        def __init__(self):
            self.ID = 1
        def update(self):
            pass
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
            self.count=0
        def init(self):
            m = Magnetic()
            self.addClass(m)
            self.obj_sensor = self.addObject(m.ID)
            m = Threshold()
            self.addClass(m)
            self.obj_th = self.addObject(m.ID)
            reactor.callLater(1,self.loop)
        def loop(self):
            self.count = self.count + 1
            self.obj_sensor.setProperty(0,self.count)
            reactor.callLater(1,self.loop)
    if len(sys.argv) <= 2:
            print 'python udpwkpf.py <ip> <port>'
            print '      <ip>: IP of the interface'
            print '      <port>: The unique port number in the interface'
            print ' ex. python udpwkpf.py 127.0.0.1 3000'
            sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[1]+':'+sys.argv[2])
    reactor.run()

