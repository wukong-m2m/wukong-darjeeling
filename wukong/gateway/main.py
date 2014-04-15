try:
    import gevent
except:
    print "Please install the bitarray module from pypi"
    print "e.g. sudo pip install gevent"
    exit(-1)

from gevent.event import Event
import signal

import gtwconfig as CONFIG
from gateway import Gateway
import zwave

import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

def main():
    shutdown_event = Event()


    if CONFIG.TRANSPORT_DEV_TYPE == 'zwave':
        transport_device = zwave.ZWTransport(CONFIG.TRANSPORT_DEV_ADDR, "zwave")
    elif CONFIG.TRANSPORT_DEV_TYPE == 'zigbee':
        raise NotImplementedError
        # transport_device = zigbee.ZBTransport(CONFIG.TRANSPORT_DEV_ADDR, "zigbee")
    elif CONFIG.TRANSPORT_DEV_TYPE == 'ip':
        raise NotImplementedError
        # transport_device = ip.UDPTransport(CONFIG.TRANSPORT_DEV_ADDR, "udp")

    g = Gateway(transport_device)

    # run
    if not g.start(CONFIG.SELF_TCP_PORT):
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