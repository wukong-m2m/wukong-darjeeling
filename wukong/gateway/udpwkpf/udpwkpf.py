import socket
import struct
import sys,os
import mptnUtils as MPTN
import uuid
import ipaddress
from twisted.internet import reactor
from twisted.internet.protocol import DatagramProtocol, ClientCreator, ReconnectingClientFactory
import cjson
import traceback
import xml.dom.minidom
from xml.dom.minidom import parseString
import time

lib_path = "/../../ComponentDefinitions/WuKongStandardLibrary.xml"

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
    DATATYPE_ARRAY          = 3
    DATATYPE_STRING         = 4

    DATATYPE_ThresholdOperator = 10
    DATATYPE_LogicalOperator   = 11
    DATATYPE_MathOperator      = 12
    DATATYPE_Pin               = 13

    WK_PROPERTY_ACCESS_READONLY  = 1<<7
    WK_PROPERTY_ACCESS_WRITEONLY = 1<<6
    WK_PROPERTY_ACCESS_READWRITE = (WK_PROPERTY_ACCESS_READONLY+WK_PROPERTY_ACCESS_WRITEONLY)

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
        self.properties= []
        for i in range(0,100):
            self.properties.append([])
        self.tablebin=[]
        self.components=[]
        self.links=[]
        self.seq = 1000
        for i in range(0,4096):
            self.tablebin.append(0)
        self.load()
        self.init()

    def init(self):
        reactor.listenUDP(self.port, self)
        reactor.callWhenRunning(self.doInit)
    def load(self):
        try:
            f=open('udpwkpf-%d.json' % self.port)
            o = cjson.decode(f.read())
            # print o
            self.location = o['location']
            self.uuid = o['uuid']
            self.nodeid = o['nodeid']
            self.components=o['components']
            self.links=o['links']
            self.mptnaddr = o['mptnaddr']
            try:
                self.properties = o['props']
            except:
                pass
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
            o = {'location': self.location,'uuid':self.uuid,'nodeid':self.nodeid,'components':self.components, 'links':self.links,'mptnaddr':self.mptnaddr,'props':self.properties}
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
        # print self.state,s
        if self.state == 'WAITID':
            if ord(data[0]) == 0xAA and ord(data[1]) == 0x55:
                self.nodeid = ord(data[2])
                self.save()
                # print 'get node id', self.nodeid
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
            #print dest_id,src_id,msg_type, map(ord, payload) #payload info
            if self.mptnaddr == 0:
                self.mptnaddr = dest_id
                self.save()
            if msg_type == 24:
                msg_id = ord(data[20])
                seq = ord(data[21])+ord(data[22])*256
                self.parseWKPF(src_id,msg_id,seq,data[23:])

    def parseWKPF(self,src_id,msgid,seq,payload):
        # print 'WKPF ID %x %x %x'%( msgid,src_id,seq)
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
            # print map(ord,payload)
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
            msg = struct.pack('3B',0,total,n_item)
            p=struct.pack('3B',WKPF.GET_WUCLASS_LIST_R,seq&255, (seq>>8)&255)+msg

            for CID in self.device.classes.keys():
                CID = int(CID)
                p = p + struct.pack('3B', (CID>>8)&0xff, CID&0xff, self.device.classtypes[CID])
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
                p = p + struct.pack('4B', obj.port, (CID>>8)&0xff, CID&0xff, 0)

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
            # print s
            self.parseTables()
        elif msgid == WKPF.REPROG_REBOOT:
            msg = chr(WKPF.WKREPROG_OK)
            p=struct.pack('3B',WKPF.REPROG_REBOOT_R,seq&255, (seq>>8)&255)+msg
            self.send(src_id,p)
        elif msgid == WKPF.WRITE_PROPERTY:
            #print map(ord,payload)
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
            elif dtype == WKPF.DATATYPE_ARRAY:
                val = map(ord, payload[5:])
                val = val[1:1+val[0]]
            elif dtype == WKPF.DATATYPE_STRING:
                val = "".join(payload[6:6+ord(payload[5])])
            else:
                val = True if ord(payload[5]) else False

            p=struct.pack('7B',WKPF.WRITE_PROPERTY_R,seq&255, (seq>>8)&255, port, (cID>>8)&0xff, cID&0xff, pID)
            self.send(src_id,p)
            # print "before WRITE_PROPERTY setProperty"
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
            # print 'type %d  size %d' % (type,len)
            if type == WKPF.LINK_TABLE:
                self.parseLinkTable(files[type])
            elif type == WKPF.COMPONENT_MAP:
                self.parseComponentMap(files[type])
            elif type == WKPF.INITVALUES_TABLE:
                self.parseInitTable(files[type])
        self.save()

    def parseLinkTable(self,data):
        links = data[0]+data[1]*256
        # print 'Links(%d):' % links
        self.links={}
        for i in range(0,links):
            p = 2 + 6 * i
            src_id = data[p]+data[p+1]*256
            s_pID = data[p+2]
            dest_id = data[p+3]+data[p+4]*256
            d_pID = data[p+5]
            # print '    %d.%d ---> %d.%d' % (src_id,s_pID,dest_id,d_pID)
            if self.links.has_key('%d.%d' % (src_id,s_pID)):
                self.links['%d.%d'%(src_id,s_pID)].append([dest_id,d_pID])
            else:
                self.links['%d.%d'%(src_id,s_pID)]= [[dest_id,d_pID]]
    def addProperties(self,port,cls,n=7):
        if self.properties[port] != []:
            return
        self.properties[port] = cls.defaultProps
        # props = []
        # for i in range(0,n): props.append({'value':0,'dirty':False})
        # self.properties[port] = props
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
        # print 'setProperty',port,pID,val
        # print "\t",self.properties
        try:
            if self.properties[port][pID]['value'] != val:
                self.properties[port][pID]['value'] = val
                self.properties[port][pID]['dirty'] = True
                self.propagateProperty(port,pID,val)
        except Exception as e:
            print e
            self.properties[port][pID]['value'] = val
            self.propagateProperty(port,pID,val)
    def remoteSetProperty(self,dest_id,cls,port,pID,val,src_cid,dest_cid):
        # print "cls=",cls
        # print "dest_id=",dest_id
        # print "port=",port
        src_id = self.mptnaddr
        if type(val) == bool:
            p = struct.pack('9B', WKPF.WRITE_PROPERTY, self.seq & 0xff, (self.seq >> 8) & 0xff, port, (cls >> 8) & 0xff, cls & 0xff, pID, WKPF.DATATYPE_BOOLEAN, val & 0xff)
        elif type(val) == list:
            val_len = len(val)
            val = val + [0]*(30 - val_len)
            p = struct.pack('39B', WKPF.WRITE_PROPERTY, self.seq & 0xff, (self.seq >> 8) & 0xff, port,
                            (cls >> 8) & 0xff, cls & 0xff, pID, WKPF.DATATYPE_ARRAY,
                             val_len, *map(lambda x: x&0xff ,val))
        elif type(val) == str:
            val_len = len(val)
            val = list(val)
            val = val + ['0']*(30 - val_len)
            p = struct.pack('39B', WKPF.WRITE_PROPERTY, self.seq & 0xff, (self.seq >> 8) & 0xff, port,
                            (cls >> 8) & 0xff, cls & 0xff, pID, WKPF.DATATYPE_STRING,
                             val_len, *map(lambda x: ord(x)&0xff ,val))
        else:
            p = struct.pack('10B', WKPF.WRITE_PROPERTY, self.seq & 0xff, (self.seq >> 8) & 0xff, port, (cls >> 8) & 0xff, cls & 0xff, pID, WKPF.DATATYPE_SHORT, (val >> 8)&0xff, val & 0xff)

        msg_type = MPTN.MPTN_MSGTYPE_FWDREQ

        self.send(dest_id,p)
        self.seq = self.seq + 1
        pass
    def remoteGetProperty(self,addr,port,pID,cb):
        # print 'remote get is not implemented yet'
        pass

    def findComponentByPort(self, port):
        for i in range(0,len(self.components)):
            c = self.components[i]
            for e in c['ports']:
                # print "findComponentByPort", port, "e", e
                if e[1] == port and e[0] == self.mptnaddr:
                    return i
        return -1
    def getComponent(self,cid):
        # print "cid ",cid
        # print "self.components ", self.components[:]
        return self.components[cid]
    def propagateProperty(self,port,pID,val):
        dirty_id = self.findComponentByPort(port)
        # print "propagateProperty dirty_id ", dirty_id
        if dirty_id == -1: return
        comp = self.getComponent(dirty_id)
        # print 'propagateProperty check propagate', self.links
        for l in self.links:
            src_id,src_propertyID = l.split('.')
            src_id = int(src_id)
            src_propertyID = int(src_propertyID)
            # print 'propagateProperty check link', src_id,dirty_id,pID,src_propertyID
            if src_id == dirty_id and pID == src_propertyID:
                target_links = self.links[l]
                # print "propagateProperty target_links", target_links
                for target in target_links:
                    try:
                        # print "propagateProperty target", target
                        comp = self.getComponent(target[0])
                        # print "propagateProperty comp", comp
                        if comp['ports'][0][0] == self.mptnaddr:
                            # print "propagateProperty setProperty"
                            self.setProperty(comp['ports'][0][1],target[1],val)
                        else:
                            self.remoteSetProperty(comp['ports'][0][0],comp['ID'],comp['ports'][0][1], target[1],val,src_id,target[0])
                    except:
                        traceback.print_exc()
                        pass

    def parseInitTable(self,data):
        number = data[0]+data[1]*256
        i = 2
        # print data
        while i < len(data):
            # print data[i:]
            cid = data[i]+data[i+1]*256
            pID = data[i+2]
            size = data[i+3]
            comp = self.getComponent(cid)
            # print comp,pID,size
            for p in comp['ports']:
                # print p,self.mptnaddr
                if p[0] == self.mptnaddr:
                    if size == 1:
                        v = True if data[i+4] else False
                        # print 'init prop %d of component %d to be %d' % (pID, cid, v)
                        self.setProperty(p[1], pID, v)
                    elif size == 2:
                        v = data[i+4]+256*data[i+5]
                        self.setProperty(p[1], pID, v)
                        # print 'init prop %d of component %d to be %d' % (pID, cid, v)
                    else:
                        print 'Unknown value size %d' % size
            i += 4 + size
        pass
    def parseComponentMap(self,data):
        n_item = data[0] + data[1]*256
        self.components=[]

        for i in range(0,n_item):
            addr = data[2+i*2]+data[2+i*2+1]*256
            n_endpoints = data[addr]
            clsid = data[addr+1]+data[addr+2]*256
            # print 'component class ID %d' % clsid
            com = {'ID':clsid, 'ports': []}
            for j in range(0,n_endpoints):
                mptnaddr = (data[addr+3+j*5+3]<<24) | (data[addr+3+j*5+2]<<16) | (data[addr+3+j*5+1]<<8) | (data[addr+3+j*5])
                port = data[addr+3+j*5+4]
                # print '    addr %x at port %d' % (mptnaddr,port)
                com['ports'].append([mptnaddr,port])
                self.device.checkObject(clsid, port)

            self.components.append(com)

    def initObjects(self):
        for i in range(0,len(self.components)):
            com = self.components[i]
            for j in range(0,len(com['ports'])):
                p = com['ports'][j]
                addr = p[0]
                port = p[1]
                self.device.checkObject(int(com['ID']), port)

    def send(self,dest_id,payload):
        src_id = self.mptnaddr
        msg_type = MPTN.MPTN_MSGTYPE_FWDREQ
        # print src_id
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
        payload_length = len(message)
        p = struct.pack('11B', 0xAA,0x55,src_id&0xff,dest_id&0xff,(dest_id>>8)&0xff,(dest_id>>16)&0xff,(dest_id>>24)&0xff,self.port%256,self.port/256,1,payload_length)
        p = p+message
        self.transport.write(p,(self.gtwaddr,MPTN.MPTN_UDP_PORT))

