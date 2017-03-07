#! -*- coding:utf-8 -*-
#
# A message queue from the Node-Red
#
import time,os,sys
class NodeRedMessage(object):
    def __init__(self,message_obj):
        self.message = message_obj
        self.ts = time.time()

class NodeRedMessageQueue(object):
    def __init__(self,seconds_to_life=3600):
        self.lastMessageFromNodeRed = None
        self.lastMessageToNodeRed = None
        self.ttl = seconds_to_life

    def addMessageFromNodeRed(self,nodeRedMessage):
        self.lastMessageFromNodeRed = nodeRedMessage

    def getMessageFromNodeRed(self):
        if not self.lastMessageFromNodeRed: return None
        if self.lastMessageFromNodeRed.ts+self.ttl >= time.time():
            return self.lastMessageFromNodeRed.message
        self.lastMessageFromNodeRed = None
        return None

    def addMessageToNodeRed(self,nodeRedMessage):
        self.lastMessageToNodeRed = nodeRedMessage

    def getMessageToNodeRed(self):
        if not self.lastMessageToNodeRed: return None
        if self.lastMessageToNodeRed.ts+self.ttl >= time.time():
            return self.lastMessageToNodeRed.message
        self.lastMessageToNodeRed = None
        return None
