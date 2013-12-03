#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import sys
import stomp

from wuclass_helper import *


class WK_Listener(object):
  def __init__(self):
    self.mode = "50,50,50,50" # testing
    super(WK_Listener, self).__init__()

  def on_error(self, headers, message):
    print('received an error %s' % message)
    # when receive a message from MCHESS, do the on_message()
  def on_message(self, headers, message):
    self.mode = message
    print('received a message "%s"' % message)

class MCHESS(VirtualNodeWuClass):
  def setup(self):
    self.conn = None
    # 1. set the connection, the following ip(140.112.49.154) is our server which is always running.
    #    the port is 61613.
    self.conn = stomp.Connection(host_and_ports=[('140.112.49.154', 61613)])
    # 2. set the listener
    self.listener = WK_Listener()
    self.conn.set_listener('', self.listener)
    # 3. start and connect to the server
    self.conn.start()
    self.conn.connect()
    # 4. subscribe to the channel that we use it to communicate with WuKong
    self.conn.subscribe(destination='/topic/ssh.WK', id=1)
    # 5. the following is just showing the way to use a func to send msg to MQ
    #self.conn.send(body='1,2,3,4', destination='/topic/ssh.WK')
    # 6. hang on the program for 5 seconds so that you could see the listener receive and print the msg
    time.sleep(5)

  def update(self, virtualnode, properties, links):
    print 'mchess update'
    dim_levels = {'L1': 0, 'L2': 0, 'R1': 0, 'R2': 0}

    modeStr = self.listener.mode
    modes = modeStr.split(',')
    dim_levels['L1'] = int(modes[0])
    dim_levels['L2'] = int(modes[1])
    dim_levels['R1'] = int(modes[2])
    dim_levels['R2'] = int(modes[3])
    print 'dim_levels', dim_levels

		
    for link in links.outdegrees():
      print 'outdegree', link
      component = link.component
      property = link.property

      # FIXME: location matching is hard coded for now
      if component.type == 'Dimmer' and any([component.location.find(k) > -1 for k in dim_levels.keys()]):
        # FIXME: hardcoded
        location = component.location.split('/')[1][:-1]
        if property.name == 'level':
          print 'set property', property.name
          property.set(dim_levels[location])
        if property.name == 'on_off':
          print 'set property', property.name
          property.set(True)




if __name__ == '__main__':
  m = MCHESS()
  m.setup()
  m.update()
