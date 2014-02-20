import json

class SensorData:

    def __init__(self, node_id, wuclass_id, port, value, timestamp):
        self.node_id = node_id
        self.wuclass_id = wuclass_id
        self.port = port
        self.value = value
        self.timestamp = timestamp

    @classmethod
    def createByPayload(self, node_id, payload):
        port = payload[2]
        class_id = (payload[3] << 8) + payload[4]
        type = payload[6]
        if type == 1: #boolean
            value = payload[7]
        else:
            value = (payload[7] << 8) + payload[8]

        return SensorData(node_id, class_id, port, value, 0)

    @classmethod
    def createByCollection(self, document):

        return SensorData(document['node_id'], document['wuclass_id'], document['port'],
                          document['value'], document['timestamp'])

    def toDocument(self):

        return json.dumps({'node_id': self.node_id, 'wuclass_id': self.wuclass_id, 'port':
                           self.port, 'value': self.value, 'timestamp': self.timestamp })


