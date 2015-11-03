import sys, os
tools = os.path.join(os.path.dirname(__file__), "..")
wukong = os.path.join(tools, "..")
gateway = os.path.join(wukong, "gateway")
sys.path.append(os.path.abspath(gateway))
# print sys.path
import mptnUtils as MPTN
import gtwconfig as CONFIG

print "========================\nGateway DB info:"
print "1.Nexthop DB"
# _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
db = MPTN.DBDict("../../gateway/gtw_nexthop_table.sqlite")
if len(db) == 0: print "None"
for (network_string, tcp_address) in db.iteritems():
    print "TCP address of ID/prefix_len:", network_string, "is", tcp_address

print "\n2.Address DB"
# _addr_db: key = address, value = True or False
db = MPTN.DBDict("../../gateway/gtw_addr_uuid_table.sqlite")
if len(db) == 0: print "None"
for (address, uuid) in db.iteritems():
    print "Registered address and its UUID:", MPTN.ID_TO_STRING(int(address)), map(ord, uuid)

print "\n3.Settings DB"
db = MPTN.DBDict("../../gateway/gtw_settings_db.sqlite")
for (key,value) in db.iteritems():
    print "The value of setting", key, "is", value

if CONFIG.TRANSPORT_INTERFACE_TYPE == 'udp':
    from transport_udp import UDPDevice
    import pickle
    print "\n4.UDP Pickle File devices.pkl"
    try:
        f = open("../../gateway/devices.pkl")
        devices = pickle.load(f)
        f.close()
    except Exception as e:
        print e

    for d in devices:
        print d.host_id, d.ip, d.port
