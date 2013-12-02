#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import sys
sys.path.append('./stomp.py-4.0.3')
import stomp


class WK_Listener(object):
	def __init__(self):
		self.mode = 0
		super(WK_Listener, self).__init__()

	def on_error(self, headers, message):
		print('received an error %s' % message)
	# when receive a message from MCHESS, do the on_message()
	def on_message(self, headers, message):
		self.mode = message
		print('received a message "%s"' % message)


class MCHESS:
	def __init__(self):
		self.conn = None

	def setup(self):
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
		self.conn.subscribe(destination='/topic/ssh.WK', id=1, ack='auto')
		# 5. the following is just showing the way to use a func to send msg to MQ
		self.conn.send(body='1,2,3,4', destination='/topic/ssh.WK')
		# 6. hang on the program for 5 seconds so that you could see the listener receive and print the msg
		time.sleep(5)

	def update(self, properties, links):
		locations = ['1', '2', '3', '4']
		
		modeStr = self.listener.mode
		modes = modeStr.split(',')
		print modes

		
		for i in range(len(modes)):
			location = locations[i]

			for link in links.to():
				compoment = link.compoment
				property = link.property

				if compoment.name == 'Dimmer' and compoment.location == location:
					if property.name == 'level':
						property.set(123)

			
			



if __name__ == '__main__':
	m = MCHESS()
	m.setup()
	m.update()
