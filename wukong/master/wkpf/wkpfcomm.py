# vi: ts=2 sw=2 expandtab
import sys, time, copy
from model.locationTree import *
from model.models import *
from globals import *
from configuration import *
import simulator
if WKPFCOMM_AGENT == "GATEWAY":
  from transportv2 import *
else:
  from transport import *

# MUST MATCH THE SIZE DEFINED IN wkcomm.h
WKCOMM_MESSAGE_PAYLOAD_SIZE=30

WKPF_PROPERTY_TYPE_SHORT         = 0
WKPF_PROPERTY_TYPE_BOOLEAN       = 1
WKPF_PROPERTY_TYPE_REFRESH_RATE  = 2
OBJECTS_IN_MESSAGE               = (WKCOMM_MESSAGE_PAYLOAD_SIZE-3)/4
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
      self.device_type = None
      try:
        if SIMULATION == "true":
          print "simulation mode"
          raise Exception('simulation', 'Set simulation in master.cfg to false if you do not want simulated discovery')
        if WKPFCOMM_AGENT == "GATEWAY":
          self.agent = getGatewayAgent()
        elif WKPFCOMM_AGENT == "NETWORKSERVER":
          self.agent = getNetworkServerAgent()
        else:
          self.agent = getZwaveAgent()
      except Exception as e:
        print "Exception while creating agent"
        print e
        is_not_connected()
        self.agent = getMockAgent()
        if SIMULATION == "true":
              self.simulator = simulator.MockDiscovery()
              print '[wkpfcomm]running in simulation mode, discover result from mock_discovery.xml'
      self.routing = None

    def addActiveNodesToLocTree(self, locTree):
      for node_info in self.getActiveNodeInfos():
        print '[wkpfcomm] active node_info', node_info
        locTree.addSensor(SensorNode(node_info))

    def verifyWKPFmsg(self, messageStart, minAdditionalBytes):
      # minPayloadLength should not include the command or the 2 byte sequence number
      return lambda command, payload: (command == pynvc.WKPF_ERROR_R) or (payload is not None and payload[0:len(messageStart)]==messageStart and len(payload) >= len(messageStart)+minAdditionalBytes)

    def getNodeIds(self):
      if SIMULATION == "true":
        return self.simulator.discovery()
      return self.agent.discovery()

    def getActiveNodeInfos(self, force=False):
      #set_wukong_status("Discovery: Requesting node info")
      return filter(lambda item: item.isResponding(), self.getAllNodeInfos(force=force))

    def getNodeInfos(self, node_ids):
      return filter(lambda info: info.id in node_ids, self.getAllNodeInfos())

    def getAllNodeInfos(self, force=False):
      if force == False:
        self.all_node_infos = WuNode.loadNodes()
        if self.all_node_infos == None:
          print ('[wkpfcomm] error in cached discovery result')
      if force == True or self.all_node_infos == None:
        print '[wkpfcomm] getting all nodes from node discovery'
        WuNode.clearNodes()
        self.all_node_infos = [self.getNodeInfo(int(destination)) for destination in self.getNodeIds()]
        self.all_node_infos = self.all_node_infos + WuSystem.getVirtualNodes().values()
        WuNode.saveNodes()
      return self.all_node_infos

    def updateAllNodeInfos(self):
      nodelist = self.getNodeIds()
      newlist=[]
      for ID in nodelist:
        ID = int(ID)
        found = False
        for info in self.all_node_infos:
          print [info.id,ID]
          if info.id == ID:
            found = True
            newlist.append(info)
            break
        if found == False:
          newlist.append(self.getNodeInfo(ID))
      print [newlist]

      self.all_node_infos = newlist

      return copy.deepcopy(self.all_node_infos)

    def getRoutingInformation(self):
      if self.routing == None:
        self.routing = self.agent.routing()
      return self.routing

    def onAddMode(self):
      return self.agent.add()

    def onDeleteMode(self):
      return self.agent.delete()

    def onStopMode(self):
      return self.agent.stop()

    def currentStatus(self):
      return self.agent.poll()

    def getNodeInfo(self, destination):
      print '[wkpfcomm] getNodeInfo of node id', destination

      (basic,generic,specific) = self.getDeviceType(destination)
      print "basic=", basic
      print "generic=", generic
      print "specific=", specific
      if generic == 0xff or generic == 0x02:
        wunode = WuNode.findById(destination)
        location = self.getLocation(destination)
        location = ''.join(c for c in location if ord(c) <= 126 and ord(c) >= 32)
        gevent.sleep(0) # give other greenlets some air to breath
        if not wunode:
          wunode = WuNode(destination, location)
        retries=3
        while retries > 0:
          wuClasses = self.getWuClassList(destination)
          if wuClasses == None:
            retries=retries-1
          else:
            break
        else:
          return wunode

          
        print '[wkpfcomm] get %d wuclasses' % (len(wuClasses))
        wunode.wuclasses = wuClasses
        gevent.sleep(0)
        retries=3
        while retries > 0 :
          wuObjects = self.getWuObjectList(destination)
          print '[wkpfcomm] get %d wuobjects' % (len(wuObjects))
          if wuObjects == None:
            retries=retries-1
          else:
            break
        else:
          return wunode


        wunode.wuobjects = wuObjects
        gevent.sleep(0)

      elif generic == 17:

        wuclassdef = WuObjectFactory.wuclassdefsbyid[2007]    # Dimmer

        if not wuclassdef:
          print '[wkpfcomm] Unknown device type', generic
          return None
        wunode = WuNode(destination, None,type='native')
        port_number =1

        # Create one
        if (WuObject.ZWAVE_DIMMER1 not in wunode.wuobjects.keys()) or wuobjects[port].wuclassdef != wuclassdef:
          # 0x100 is a mgic number. When we see this in the code generator,
          # we will generate ZWave command table to implement the wuclass by
          # using the Z-Wave command.
          if specific == 1:
            wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, WuObject.ZWAVE_DIMMER1, False, property_values={})

          elif specific == 3:
            wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, WuObject.ZWAVE_DIMMER1, False, property_values={})

        if (WuObject.ZWAVE_DIMMER2 not in wunode.wuobjects.keys()) or wuobjects[port].wuclassdef != wuclassdef:
          # 0x100 is a mgic number. When we see this in the code generator,
          # we will generate ZWave command table to implement the wuclass by
          # using the Z-Wave command.
          if specific == 1:
            wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, WuObject.ZWAVE_DIMMER2, False, property_values={})

        if (WuObject.ZWAVE_DIMMER3 not in wunode.wuobjects.keys()) or wuobjects[port].wuclassdef != wuclassdef:
          # 0x100 is a mgic number. When we see this in the code generator,
          # we will generate ZWave command table to implement the wuclass by
          # using the Z-Wave command.
          if specific == 1:
            wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, WuObject.ZWAVE_DIMMER3, False, property_values={})

      else:
        # Create a virtual wuclass for non wukong device. We support switch only now.
        # We may support others in the future.

        wuclassdef = WuObjectFactory.wuclassdefsbyid[2001]    # Light_Actuator

        if not wuclassdef:
          print '[wkpfcomm] Unknown device type', generic
          return None
        wunode = WuNode(destination, None,type='native')
        port_number =1


        # Create one
        for k in [WuObject.ZWAVE_SWITCH_PORT1, WuObject.ZWAVE_SWITCH_PORT2, WuObject.ZWAVE_SWITCH_PORT3]:
          if (k not in wunode.wuobjects.keys()) or wuobjects[port].wuclassdef != wuclassdef:
            # 0x100 is a mgic number. When we see this in the code generator,
            # we will generate ZWave command table to implement the wuclass by
            # using the Z-Wave command.
            wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, k, False, property_values={})


      return wunode

    def getDeviceType(self, destination):
      self.device_type = self.agent.getDeviceType(destination)
      return self.device_type

    def getLocation(self, destination):
      print '[wkpfcomm] getLocation', destination
      #########This code is put here just in case we need to change some node location before get it.
      #Sometimes invalid locations block discovery, we have to correct it beforehand
      #comm = getComm()
      #if comm.setLocation(1, "WuKong")
      ##########################################

      length = 0
      location = ''
      retries=3
      if SIMULATION == "true":
          location = self.simulator.mockLocation(destination)
          return location
      while (length == 0 or len(location) < length): # There's more to the location string, so send more messages to get the rest
        # +1 because the first byte in the data stored on the node is the location string length
        offset = len(location) + 1 if length > 0 else 0
        reply = self.agent.send(destination, pynvc.WKPF_GET_LOCATION, [offset], [pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          retries=retries-1
          if retries == 0:
            return ''
          continue
        if reply.command == pynvc.WKPF_ERROR_R:
          print "[wkpfcomm] WKPF RETURNED ERROR ", reply.command
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
      print '[wkpfcomm] setLocation', destination

      # Put length in front of location
      locationstring = [len(location)] + [int(ord(char)) for char in location]
      offset = 0
      chunksize = 10
      while offset < len(locationstring):
        chunk = locationstring[offset:offset+chunksize]
        message = [offset, len(chunk)] + chunk
        offset += chunksize

        reply = self.agent.send(destination, pynvc.WKPF_SET_LOCATION, message, [pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          return -1

        if reply.command == pynvc.WKPF_ERROR_R:
          print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
          return False
      return True

    def getFeatures(self, destination):
      print '[wkpfcomm] getFeatures'

      reply = self.agent.send(destination, pynvc.WKPF_GET_FEATURES, [], [pynvc.WKPF_GET_FEATURES_R, pynvc.WKPF_ERROR_R])


      if reply == None:
        return ""

      if reply.command == pynvc.WKPF_ERROR_R:
        print "[wkpfcomm] WKPF RETURNED ERROR ", reply.command
        return [] # graceful degradation

      print '[wkpfcomm] ' + reply
      return reply[3:]

    def setFeature(self, destination, feature, onOff):
      print '[wkpfcomm] setFeature'

      reply = self.agent.send(destination, pynvc.WKPF_SET_FEATURE, [feature, onOff], [pynvc.WKPF_SET_FEATURE_R, pynvc.WKPF_ERROR_R])
      print '[wkpfcomm] ' + reply


      if reply == None:
        return -1

      if reply.command == pynvc.WKPF_ERROR_R:
        print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
        return False
      return True

    def getWuClassList(self, destination):
      print '[wkpfcomm] getWuClassList'

      #set_wukong_status("Discovery: Requesting wuclass list from node %d" % (destination))

      wuclasses = {}
      total_number_of_messages = None
      message_number = 0
      if SIMULATION == "true":
        return self.simulator.mockWuClassList(destination)

      while (message_number != total_number_of_messages):
        reply = self.agent.send(destination, pynvc.WKPF_GET_WUCLASS_LIST, [message_number], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R])

        message_number += 1

        print '[wkpfcomm] Respond received'
        if reply == None:
          return {}
        if reply.command == pynvc.WKPF_ERROR_R:
          print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
          return {}
        if total_number_of_messages is None:
          total_number_of_messages = reply.payload[3]

        reply = reply.payload[5:]
        print "reply=", reply
        while len(reply) > 1:
          wuclass_id = (reply[0] <<8) + reply[1]
          virtual_or_publish = reply[2]

          virtual = virtual_or_publish & 0x1
          publish = virtual_or_publish & 0x2

          #virtual wuclass, non-publish wuclass will not be shown upon discovery, because we cannot create new wuobjs using them
          #to create new virtual wuobjs, we need to re-download virtual wuclass...
          if publish and (not virtual):
            node = WuNode.findById(destination)

            if not node:
              print '[wkpfcomm] Unknown node id', destination
              break

            wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]

            if not wuclassdef:
              print '[wkpfcomm] Unknown wuclass id', wuclass_id
              break

            wuclasses[wuclass_id] = wuclassdef

          reply = reply[3:]

      return wuclasses

    def getWuObjectList(self, destination):
      print '[wkpfcomm] getWuObjectList'

      #set_wukong_status("Discovery: Requesting wuobject list from node %d" % (destination))

      wuobjects = {}
      total_number_of_messages = None
      total_number_of_wuobjects = None
      message_number = 0
      if SIMULATION == "true":
        return self.simulator.mockWuObjectList(destination)

      while (message_number != total_number_of_messages):
        reply = self.agent.send(destination, pynvc.WKPF_GET_WUOBJECT_LIST, [message_number], [pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R])


        print '[wkpfcomm] Respond received'
        if reply == None:
          return {}
        if reply.command == pynvc.WKPF_ERROR_R:
          print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
          return {}
        print '[wkpfcomm] Respond payload is', reply.payload
        if total_number_of_messages is None:
          total_number_of_messages = reply.payload[3]
        if total_number_of_wuobjects is None:
          total_number_of_wuobjects = reply.payload[4]
        index_of_message = reply.payload[2]
        expected_num_byte = 5
        if index_of_message < total_number_of_messages-1:
          expected_num_byte = expected_num_byte + 4*OBJECTS_IN_MESSAGE
        else:
          expected_num_byte = expected_num_byte + 4*(total_number_of_wuobjects-index_of_message*OBJECTS_IN_MESSAGE)
        if len(reply.payload) != expected_num_byte:
          continue

        message_number += 1

        reply = reply.payload[5:]
        while len(reply) > 1:
          print reply
          try:
            port_number = reply[0]
            wuclass_id = (reply[1] <<8) + reply[2]
            virtual = bool(int(reply[3]))
          except:
            print '[wkpfcomm] reply too short'
            return None
          node = WuNode.findById(destination)

          if not node:
            print '[wkpfcomm] Unknown node id', destination

          try:
            wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]
            if not wuclassdef:
              print '[wkpfcomm] Unknown wuclass id', wuclass_id
              break


            if (not node) or (port_number not in node.wuobjects.keys()) or node.wuobjects[port_number].wuclassdef != wuclassdef:
              wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number, virtual)
            else:
              wuobject = node.wuobjects[port_number]
            wuobjects[port_number] = wuobject
          except Exception as e:
            print "[wkpfcomm] incorrect wuclass id ", str(e)
            return None

          reply = reply[4:]


      return wuobjects

    def getProperty(self, id, port, wuclassid, property_number):
    # def getProperty(self, wuproperty):
      print '[wkpfcomm] getProperty'

      # wuobject = wuproperty.wuobject
      # wuclass = wuobject.wuclassdef
      # wunode = wuobject.wunode
      # value = wuproperty.value
      # datatype = wuproperty.datatype
      # number = wuproperty.number

      # reply = self.zwave.send(wunode.id,
      #         pynvc.WKPF_READ_PROPERTY,
      #         [wuobject.port_number, wuclass.id/256,
      #               wuclass.id%256, number],
      #         [pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R])
      reply = self.agent.send(id,
              pynvc.WKPF_READ_PROPERTY,
              [port, wuclassid/256,
                    wuclassid%256, property_number],
              [pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R])



      if reply == None:
        return (None, None, None)

      if reply.command == pynvc.WKPF_ERROR_R:
        print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
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

    def setProperty(self, id, port, wuclassid, property_number, datatype, value):
    # def setProperty(self, wuproperty):
      print '[wkpfcomm] setProperty'
      master_busy()

      # wuobject = wuproperty.wuobject
      # wuclassdef = wuobject.wuclassdef
      # wunode = wuobject.wunode
      # value = wuproperty.value
      # datatype = wuproperty.datatype
      # #number = wuproperty.wupropertydef.number

      if datatype == 'boolean':
        datatype = WKPF_PROPERTY_TYPE_BOOLEAN

      elif datatype == 'short':
        datatype = WKPF_PROPERTY_TYPE_SHORT

      elif datatype == 'refresh_rate':
        datatype = WKPF_PROPERTY_TYPE_REFRESH_RATE

      if datatype == WKPF_PROPERTY_TYPE_BOOLEAN:
        payload=[port, wuclassid/256,
        wuclassid%256, property_number, datatype, 1 if value else 0, 
        0, 0, 0, 0, 0]    
        # the last 5 0s are sender and receiver component id. set them to 0 to bypass property locking check.
        #Property locking check checks if the desired id and property is still the desired component in case of link change.
        #see wkpf_generate_piggyback_token() in wkpf_links.c 


      elif datatype == WKPF_PROPERTY_TYPE_SHORT or datatype == WKPF_PROPERTY_TYPE_REFRESH_RATE:
        payload=[port, wuclassid/256,
        wuclassid%256, property_number, datatype, value/256, value%256,
        0, 0, 0, 0, 0]    
        # the last 5 0s are sender and receiver component id. see wkpf_generate_piggyback_token() in wkpf_links.c 


      reply = self.agent.send(id, pynvc.WKPF_WRITE_PROPERTY, payload, [pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R])
      # reply = self.zwave.send(wunode.id, pynvc.WKPF_WRITE_PROPERTY, payload, [pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R])
      print '[wkpfcomm] getting reply from send command'


      master_available()
      if reply == None:
        print '[wkpfcomm] no reply received'
        return None

      if reply.command == pynvc.WKPF_ERROR_R:
        print "[wkpfcomm] WKPF RETURNED ERROR ", reply.payload
        return None
      master_available()

      print '[wkpfcomm] reply received'
      return reply

    def reprogram(self, destination, filename, retry=1):
      master_busy()

      if retry < 0:
        retry = 1

      ret = self.reprogramInfusion(destination, filename)
      while retry and not ret:
        print "[wkpfcomm] Retrying after 5 seconds..."
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
        print "[wkpfcomm] Can't read infusion file"
        return False

      # Start the reprogramming process
      print "[wkpfcomm] Sending REPRG_OPEN command with image size ", len(bytecode)
      reply = self.agent.send(destination, pynvc.REPRG_DJ_OPEN, [len(bytecode) & 0xFF, len(bytecode) >> 8 & 0xFF], [pynvc.REPRG_DJ_OPEN_R])

      if reply == None:
        print "[wkpfcomm] No reply from node to REPRG_OPEN command"
        return False

      if reply.payload[2] != pynvc.REPRG_DJ_RETURN_OK:
        print "[wkpfcomm] Got error in response to REPRG_OPEN: " + reply.payload[2]

      pagesize = reply.payload[3] + reply.payload[4]*256

      print "[wkpfcomm] Uploading", len(bytecode), "bytes."

      pos = 0
      while not pos == len(bytecode):
        payload_pos = [pos%256, pos/256]
        payload_data = bytecode[pos:pos+REPRG_CHUNK_SIZE]
        print "[wkpfcomm] Uploading bytes", pos, "to", pos+REPRG_CHUNK_SIZE, "of", len(bytecode)
        print '[wkpfcomm]', pos/pagesize, (pos+len(payload_data))/pagesize, "of pagesize", pagesize
        if pos/pagesize == (pos+len(payload_data))/pagesize:
          self.agent.send(destination, pynvc.REPRG_DJ_WRITE, payload_pos+payload_data, [])
          pos += len(payload_data)
        else:
          print "[wkpfcomm] Send last packet of this page and wait for a REPRG_DJ_WRITE_R after each full page"
          reply = self.agent.send(destination, pynvc.REPRG_DJ_WRITE, payload_pos+payload_data, [pynvc.REPRG_DJ_WRITE_R])
          print "[wkpfcomm] Reply: ", reply
          if reply == None:
            print "[wkpfcomm] No reply received. Code update failed. :-("
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_OK:
            print "[wkpfcomm] Received REPRG_DJ_RETURN_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_REQUEST_RETRANSMIT:
            pos = reply.payload[3] + reply.payload[4]*256
            print "[wkpfcomm] ===========>Received REPRG_DJ_WRITE_R_RETRANSMIT request to retransmit from ", pos
          else:
            print "[wkpfcomm] Unexpected reply:", reply.payload
            return False
        if pos == len(bytecode):
          print "[wkpfcomm] Send REPRG_DJ_COMMIT after last packet"
          reply = self.agent.send(destination, pynvc.REPRG_DJ_COMMIT, [pos%256, pos/256], [pynvc.REPRG_DJ_COMMIT_R])
          print "[wkpfcomm] Reply: ", reply
          if reply == None:
            print "[wkpfcomm] No reply, commit failed."
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_FAILED:
            print "[wkpfcomm] Received REPRG_DJ_RETURN_FAILED, commit failed."
            return False
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_REQUEST_RETRANSMIT:
            pos = reply.payload[3] + reply.payload[4]*256
            print "[wkpfcomm] ===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos
            if pos >= len(bytecode):
              print "[wkpfcomm] Received REPRG_DJ_RETURN_REQUEST_RETRANSMIT >= the image size. This shoudn't happen!"
          elif reply.payload[2] == pynvc.REPRG_DJ_RETURN_OK:
            print "[wkpfcomm] Commit OK.", reply.payload
          else:
            print "[wkpfcomm] Unexpected reply:", reply.payload
            return False
      self.agent.send(destination, pynvc.REPRG_DJ_REBOOT, [], [])
      print "[wkpfcomm] Sent reboot.", reply.payload
      return True;

    def reprogramNvmdefault(self, destination, filename):
      print "[wkpfcomm] Reprogramming Nvmdefault..."
      MESSAGESIZE = 16

      reply = self.agent.send(destination, pynvc.REPRG_OPEN, [], [pynvc.REPRG_OPEN_R])

      if reply == None:
        print "[wkpfcomm] No reply, abort"
        return False

      reply = [reply.command] + reply.payload[2:] # without the seq numbers

      pagesize = reply[1]*256 + reply[2]

      lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(filename).readlines() if l.startswith('0x')]
      bytecode = []
      for l in lines:
        for b in l.split():
          bytecode.append(int(b, 16))

      print "[wkpfcomm] Uploading", len(bytecode), "bytes."

      pos = 0
      while not pos == len(bytecode):
        payload_pos = [pos/256, pos%256]
        payload_data = bytecode[pos:pos+MESSAGESIZE]
        print "[wkpfcomm] Uploading bytes", pos, "to", pos+MESSAGESIZE, "of", len(bytecode)
        print '[wkpfcomm]', pos/pagesize, (pos+len(payload_data))/pagesize, "of pagesize", pagesize
        if pos/pagesize == (pos+len(payload_data))/pagesize:
          #pynvc.sendcmd(destination, pynvc.REPRG_WRITE, payload_pos+payload_data)
          self.agent.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [])
          pos += len(payload_data)
        else:
          print "[wkpfcomm] Send last packet of this page and wait for a REPRG_WRITE_R_RETRANSMIT after each full page"
          reply = self.agent.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [pynvc.REPRG_WRITE_R_OK, pynvc.REPRG_WRITE_R_RETRANSMIT])
          print "[wkpfcomm] Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
          if reply == None:
            print "[wkpfcomm] No reply received. Code update failed. :-("
            return False
          elif reply.command == pynvc.REPRG_WRITE_R_OK:
            print "[wkpfcomm] Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.command == pynvc.REPRG_WRITE_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "[wkpfcomm] ===========>Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos

        if pos == len(bytecode):
          print "[wkpfcomm] Send REPRG_COMMIT after last packet"
          reply = self.agent.send(destination, pynvc.REPRG_COMMIT, [pos/256, pos%256], [pynvc.REPRG_COMMIT_R_RETRANSMIT, pynvc.REPRG_COMMIT_R_FAILED, pynvc.REPRG_COMMIT_R_OK])
          if reply == None:
            print "[wkpfcomm] Commit failed."
            return False
          elif reply.command == pynvc.REPRG_COMMIT_R_OK:
            print '[wkpfcomm] ' + reply.payload
            print "[wkpfcomm] Commit OK."
          elif reply.command == pynvc.REPRG_COMMIT_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "[wkpfcomm] ===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos

      reply = self.agent.send(destination, pynvc.SETRUNLVL, [pynvc.RUNLVL_RESET], [pynvc.SETRUNLVL_R])

      if reply == None:
        print "[wkpfcomm] Going to runlevel reset failed. :-("
        return False;
      else:
        return True;

def getComm():
  return Communication.init()
