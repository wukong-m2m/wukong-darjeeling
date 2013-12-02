#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import sys
sys.path.append('.')
import stomp

### IMPORTANT!!! ###
# just use the class and implement what you want to do in the
# func 'on_message()'.
# E.g. 
#       on_message(...):
#                       if (message == 'A_mode'):
#                           do_sth_for_A();
#                       ...
#
# !!!This is a independent thread to handle the received message!!!

class WK_Listener(object):
    def on_error(self, headers, message):
        print('received an error %s' % message)
    # when receive a message from MCHESS, do the on_message()
    def on_message(self, headers, message):
        print('received a message %s' % message)

# 1. set the connection, the following ip(140.112.49.154) is our server which is always running.
#    the port is 61613.
conn = stomp.Connection(host_and_ports=[('140.112.49.154', 61613)])
# 2. set the listener
conn.set_listener('', WK_Listener())
# 3. start and connect to the server
conn.start()
conn.connect()
# 4. subscribe to the channel that we use it to communicate with WuKong
conn.subscribe(destination='/topic/ssh.WK', id=1, ack='auto')
# 5. the following is just showing the way to use a func to send msg to MQ
conn.send(body=' '.join(sys.argv[1:]), destination='/topic/ssh.WK')
# 6. hang on the program for 5 seconds so that you could see the listener receive and print the msg
time.sleep(5)
# 7. disconnect the connection before program exits
#    for you, you don't need to use the func
conn.disconnect()
