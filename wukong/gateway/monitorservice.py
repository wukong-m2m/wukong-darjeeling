import gevent
from gevent.queue import Queue, Empty
from gevent import monkey
monkey.patch_all()
import sys
try:
    from pymongo import MongoClient
except:
    print "Please install python mongoDB driver pymongo by using"
    print "easy_install pymongo"
    sys.exit(-1)

#from txCarbonClient import CarbonClientService
import json
import ast
import datetime
import time
from pserverclient import ProgressionServerClient
import color_logging, logging
logger = logging
import gtwconfig as config
if config.ENABLE_CONTEXT:
    import xmpp

if config.ENABLE_GRAPHITE:
    from twisted.internet import reactor
    from txCarbonClient import CarbonClientService

class UserData:
    def __init__(self):
        self.message = {}
        self.message['From']='Wukong'
        self.send = False
        self.cnt = 0

    def addData(self, sensorData):
        wuclass_id = sensorData.wuclass_id
        property_num = sensorData.property_num
        value = sensorData.value
        if value == 65535:
            value = -1
        table = [[1907, 'CabinetDoor', 'Slipper1', 'Slipper2', 'Slipper3'],
                 [1909, 'CabinetSpace1', 'CabinetSpace2', 'CabinetSpace3'],
                 [1910, 'PHONE'],
                 [2022, 'No','No','TV']]
        for wuclass in table:
            if wuclass_id == wuclass[0]:
                if wuclass[property_num+1]:
                    self.message[wuclass[property_num+1]] = value
                    self.send = True
                break

    def toDocument(self):
        self.message['id'] = self.cnt
        self.cnt += 1
        return json.dumps(self.message)

    def reset(self):
        self.message = {}
        self.message['From']='Wukong'
        self.send = False

class ContextData:
    def __init__(self):
        self.message = {}
        self.message['From']='Wukong'
        self.send = False

    def addData(self, sensorData):
        wuclass_id = sensorData.wuclass_id
        property_num = sensorData.property_num
        value = sensorData.value
        if value == 65535:
            value = -1
        table = [[9000, 'Floorlamp', 'Floorlamp_R', 'Floorlamp_G', 'Floorlamp_B', 'Floorlamp_Lux'],
                 [9001, 'Bloom', 'Bloom_R', 'Bloom_G', 'Bloom_B', 'Bloom_Lux'],
                 [9002, 'Go', 'Go_R', 'Go_G', 'Go_B', 'Go_Lux'],
                 [9003, 'Strip', 'Strip_R', 'Strip_G', 'Strip_B', 'Strip_Lux'],
                 [9004, 'Fan', 'Fan_Speed', 'Fan_Rotate'],
                 [9005, 'Mist'],
                 [9006, 'Music', 'Music_Type', 'Music_Vol'],
                 [9007, 'TV', 'TV_Mute'],
                 [9008, 'PHONE']]
        for wuclass in table:
            if wuclass_id == wuclass[0]:
                if wuclass[property_num+1]:
                    self.message[wuclass[property_num+1]] = value
                    self.send = True
                break

    def toDocument(self):
        return json.dumps(self.message)

    def reset(self):
        self.message = {}
        self.message['From']='Wukong'
        self.send = False

class SensorData:
    def __init__(self, node_id, wuclass_id, port, property_num, value, timestamp):
        self.node_id = node_id
        self.wuclass_id = wuclass_id
        self.port = port
        self.property_num  = property_num
        self.value = value
        self.timestamp = timestamp

    @classmethod
    def createByPayload(self, node_id, payload):
        if (len(payload) >= 7):
            class_id = (payload[2] << 8) + payload[3]
            port = payload[4]
            property_num = payload[5]
            type = payload[6]
            if type == 1: #boolean
                value = payload[7]
            else:
                value = (payload[7] << 8) + payload[8]
            return SensorData(node_id, class_id, port, property_num, value, time.strftime("%d%H%M%S"))
        return None

    @classmethod
    def createByCollection(self, document):
        return SensorData(document['node_id'], document['wuclass_id'], document['port'], document['property'],
                          document['value'], document['timestamp'])

    def toDocument(self):
        return json.dumps({'node_id': self.node_id, 'wuclass_id': self.wuclass_id, 'port':
                           self.port, 'property': self.property_num, 'value': self.value, 'timestamp': self.timestamp })

    def graphite_key(self):
        return 'Wudevice' + self.node_id + '.Wuclass' + wuclass_id + '.Port' + port