class WuObject:
    def __init__(self,cls):
        self.cls = cls
        self.port = 0
        self.refresh_rate = 0
        self.next_scheduled_update = 0
    def getID(self):
        return self.cls.ID
    def setProperty(self,pID,val):
        # print 'wuobject setProperty'
        self.cls.setProperty(self.port,pID,val)
    def getProperty(self,pID):
        return self.cls.getProperty(self.port,pID)

class WuClass:
    def __init__(self):
        self.ID = 0
        self.wkpf = None
        self.propertyNumber = 0
        self.props_datatype_and_access = [] # this follows the definition of properties[] in wuclass_t
        self.defaultProps = []              # this is default value list
    def update(self,obj,pID,value):
        pass
    def newObject(self):
        return WuObject(self)
    def setProperty(self,port,pID,val):
        # print 'WuClass setProperty'
        self.wkpf.setProperty(port,pID,val)
    def getProperty(self,port,pID):
        return self.wkpf.getProperty(port,pID)
    def getWuClassID(self,name):
        for p in sys.path:
            path = p+lib_path
            if os.path.isfile(path):
                break
        dom = xml.dom.minidom.parse(path)
        for cls in dom.getElementsByTagName("WuClass"):
            if cls.attributes['name'].nodeValue == name:
                return int(cls.attributes['id'].nodeValue)
        print "Can not find class ID for ", name
        return -1
    def unicode_to_int(self, key):
        datatype      = {'short':WKPF.DATATYPE_SHORT,
                         'boolean':WKPF.DATATYPE_BOOLEAN,
                         'refresh_rate':WKPF.DATATYPE_REFRESH,
                         'array':WKPF.DATATYPE_ARRAY,
                         'string':WKPF.DATATYPE_STRING}
        datatype_enum = {'ThresholdOperator':WKPF.DATATYPE_ThresholdOperator,
                         'LogicalOperator':WKPF.DATATYPE_LogicalOperator,
                         'MathOperator':WKPF.DATATYPE_MathOperator,
                         'Pin':WKPF.DATATYPE_Pin}
        access        = {'readonly':WKPF.WK_PROPERTY_ACCESS_READONLY,
                         'writeonly':WKPF.WK_PROPERTY_ACCESS_WRITEONLY,
                         'readwrite':WKPF.WK_PROPERTY_ACCESS_READWRITE}
        if key in datatype:
            return datatype[key]
        elif key in datatype_enum:
            return datatype_enum[key]
        elif key in access:
            return access[key]
        else:
            raise NotImplementedError
    def getWuTypedefEnum(self, name):
        for p in sys.path:
            path = p+lib_path
            if os.path.isfile(path):
                break
        component_string = open(path).read()
        dom = parseString(component_string)
        wutypedefs_dom = dom.getElementsByTagName("WuTypedef")
        wuTypedefs = {}
        for wutypedef in wutypedefs_dom:
                wuTypedefs[wutypedef.getAttribute('name')] = tuple([element.getAttribute('value') for element in wutypedef.getElementsByTagName('enum')])
        enum_val_tuple = wuTypedefs[name]
        enum_val_dict = {}
        for i in range(len(enum_val_tuple)):
            enum_val_dict[str(enum_val_tuple[i])] = i
            # print enum_val_dict
        return enum_val_dict
    def addDefaultProperties(self, DefaultVal):
        self.defaultProps.append({'value':DefaultVal,'dirty':False})
    def WKPF_IS_READONLY_PROPERTY(self, typeAndAccess):
        return ((~typeAndAccess) & WKPF.WK_PROPERTY_ACCESS_WRITEONLY)
    def WKPF_IS_WRITEONLY_PROPERTY(self, typeAndAccess):
        return ((~typeAndAccess) & WKPF.WK_PROPERTY_ACCESS_READONLY)
    def WKPF_GET_PROPERTY_DATATYPE(self, typeAndAccess):
        return ((typeAndAccess) & ~WKPF.WK_PROPERTY_ACCESS_READWRITE)
    def loadClass(self,name):
        for p in sys.path:
            path = p+lib_path
            if os.path.isfile(path):
                break
        dom = xml.dom.minidom.parse(path)
        obj = self.__class__
        for cls in dom.getElementsByTagName("WuClass"):
            if cls.attributes['name'].nodeValue == name:
                self.ID = int(cls.attributes['id'].nodeValue)
                pID_count = 0
                self.names=[]
                for p in cls.getElementsByTagName('property'):
                    # create a props_datatype_and_access list
                    datatype_unicode = p.attributes['datatype'].nodeValue
                    access_unicode   = p.attributes['access'].nodeValue
                    x = self.unicode_to_int(datatype_unicode) + self.unicode_to_int(access_unicode)
                    self.props_datatype_and_access.append(x)
                    # create a defaultProps list to store default value of standardlibrary.xml

                    try:
                        default_unicode = p.attributes['default'].nodeValue
                        if self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_BOOLEAN:
                            if default_unicode == 'false':
                                self.addDefaultProperties(False)
                            elif default_unicode == 'true':
                                self.addDefaultProperties(True)
                            else:
                                self.addDefaultProperties(bool(default_unicode))
                        elif self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_SHORT or self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_REFRESH:
                            self.addDefaultProperties(int(default_unicode))
                        elif self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_ARRAY:
                            self.addDefaultProperties([int(default_unicode)])
                        #elif self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_STRING:
                        #    self.addDefaultProperties(str(default_unicode))
                        elif self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_ThresholdOperator or self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_LogicalOperator or self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_MathOperator or self.WKPF_GET_PROPERTY_DATATYPE(x) == WKPF.DATATYPE_Pin:
                            enum_val_dict = self.getWuTypedefEnum(datatype_unicode)
                            enum_val      = enum_val_dict[str(default_unicode).upper()]
                            self.addDefaultProperties(enum_val)
                        else:
                            raise NotImplementedError
                    except Exception as e: # if default is not defined, it will fall into here
                        self.addDefaultProperties(0)
                        # print e

                    # count property number
                    obj.__dict__[p.attributes['name'].nodeValue] = pID_count
                    self.names.append(p.attributes['name'].nodeValue)
                    pID_count = pID_count + 1
                self.propertyNumber = pID_count
                return
        print "Can not find class ID for ", name
        self.propertyNumber = 0
        return
    def getPropertyNumber(self):
        return self.propertyNumber
    def getPropertyName(self,ID):
        try:
            return self.names[ID]
        except:
            return '%d' % ID

