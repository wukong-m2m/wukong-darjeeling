import serial, time, logging, sys
from xbee import ZigBee

logging.basicConfig(level=logging.DEBUG)
if not sys.flags.debug: logging.disable(logging.DEBUG)
DEBUGF = logging.debug

xbee = 0
addr = {0:'\x00\x13\xa2\x00\x40\x77\x33\xf3',
        1:'\x00\x13\xa2\x00\x40\x77\x34\x0d'}

def init():
    global xbee
    ser = serial.Serial('/dev/ttyUSB0',9600)
    xbee = ZigBee(ser, escaped=True)

def receive(waitmsec):
    packet = xbee.wait_read_frame_timeout(waitmsec)
    if packet == None:
        return None

    DEBUGF(("recv ZB packet:", packet))
    name = packet['id']
    if name == "rx" or name == "rx_explicit":
        return [None, [ord(item) for item in packet['rf_data']]]
    else:
        print "Unknwon ZB packet received", packet
    return [None, None]
    
def send(dest, buf):
    if xbee == 0:
        raise IOError("Call pyzigbee.init first")
    elif dest < 0 or dest >= len(addr):
        raise ValueError("ZB address out of range: %d" % dest)

    try:
        payload = ''.join(chr(item) for item in buf)
    except TypeError:
        DEBUGF(("ZB payload content:", payload))
        raise TypeError("Something not byte int is in the ZB payload list")

    xbee.send('tx', dest_addr_long=addr[dest], dest_addr='\xff\xfe', data=payload)
    
    packet = xbee.wait_read_frame_timeout(1000) # alternative (Blocked IO): packet = xbee.wait_read_frame()
    DEBUGF(("sent status ZB packet:", packet))
    if packet['id'] == 'tx_status':
        if ord(packet['deliver_status']) != 0:
            raise IOError("ZB packet delivery: Failed")
    else:
        raise IOError("ZB packet delivery status: Unknown")

def setdebug(enabled):
  pass # TODO
