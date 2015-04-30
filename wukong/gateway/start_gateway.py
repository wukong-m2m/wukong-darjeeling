try:
    import gevent
except:
    print "Please install the bitarray module from pypi"
    print "e.g. sudo pip install gevent"
    exit(-1)

from gevent.event import Event
import signal

import gtwconfig as CONFIG
from gtwclass import Gateway
import zwave
import udp2 as udp

import color_logging, logging
logger = logging

def main():
    shutdown_event = Event()


    if CONFIG.TRANSPORT_INTERFACE_TYPE.lower() == 'zwave':
        transport_interface = zwave.ZWTransport(CONFIG.TRANSPORT_INTERFACE_ADDR, "zwave")
    elif CONFIG.TRANSPORT_INTERFACE_TYPE.lower() == 'udp':
        transport_interface = udp.UDPTransport(CONFIG.TRANSPORT_INTERFACE_ADDR, "udp")
    elif CONFIG.TRANSPORT_INTERFACE_TYPE.lower() == 'zigbee':
        raise NotImplementedError
        # transport_interface = zigbee.ZBTransport(CONFIG.TRANSPORT_DEV_ADDR, "zigbee")
    else:
        logger.error("Unsupported type of transport interface")
        exit(-1)

    g = Gateway(transport_interface)

    # run
    if not g.start(CONFIG.SELF_TCP_SERVER_PORT):
        logger.error("Fail to start. Going down.")
        exit(0)

    # Signal Handling for graceful stop
    def signal_handler():
        logger.warn("Shutdown signal received")
        g.stop()
        shutdown_event.set()

    gevent.signal(signal.SIGTERM, signal_handler)
    gevent.signal(signal.SIGQUIT, signal_handler)
    gevent.signal(signal.SIGINT, signal_handler)
    #gevent.signal(signal.SIGUSR1, signal_handler)
    #gevent.signal(signal.SIGHUP, signal_handler)

    shutdown_event.wait()
    logger.warn("Going down.")

if __name__ == "__main__":
    main()
