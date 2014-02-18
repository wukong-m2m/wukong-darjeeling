import gevent
from gevent.event import Event
import signal

import gtwconfig as CONST
from broker import Broker
import zw
import ip
import rpcservice
import relayservice

_broker = None

def gateway_init():
    global _broker
    # Broker
    _broker = Broker()

    # IP protocol interface
    i_ip = ip.IPInterface(('localhost', CONST.IP_PORT), 'IP')
    _broker.register_interface(i_ip)

    # RPC protocol service
    s_rpc = rpcservice.RPCService('RPC', _broker.get_interface, _broker.get_all_interfaces())
    _broker.register_service(s_rpc)

    # ZWave protocol interface
    i_zw = zw.ZWInterface(CONST.ZWAVE_ADDR, 'ZW')
    _broker.register_interface(i_zw)

    # Master setup service
    s_relay = relayservice.RelayService('RELAY')
    _broker.register_service(s_relay)

def gateway_start():
    global _broker

    shutdown_event = Event()

    _broker.start()
    print "[Gateway] Started"

    # Signal Handling for graceful stop
    def signal_handler():
        print "\n\n*** WARNING ***"
        print "[Gateway] Shutdown signal received"
        _broker.close()
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
    gateway_start()