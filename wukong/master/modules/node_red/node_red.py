import tornado.ioloop, tornado.web
import tornado.template as template
from noderedmessagequeue import NodeRedMessage,NodeRedMessageQueue

global nodeRedMessageQueue
nodeRedMessageQueue = NodeRedMessageQueue()
class NodeRedInputFrom(tornado.web.RequestHandler):
    #
    # Store the message from NodeRed
    #
    def post(self):
        global nodeRedMessageQueue
        message= self.get_argument('message',default=None,strip=False)
        if not message:
            self.write({'status':1,'message':'No Message Received'})
            self.finish()
            return
        try:
            message_obj = json.loads(message)
            nodeRedMessageQueue.addMessageFromNodeRed(NodeRedMessage(message_obj))

        except :
            self.write({'status':0,'message':'Message Error:%s' % (traceback.format_exc())})
            self.finish()
            return

        self.write({'status':1,'message':'Message Received'})
        self.finish()

class NodeRedOutputTo(tornado.web.RequestHandler):
    #
    # Store the message to NodeRed
    #
    def post(self):
        global nodeRedMessageQueue
        message= self.get_argument('message',default=None,strip=False)
        if not message:
            self.write({'status':1,'message':'No Message Received'})
            self.finish()
            return
        try:
            message_obj = json.loads(message)
            nodeRedMessageQueue.addMessageToNodeRed(NodeRedMessage(message_obj))

        except :
            self.write({'status':0,'message':'Message Error:%s' % (traceback.format_exc())})
            self.finish()
            return

        self.write({'status':1,'message':'Message Received'})
        self.finish()

#
# This is a prototype to deliver live value of device to GUI
# NodeRedMessage is the sample to develop
#
class ReadMessageFromNodeRed(tornado.web.RequestHandler):
    def get(self,app_id):
        app_ind = getAppIndex(app_id)
        if app_ind == None:
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
            self.finish()
            return
        nodeId = self.get_argument('id','')
        nodeType = self.get_argument('type','')
        slotname = self.get_argument('slot','')
        value = ''
        if slotname == 'message':
            global nodeRedMessageQueue
            message = nodeRedMessageQueue.getMessageFromNodeRed()
            value = message['payload'] if message else None
        self.write({'status':1,'value': value})
        self.finish()
class ReadMessageToNodeRed(tornado.web.RequestHandler):
    def get(self):
        global nodeRedMessageQueue
        message = nodeRedMessageQueue.getMessageToNodeRed()
        self.write({'status':1,'payload': message})
        self.finish()