class MonitorService(object):
    def __init__(self):

        if config.ENABLE_MONGO:
            try:
                self._mongodb_client = MongoClient(config.MONGODB_URL)
            except:
                print "MongoDB instance " + url + " can't be connected."
                print "Please install the mongDB, pymongo module."
                sys.exit(-1)
            print "MongoDB init"

        if config.ENABLE_GRAPHITE:
            try:
                self._graphite_client = CarbonClientService(reactor, config.GRAPHITE_IP, config.GRAPHITE_PORT)
            except Exception as exception:
                print exception
                print "Graphite instance on " + config.GRAPHITE_IP + ":" + str(config.GRAPHITE_PORT) + " can't be connected";
                print "Please install the txCarbonClient module by using pip (pip install txCarbonClient)"
            print "Graphite Client init"

        self._pserver_client = ProgressionServerClient() if config.ENABLE_PROGRESSION else None
        self._task = Queue()
        if config.ENABLE_CONTEXT:
            self.xmpp_wait = False
            self.xmpp_user = UserData()
            #self.xmpp_context = ContextData()
            self.client = xmpp.Client(config.XMPP_SERVER)
            self.client.connect(server=(config.XMPP_IP, config.XMPP_PORT))
            self.client.auth(config.XMPP_USER, config.XMPP_PASS)
            self.client.sendInitPresence(requestRoster=0)
            mes = xmpp.Presence(to=config.XMPP_NICK)
            self.client.send(mes)
            self.client.Process(1)
            if config.ENABLE_PUB:
                self.client.RegisterHandler('message', self.message_handler)

    def handle_monitor_message(self, context, message):
        self._task.put_nowait((context, message))

    def serve_monitor(self):
        while True:
            try:
                context, message = self._task.get(timeout=1)
            except Empty:
        	if config.ENABLE_CONTEXT:
		    if self.xmpp_wait:
                        self.xmpp_wait = False
                        if self.xmpp_user.send:
                            message =  xmpp.Message(config.XMPP_ROOM, self.xmpp_user.toDocument())
                            message.setAttr('type', 'groupchat')
                            self.client.send(message)
                            self.xmpp_user.reset()
                        '''if self.xmpp_context.send:
                            message =  xmpp.Message(config.XMPP_ROOM, self.xmpp_context.toDocument())
                            message.setAttr('type', 'groupchat')
                            self.client.send(message)
                            self.xmpp_context.reset()'''
                    self.client.Process(1)
                continue

            data_collection = SensorData.createByPayload(context, message)
            logging.debug(data_collection.toDocument())
            if (data_collection != None):
                if config.ENABLE_MONITOR:
                    self._mongodb_client.wukong.readings.insert(ast.literal_eval(data_collection.toDocument()))

                if config.ENABLE_PROGRESSION:
                    self._pserver_client.send(data_collection.node_id, data_collection.port, data_collection.value)
                if config.ENABLE_GRAPHITE:
                    self._graphite_client.publish_metric(config.SYSTEM_NAME + "." + data_collection.graphite_key(), data_collection.value)
                if config.ENABLE_CONTEXT:
                    self.xmpp_wait = True
                    self.xmpp_user.addData(data_collection)
                    #self.xmpp_context.addData(data_collection)
            if config.ENABLE_CONTEXT:
                self.client.Process(1)
            gevent.sleep(0.001)
    def message_handler(self, sess, mess):
        text = mess.getBody()
        if text:
            print 'input: '+text
            try:
                input_data = json.loads(text)
                if input_data['From'] == 'CE':
                    iq = xmpp.Iq(typ='set', to='pubsub.'+config.XMPP_SERVER, xmlns=xmpp.NS_CLIENT)
                    iq.pubsub = iq.addChild(name='pubsub', namespace=xmpp.NS_PUBSUB)
                    iq.pubsub.publish = iq.pubsub.addChild(name='publish', attrs={'node': 'nooneknow'})
                    iq.pubsub.publish.item = iq.pubsub.publish.addChild(name='item', attrs={'id': '5'})
                    mess = iq.pubsub.publish.item.addChild(name='message')
                    if 'UserState1' in input_data:
                        mess.setData('{"topicId": "nooneknow", "Command_Mode": 1'+
                                     ', "UserA": '+str(input_data['UserState1'])+
                                     ', "UserB": '+str(input_data['UserState2'])+
                                     ', "UserC": '+str(input_data['UserState3'])+
                                     ', "UserD": '+str(input_data['UserState4'])+
                                     ', "UserG": '+str(input_data['UserState5'])+'}')
                        self.client.send(iq)
                        print 'user state'
                    elif 'State' in input_data:
                        mess.setData('{"topicId": "nooneknow", "Command_Mode": 2, "Context": "'+input_data['State']+'"}')
                        self.client.send(iq)
                        print 'state'
                    elif 'Preview' in input_data:
                        mess.setData('{"topicId": "nooneknow", "Command_Mode": 3'+
                                     ', "Floorlamp": '+str(input_data['Preview'][0])+
                                     ', "Floorlamp_R": '+str(input_data['Preview'][1])+
                                     ', "Floorlamp_G": '+str(input_data['Preview'][2])+
                                     ', "Floorlamp_B": '+str(input_data['Preview'][3])+
                                     ', "Floorlamp_Lux": '+str(input_data['Preview'][4])+
                                     ', "Bloom": '+str(input_data['Preview'][5])+
                                     ', "Bloom_R": '+str(input_data['Preview'][6])+
                                     ', "Bloom_G": '+str(input_data['Preview'][7])+
                                     ', "Bloom_B": '+str(input_data['Preview'][8])+
                                     ', "Bloom_Lux": '+str(input_data['Preview'][9])+
                                     ', "Go": '+str(input_data['Preview'][10])+
                                     ', "Go_R": '+str(input_data['Preview'][11])+
                                     ', "Go_G": '+str(input_data['Preview'][12])+
                                     ', "Go_B": '+str(input_data['Preview'][13])+
                                     ', "Go_Lux": '+str(input_data['Preview'][14])+
                                     ', "Strip": '+str(input_data['Preview'][15])+
                                     ', "Strip_R": '+str(input_data['Preview'][16])+
                                     ', "Strip_G": '+str(input_data['Preview'][17])+
                                     ', "Strip_B": '+str(input_data['Preview'][18])+
                                     ', "Strip_Lux": '+str(input_data['Preview'][19])+
                                     ', "Fan": '+str(input_data['Preview'][20])+
                                     ', "Fan_Speed": '+str(input_data['Preview'][21])+
                                     ', "Fan_Rotate": '+str(input_data['Preview'][22])+
                                     ', "Mist": '+str(input_data['Preview'][23])+
                                     ', "Music": '+str(input_data['Preview'][24])+
                                     ', "Music_Type": '+str(input_data['Preview'][25])+
                                     ', "Music_Vol": '+str(input_data['Preview'][26])+
                                     ', "TV": '+str(input_data['Preview'][27])+
                                     ', "TV_Mute": '+str(input_data['Preview'][28])+
                                     '}')
                        self.client.send(iq)
                        print 'preview'
            except Exception, e:
                print str(e)
                return
