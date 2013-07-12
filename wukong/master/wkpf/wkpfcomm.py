import sys, time, copy
from transport import *
from locationTree import *
from models import *
from globals import *
from configuration import *

# MUST MATCH THE SIZE DEFINED IN wkcomm.h
WKCOMM_MESSAGE_PAYLOAD_SIZE=40

WKPF_PROPERTY_TYPE_SHORT         = 0
WKPF_PROPERTY_TYPE_BOOLEAN       = 1
WKPF_PROPERTY_TYPE_REFRESH_RATE  = 2

# routing services here
class Communication:
    _communication = None
    @classmethod
    def init(cls):
      if not cls._communication:
        cls._communication = Communication()
      return cls._communication

    def __init__(self):
      self.all_node_infos = []
      self.broker = getAgent()
      self.zwave = getZwaveAgent()
      if not self.zwave:
        print 'cannot initiate zwaveagent'
        raise Exception

    def addActiveNodesToLocTree(self, locTree):
      for node_info in self.getActiveNodeInfos():
        print 'active node_info', node_info
        locTree.addSensor(SensorNode(node_info))

    def verifyWKPFmsg(self, messageStart, minAdditionalBytes):
      # minPayloadLength should not include the command or the 2 byte sequence number
      return lambda command, payload: (command == pynvc.WKPF_ERROR_R) or (payload != None and payload[0:len(messageStart)]==messageStart and len(payload) >= len(messageStart)+minAdditionalBytes)

    def getNodeIds(self):
      return self.zwave.discovery()

    def getActiveNodeInfos(self, force=False):
      logging.info('getActiveNodeInfos 2')

      set_wukong_status("Discovery: Requesting node info")

      self.all_node_infos = self.getAllNodeInfos(force=force)

      set_wukong_status("")
      return filter(lambda item: item.isResponding(), self.all_node_infos)

    def getNodeInfos(self, node_ids):
      print 'getNodeInfos', node_ids
      if self.all_node_infos:
        return filter(lambda info: info.id in node_ids, self.all_node_infos)
      else:
        return [self.getNodeInfo(int(destination)) for destination in node_ids]

    def getAllNodeInfos(self, force=False):
      if force or self.all_node_infos == []:
        nodeIds = self.getNodeIds()
        self.all_node_infos = [self.getNodeInfo(int(destination)) for destination in nodeIds]
      print 'got all nodeInfos'
      return self.all_node_infos

    def getRoutingInformation(self):
      return self.zwave.routing()

    def onAddMode(self):
      return self.zwave.add()

    def onDeleteMode(self):
      return self.zwave.delete()

    def onStopMode(self):
      return self.zwave.stop()
        
    def currentStatus(self):
      return self.zwave.poll()

    def getNodeInfo(self, destination):
      print 'getNodeInfo of node id', destination

      location = self.getLocation(destination)
      node = WuNode.create(destination, location)
      gevent.sleep(0) # give other greenlets some air to breath

      wuClasses = self.getWuClassList(destination)
      gevent.sleep(0)

      wuObjects = self.getWuObjectList(destination)
      gevent.sleep(0)

      return node

    def getLocation(self, destination):
      print 'getLocation', destination

      length = 0
      location = ''

      while (length == 0 or len(location) < length): # There's more to the location string, so send more messages to get the rest
        # +1 because the first byte in the data stored on the node is the location string length
        offset = len(location) + 1 if length > 0 else 0
        reply = self.zwave.send(destination, pynvc.WKPF_GET_LOCATION, [offset], [pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          return ''
        if reply.command == pynvc.WKPF_ERROR_R:
          print "WKPF RETURNED ERROR ", reply.command
          return '' # graceful degradation
        if len(reply.payload) <= 2:
          return ''

        if length == 0:
          length = reply.payload[2] # byte 3 in the first message is the total length of the string
          if length == 0:
            return ''
          location = ''.join([chr(byte) for byte in reply.payload[3:]])
        else:
          location += ''.join([chr(byte) for byte in reply.payload[2:]])

      return location[0:length] # The node currently send a bit too much, so we have to truncate the string to the length we need

    def setLocation(self, destination, location):
      print 'setLocation', destination

      # Put length in front of location
      locationstring = [len(location)] + [int(ord(char)) for char in location]
      offset = 0
      chunksize = 10
      while offset < len(locationstring):
        chunk = locationstring[offset:offset+chunksize]
        message = [offset, len(chunk)] + chunk
        offset += chunksize

        reply = self.zwave.send(destination, pynvc.WKPF_SET_LOCATION, message, [pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          return -1

        if reply.command == pynvc.WKPF_ERROR_R:
          print "WKPF RETURNED ERROR ", reply.payload
          return False
      return True

    def getFeatures(self, destination):
      print 'getFeatures'

      reply = self.zwave.send(destination, pynvc.WKPF_GET_FEATURES, [], [pynvc.WKPF_GET_FEATURES_R, pynvc.WKPF_ERROR_R])


      if reply == None:
        return ""

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.command
        return [] # graceful degradation

      print reply
      return reply[3:]

    def setFeature(self, destination, feature, onOff):
      print 'setFeature'

      reply = self.zwave.send(destination, pynvc.WKPF_SET_FEATURE, [feature, onOff], [pynvc.WKPF_SET_FEATURE_R, pynvc.WKPF_ERROR_R])
      print reply


      if reply == None:
        return -1

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return False
      return True

    def getWuClassList(self, destination):
      print 'getWuClassList'

      set_wukong_status("Discovery: Requesting wuclass list from node %d" % (destination))

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUCLASS_LIST, [], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R])

      print 'Respond received'

      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuclasses = []
      reply = reply.payload[3:]
      while len(reply) > 1:
        wuclass_id = (reply[0] <<8) + reply[1]
        virtual = reply[2] == 1

        node = WuNode.find(id=destination)

        if not node:
          print 'Unknown node id', destination
          break

        wuclassdef = WuClassDef.find(id=wuclass_id)

        if not wuclassdef:
          print 'Unknown wuclass id', wuclass_id
          break

        wuclass = WuClass.find(node_identity=node.identity,
            wuclassdef_identity=wuclassdef.identity)

        # Create one
        if not wuclass:
          wuclass = WuClass.create(wuclassdef, node, virtual)
          # No need to recreate property definitions, as they are already
          # created when parsing XML

        wuclasses.append(wuclass)
        reply = reply[3:]
      return wuclasses

    def getWuObjectList(self, destination):
      print 'getWuObjectList'

      set_wukong_status("Discovery: Requesting wuobject list from node %d" % (destination))

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUOBJECT_LIST, [], [pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R])

      print 'Respond received'


      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuobjects = []
      reply = reply.payload[3:]
      while len(reply) > 1:
        wuclass_id = (reply[1] <<8) + reply[2]
        port_number = reply[0]
        node = WuNode.find(id=destination)

        if not node:
          print 'Unknown node id', destination
          break

        wuclassdef = WuClassDef.find(id=wuclass_id)

        if not wuclassdef:
          print 'Unknown wuclass id', wuclass_id
          break

        wuclass = WuClass.find(node_identity=node.identity,
            wuclassdef_identity=wuclassdef.identity)

        if not wuclass:
          print 'Unknown wuclass, should call getWuClassList before WuObjectList'
          break

        wuobject = WuObject.find(wuclass_identity=wuclass.identity)

        # Create one
        if not wuobject:
          wuobject = WuObject.create(port_number, wuclass)

        wuobjects.append(wuobject)
        reply = reply[3:]
      return wuobjects

    # Only used by inspector
    def getProperty(self, wuproperty):
      print 'getProperty'

      wuobject = wuproperty.wuobject()
      wuclass = wuobject.wuclass()
      wunode = wuobject.wunode()
      value = wuproperty.value
      datatype = wuproperty.datatype
      number = wuproperty.number

      reply = self.zwave.send(wunode.id, 
              pynvc.WKPF_READ_PROPERTY,
              [wuobject.port_number, wuclass.id/256, 
                    wuclass.id%256, number], 
              [pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R])


      if reply == None:
        return (None, None, None)

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return (None, None, None)

      # compatible
      reply = [reply.command] + reply.payload

      datatype = reply[7]
      status = reply[8]
      if datatype == WKPF_PROPERTY_TYPE_BOOLEAN:
        value = reply[9] != 0
      elif datatype == WKPF_PROPERTY_TYPE_SHORT or datatype == WKPF_PROPERTY_TYPE_REFRESH_RATE:
        value = (reply[9] <<8) + reply[10]
      else:
        value = None
      return (value, datatype, status)

    def setProperty(self, wuproperty):
      print 'setProperty'
      master_busy()

      wuobject = wuproperty.wuobject()
      wuclassdef = wuobject.wuclass().wuclassdef()
      wunode = wuobject.wunode()
      value = wuproperty.value
      datatype = wuproperty.datatype
      number = wuproperty.wupropertydef().number

      if datatype == 'boolean':
        datatype = WKPF_PROPERTY_TYPE_BOOLEAN

      elif datatype == 'short':
        datatype = WKPF_PROPERTY_TYPE_SHORT

      elif datatype == 'refresh_rate':
        datatype = WKPF_PROPERTY_TYPE_REFRESH_RATE

      if datatype == WKPF_PROPERTY_TYPE_BOOLEAN:
        payload=[wuobject.port_number, wuclassdef.id/256,
        wuclassdef.id%256, number, datatype, 1 if value else 0]

      elif datatype == WKPF_PROPERTY_TYPE_SHORT or datatype == WKPF_PROPERTY_TYPE_REFRESH_RATE:
        payload=[wuobject.port_number, wuclassdef.id/256,
        wuclassdef.id%256, number, datatype, value/256, value%256]

      reply = self.zwave.send(wunode.id, pynvc.WKPF_WRITE_PROPERTY, payload, [pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R])
      print 'getting reply from send command'


      master_available()
      if reply == None:
        print 'no reply received'
        return None

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return None
      master_available()

      print 'reply received'
      return reply

    def reprogram(self, destination, filename, retry=1):
      master_busy()
      
      if retry < 0:
        retry = 1

      ret = self.reprogramInfusion(destination, filename)
      while retry and not ret:
        print "Retrying after 5 seconds..."
        time.sleep(5)
        ret = self.reprogramInfusion(destination, filename)
        retry -= 1
      master_available()
      return ret

    def reprogramInfusion(self, destination, filename):
      REPRG_CHUNK_SIZE = WKCOMM_MESSAGE_PAYLOAD_SIZE - 2 # -2 bytes for the position

      bytecode = []
      with open(filename, "rb") as f:
        byte = f.read(1)
        while byte != "":
          bytecode.append(ord(byte))
          byte = f.read(1)

      infusion_length = len(bytecode)
      if infusion_length == 0:
        print "Can't read infusion file"
        return False

      # Start the reprogramming process
      print "Sending REPRG_OPEN command with image size ", len(bytecode)
      reply = self.zwave.send(destination, pynvc.REPRG_DJ_OPEN, [len(bytecode) >> 8 & 0xFF, len(bytecode) & 0xFF], [pynvc.REPRG_DJ_OPEN_R])

      if reply == None:
        print "No reply from node to REPRG_OPEN command"
        return False

      if reply.payload[2] != pynvc.REPRG_DJ_RETURN_OK:
        print "Got error in response to REPRG_OPEN: " + reply.payload[2]

      pagesize = reply.payload[3] + reply.payload[4]*256

      print "Uploading", len(bytecode), "bytes."

      pos = 0
      while not pos == len(bytecode):
        payload_pos = [pos%256, pos/256]
        payload_data = bytecode[pos:pos+REPRG_CHUNK_SIZE]
        print "Uploading bytes", pos, "to", pos+REPRG_CHUNK_SIZE, "of", len(bytecode)
        print pos/pagesize, (pos+len(payload_data))/pagesize, "of pagesize", pagesize
        if pos/pagesize == (pos+len(payload_data))/pagesize:
          self.zwave.send(destination, pynvc.REPRG_DJ_WRITE, payload_pos+payload_data, [])
          pos += len(payload_data)
        else:
          print "Send last packet of this page and wait for a REPRG_DJ_WRITE_R after each full page"
          reply = self.zwave.send(destination, pynvc.REPRG_DJ_WRITE, payload_pos+payload_data, [pynvc.REPRG_DJ_WRITE_R])
          print "Reply: ", reply
          if reply == None:
            print "No reply received. Code update failed. :-("
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_OK:
            print "Received REPRG_DJ_RETURN_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_REQUEST_RETRANSMIT:
            pos = reply.payload[3] + reply.payload[4]*256
            print "===========>Received REPRG_DJ_WRITE_R_RETRANSMIT request to retransmit from ", pos
          else:
            print "Unexpected reply:", reply.payload
            return False
        if pos == len(bytecode):
          print "Send REPRG_DJ_COMMIT after last packet"
          reply = self.zwave.send(destination, pynvc.REPRG_DJ_COMMIT, [pos%256, pos/256], [pynvc.REPRG_DJ_COMMIT_R])
          print "Reply: ", reply
          if reply == None:
            print "No reply, commit failed."
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_FAILED:
            print "Received REPRG_DJ_RETURN_FAILED, commit failed."
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_REQUEST_RETRANSMIT:
            pos = reply.payload[3] + reply.payload[4]*256
            print "===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos
            if pos >= len(bytecode):
              print "Received REPRG_DJ_RETURN_REQUEST_RETRANSMIT >= the image size. This shoudn't happen!"
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_OK:
            print "Commit OK.", reply.payload
          else:
            print "Unexpected reply:", reply.payload
            return False
      self.zwave.send(destination, pynvc.REPRG_DJ_REBOOT, [], [])
      print "Sent reboot.", reply.payload
      return True;

    def reprogramNvmdefault(self, destination, filename):
      print "Reprogramming Nvmdefault..."
      MESSAGESIZE = 16

      reply = self.zwave.send(destination, pynvc.REPRG_OPEN, [], [pynvc.REPRG_OPEN_R])

      if reply == None:
        print "No reply, abort"
        return False

      reply = [reply.command] + reply.payload[2:] # without the seq numbers

      pagesize = reply[1]*256 + reply[2]

      lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(filename).readlines() if l.startswith('0x')]
      bytecode = []
      for l in lines:
        for b in l.split():
          bytecode.append(int(b, 16))

      print "Uploading", len(bytecode), "bytes."

      pos = 0
      while not pos == len(bytecode):
        payload_pos = [pos/256, pos%256]
        payload_data = bytecode[pos:pos+MESSAGESIZE]
        print "Uploading bytes", pos, "to", pos+MESSAGESIZE, "of", len(bytecode)
        print pos/pagesize, (pos+len(payload_data))/pagesize, "of pagesize", pagesize
        if pos/pagesize == (pos+len(payload_data))/pagesize:
          #pynvc.sendcmd(destination, pynvc.REPRG_WRITE, payload_pos+payload_data)
          self.zwave.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [])
          pos += len(payload_data)
        else:
          print "Send last packet of this page and wait for a REPRG_WRITE_R_RETRANSMIT after each full page"
          reply = self.zwave.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [pynvc.REPRG_WRITE_R_OK, pynvc.REPRG_WRITE_R_RETRANSMIT])
          print "Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
          if reply == None:
            print "No reply received. Code update failed. :-("
            return False
          elif reply.command == pynvc.REPRG_WRITE_R_OK:
            print "Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.command == pynvc.REPRG_WRITE_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos

        if pos == len(bytecode):
          print "Send REPRG_COMMIT after last packet"
          reply = self.zwave.send(destination, pynvc.REPRG_COMMIT, [pos/256, pos%256], [pynvc.REPRG_COMMIT_R_RETRANSMIT, pynvc.REPRG_COMMIT_R_FAILED, pynvc.REPRG_COMMIT_R_OK])
          if reply == None:
            print "Commit failed."
            return False
          elif reply.command == pynvc.REPRG_COMMIT_R_OK:
            print reply.payload
            print "Commit OK."
          elif reply.command == pynvc.REPRG_COMMIT_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos

      reply = self.zwave.send(destination, pynvc.SETRUNLVL, [pynvc.RUNLVL_RESET], [pynvc.SETRUNLVL_R])

      if reply == None:
        print "Going to runlevel reset failed. :-("
        return False;
      else:
        return True;

def getComm():
  return Communication.init()
