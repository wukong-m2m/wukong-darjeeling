try:
    from pymongo import MongoClient
except:
    print "Please install python mongoDB driver pymongo by using"
    print "easy_install pymongo"
    sys.exit(-1)

import gevent
from gevent.queue import Queue
import json
import ast
import datetime

import gtwconfig as CONFIG
import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

class SensorData:
    def __init__(self, node_id, wuclass_id, port, value, timestamp):
        self.node_id = node_id
        self.wuclass_id = wuclass_id
        self.port = port
        self.value = value
        self.timestamp = timestamp

    @classmethod
    def createByPayload(self, node_id, payload):
        if (len(payload) >= 7):
            class_id = (payload[2] << 8) + payload[3]
            port = payload[4]
            type = payload[5]
            if type == 1: #boolean
                value = payload[6]
            else:
                value = (payload[6] << 8) + payload[7]
            now = datetime.datetime.now()
            return SensorData(node_id, class_id, port, value, now.year*10000000000+now.month*100000000+now.day*1000000+now.hour*10000+now.minute*100+now.second)
        return None

    @classmethod
    def createByCollection(self, document):
        return SensorData(document['node_id'], document['wuclass_id'], document['port'],
                          document['value'], document['timestamp'])

    def toDocument(self):
        return json.dumps({'node_id': self.node_id, 'wuclass_id': self.wuclass_id, 'port':
                           self.port, 'value': self.value, 'timestamp': self.timestamp })

class MonitorService(object):
    def __init__(self, url):
        try:
            self._mongodb_client = MongoClient(url)
        except:
            print "MongoDB instance " + url + " can't be connected."
            print "Please install the mongDB, pymongo module."
            sys.exit(-1)
        print "MongoDB init"
        self._task = Queue()

    def handle_monitor_message(self, context, message):
        self._task.put_nowait((context, message))

    def serve_monitor(self):
        while True:
            context, message = self._task.get()
            data_collection = SensorData.createByPayload(context, message)
            logging.debug(data_collection.toDocument())
            if (data_collection != None):
                self._mongodb_client.wukong.readings.insert(ast.literal_eval(data_collection.toDocument()))
            gevent.sleep(0)

