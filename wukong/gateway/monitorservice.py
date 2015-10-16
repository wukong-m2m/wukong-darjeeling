import sys
try:
    from pymongo import MongoClient
except:
    print "Please install python mongoDB driver pymongo by using"
    print "easy_install pymongo"
    sys.exit(-1)

import gevent
from gevent.queue import Queue
from txCarbonClient import CarbonClientService
import json
import ast
import datetime
import time
from pserverclient import ProgressionServerClient
import color_logging, logging
logger = logging
import gtwconfig as config

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
            return SensorData(node_id, class_id, port, value, time.strftime("%Y%m%d%H%M%S"))
        return None

    @classmethod
    def createByCollection(self, document):
        return SensorData(document['node_id'], document['wuclass_id'], document['port'],
                          document['value'], document['timestamp'])

    def toDocument(self):
        return json.dumps({'node_id': self.node_id, 'wuclass_id': self.wuclass_id, 'port':
                           self.port, 'value': self.value, 'timestamp': self.timestamp })

    def graphite_key(self):
        return 'Wudevice' + self.node_id + '.Wuclass' + wuclass_id + '.Port' + port

class MonitorService(object):
    def __init__(self):
        try:
            self._mongodb_client = MongoClient(config.MONGODB_URL)
        except:
            print "MongoDB instance " + url + " can't be connected."
            print "Please install the mongDB, pymongo module."
            sys.exit(-1)
        print "MongoDB init"

        try:
            self._graphite_client = CarbonClientService(config.GRAPHITE_IP, config.GRAPHITE_PORT)
        except:
            print "Graphite instance on " + config.GRAPHITE_IP + ":" + config.GRAPHITE_PORT + " can't be connected";
            print "Please install the txCarbonClient module."
        print "Graphite Client init"

        self._pserver_client = ProgressionServerClient() if config.ENABLE_PROGRESSION else None
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
                if config.ENABLE_PROGRESSION:
                    self._pserver_client.send(data_collection.node_id, data_collection.port, data_collection.value)
                if config.ENABLE_GRAPHITE:
                    self._graphite_client.publish_metric(config.SYSTEM_NAME + "." + data_collection.graphite_key(), data_collection.value)

            gevent.sleep(0.001)
