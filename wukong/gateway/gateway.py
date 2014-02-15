import gevent
from gevent.event import Event
import signal

import gtwconfig as CONST
from broker import Broker
import tcp
import rpcservice
import udp 
import zw

def gateway_init():
    shutdown_event = Event()

    # Broker
    broker = Broker()
    broker.start()

    # TCP protocol interface
    i_tcp = tcp.TCPInterface(('localhost', CONST.TCP_PORT), 'TCP0', broker)
    i_tcp.start()

    # RPC protocol service
    s_rpc = rpcservice.RPCService(broker)
    s_rpc.start()

    # UDP protocol interface
    i_udp = udp.UDPInterface(('localhost', CONST.UDP_PORT), 'UDP0', broker)
    i_udp.start()

    # ZWave protocol interface
    i_zw = zw.ZWInterface(CONST.ZWAVE_ADDR, 'ZW0', broker)
    i_zw.start()

    # Signal Handling for graceful stop
    def signal_handler():
        print "\n\n*** WARNING ***"
        print "[Gateway] Shutdown signal received"
        broker.close()
        s_rpc.close()
        i_tcp.close()
        i_udp.close()
        i_zw.close()
        shutdown_event.set()

    gevent.signal(signal.SIGTERM, signal_handler)
    gevent.signal(signal.SIGQUIT, signal_handler)
    gevent.signal(signal.SIGINT, signal_handler)
    #gevent.signal(signal.SIGUSR1, signal_handler)
    #gevent.signal(signal.SIGHUP, signal_handler)

    shutdown_event.wait()
    print "[Gateway] Shuting down"

if __name__ == "__main__":

    gateway_init()