import sys, os
tools = os.path.join(os.path.dirname(__file__), "..")
wukong = os.path.join(tools, "..")
master = os.path.join(wukong, "master")
sys.path.append(os.path.abspath(master))
wkpf = os.path.join(master, "wkpf")
sys.path.append(os.path.abspath(wkpf))
# print sys.path
import mptnUtils as MPTN

print "========================\nMaster DB info:"
print "\n1.Gateway DB"
db = MPTN.DBDict("../../master/master_gateway_info.sqlite")
if len(db) == 0: print "None"
# Gateway(id=167772166, tcp_address=('127.0.0.1', 9001), if_address=167772166, if_address_len=4, prefix=167772160, prefix_len=24,
#         network=IPv4Network('10.0.0.0/24'), network_size=256, netmask=4294967040, hostmask=255,
#         uuid='\xe5\xa8\xc3\xc6\xa8\xcdB;\x86\xfb\xc5O\\x\xad\xd4')
for (gateway_id, gateway) in db.iteritems():
    print "Gateway ID:", MPTN.ID_TO_STRING(int(gateway_id)), "IF_ADDR:", MPTN.ID_TO_STRING(gateway.if_address), "IF_ADDR_LEN:", gateway.if_address_len, "PREFIX:", MPTN.ID_TO_STRING(gateway.prefix), "PREFIX_LEN:", gateway.prefix_len, "NETWORK:", str(gateway.network), "NETWORK_SIZE:", gateway.network_size, "NETMASK:", MPTN.ID_TO_STRING(gateway.netmask), "HOSTMASK:", MPTN.ID_TO_STRING(gateway.hostmask), "UUID:", map(ord, gateway.uuid), "TCP_ADDRESS", str(gateway.tcp_address)

print "\n2.Node DB"
db = MPTN.DBDict("../../master/master_node_info.sqlite")
if len(db) == 0: print "None"
for (node_id, node) in db.iteritems():
    print "Node ID:", MPTN.ID_TO_STRING(int(node_id)), "IF_ADDR:", MPTN.ID_TO_STRING(node.if_address), "IS_GATEWAY:", node.is_gateway, "GATEWAY ID:", MPTN.ID_TO_STRING(node.gateway_id), "UUID:", map(ord, node.uuid)