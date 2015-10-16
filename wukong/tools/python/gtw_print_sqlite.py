import sys, os
tools = os.path.join(os.path.dirname(__file__), "..")
wukong = os.path.join(tools, "..")
master = os.path.join(wukong, "master")
sys.path.append(os.path.abspath(master))
wkpf = os.path.join(master, "wkpf")
sys.path.append(os.path.abspath(wkpf))
# print sys.path
import mptnUtils as MPTN

print "========================\nGateway DB info:"
print "1.Nexthop DB"
# _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
db = MPTN.DBDict("../../gateway/gtw_nexthop_table.sqlite")
if len(db) == 0: print "None"
for (network_string, tcp_address) in db.iteritems():
    print "Tcp address of ID/prefix_len:", network_string, "is", tcp_address

print "\n2.Address DB"
# _addr_db: key = address, value = True or False
db = MPTN.DBDict("../../gateway/gtw_addr_table.sqlite")
if len(db) == 0: print "None"
for (address, value) in db.iteritems():
    print "Registered address:", MPTN.ID_TO_STRING(int(address))

print "\n3.UUID DB"
# _uuid_db: key = MPTN ID, value = UUID (such as MAC address)
db = MPTN.DBDict("../../gateway/gtw_uuid_table.sqlite")
if len(db) == 0: print "None"
for (mptn_id, uuid) in db.iteritems():
    print "UUID of ID:", MPTN.ID_TO_STRING(int(mptn_id)), "is", map(ord, uuid)

print "\n4.Settings DB"
db = MPTN.DBDict("../../gateway/gtw_settings_db.sqlite")
for (key,value) in db.iteritems():
    print "The value of setting", key, "is", value