class Device:
    FLAG_APP_CAN_CREATE_INSTANCE = 2
    FLAG_VIRTUAL = 1
    def __init__(self,addr,localaddr):
        tcp_address = localaddr.split(":")
        address = MPTN.ID_FROM_STRING(tcp_address[0])
        port = int(tcp_address[1])
        self.wkpf = WKPF(self,address,port,addr)
        self.classes={}
        self.classtypes = {}
        self.objects= []
        self.init()
        self.wkpf.initObjects()
        reactor.callLater(1,self.updateTheNextDirtyObject)
        reactor.callLater(1,self.updateRefreshRateObject)
        pass
    def getLocation(self):
        return self.wkpf.location
    def checkObject(self,clsid,port):
        i = 0
        while i < len(self.objects):
            obj = self.objects[i]
            if obj.port == port:
                break;
            i = i + 1
        if i == len(self.objects):
            # If i == len(self.objects), it means that device only has this wuclass and doesn't have wuobject.
            # This will happen as we only use addWuClass function and doesn't use addWuObject function in the python device
            # Afrer we deploy the FBP with this wuclass, the wuobject will be created here.
            # print 'add object class %d at port %d' % (clsid,port)
            try:
                if clsid in self.classes:
                    cls = self.classes[clsid]
                    if cls:
                        obj = cls.newObject()
                        obj.port = port
                        self.wkpf.addProperties(obj.port, obj.cls)
                        self.objects.append(obj)
            except:
                traceback.print_exc()
                print "Can not find class %d" % clsid

    def wkpf_schedule_next_update_for_wuobject(self, obj):
        for i in range(int(obj.cls.propertyNumber)):
            if obj.cls.WKPF_GET_PROPERTY_DATATYPE(obj.cls.props_datatype_and_access[i]) == WKPF.DATATYPE_REFRESH:
                p = self.wkpf.properties[obj.port][i]
                obj.refresh_rate = p['value']
                if obj.refresh_rate == 0:
                    obj.next_scheduled_update = 0
                else:
                    obj.next_scheduled_update = obj.refresh_rate + int(round(time.time() *1000))
                return

    def updateRefreshRateObject(self):
        for obj in self.objects:
            #print obj.port
            #print self.wkpf.properties
            if obj.refresh_rate > 0 and obj.next_scheduled_update < int(round(time.time() *1000)):
                self.wkpf_schedule_next_update_for_wuobject(obj)
                obj.cls.update(obj, None, None)
        reactor.callLater(0, self.updateRefreshRateObject)

    def updateTheNextDirtyObject(self):
        for obj in self.objects:
            for i in range(0,len(self.wkpf.properties[obj.port])):
                p = self.wkpf.properties[obj.port][i]
                if p['dirty'] == True:
                    p['dirty'] = False
                    try:
                        obj.cls.update(obj,i,p['value'])
                    except:
                        traceback.print_exc()
                        pass
        reactor.callLater(0.3, self.updateTheNextDirtyObject)

    def getPortClassID(self,port):
        return self.objects[port].cls.ID

    def addClass(self,cls,flags):
        self.classes[cls.ID] = cls
        self.classtypes[cls.ID] =  flags
        cls.wkpf = self.wkpf
    def addObject(self,ID):
        cls = self.classes[ID]
        if cls:
            obj = cls.newObject()
            obj.port = len(self.objects)+1
            self.wkpf.addProperties(obj.port, obj.cls)
            self.objects.append(obj)
            self.wkpf_schedule_next_update_for_wuobject(obj)
            return obj
        return None
    def setProperty(self,pID, val):
        self.wkpf.setProperty(pID,val)
    def getProperty(self,pID):
        return self.wkpf.getProperty(port,pID)
