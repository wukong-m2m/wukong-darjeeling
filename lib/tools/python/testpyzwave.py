import pyzwave
import gevent
import sys

if len(sys.argv) < 2:
	print "Usagae: python testpyzwave.py port command..."
	print "\tcommand:"
	print "\t\treset\t: Factory reset controller"
	print "\t\tdiscover\t: Print discovered zwaveIDs"
	print "\t\tgetaddr\t: Get itself zwaveID"
	print "\t\tbasicset zwaveID value\t: Set certain value onto node with zwaveID"
	print "\t\tisnodefail zwaveID\t: Test if the node with zwaveID fails"
	print "\t\tremovefail zwaveID\t: Remove node with zwaveID if it fails"
	exit(-1)

pyzwave.setVerbose(0)
pyzwave.setdebug(0)
pyzwave.init(sys.argv[1])
gevent.sleep(1)

# print "sending packet"
# while True:
# 	pyzwave.send(5, [0x88]+[0, 19, 5, 2, 32, 2, 1, 0x1e])
# 	gevent.sleep(3)
# for i in [1,2,3,4,5]:
# 	print "got device type %s from device radio address %X" % (str(pyzwave.getDeviceType(i)), i)

if sys.argv[2] == "reset":
	print "-"*10+"pyzwave.hardReset"
	print str(pyzwave.hardReset())

elif sys.argv[2] == "discover":
	print "-"*10+"pyzwave.discover"
	print str(pyzwave.discover())

elif sys.argv[2] == "getaddr":
	print "-"*10+"pyzwave.getAddr"
	print str(pyzwave.getAddr())

elif sys.argv[2] == "basicset":
	print "-"*10+"pyzwave.basicSet"
	i = int(sys.argv[3])
	v = int(sys.argv[4])
	print str(pyzwave.basicSet(i, v))

elif sys.argv[2] == "isnodefail":
	print "-"*10+"pyzwave.isNodeFail"
	i = int(sys.argv[3])
	print str(pyzwave.isNodeFail(i))

elif sys.argv[2] == "removefail":
	print "-"*10+"pyzwave.removeFail"
	i = int(sys.argv[3])
	print str(pyzwave.removeFail(i))
