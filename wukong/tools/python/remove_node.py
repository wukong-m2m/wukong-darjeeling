import sys, os
tools = os.path.join(os.path.dirname(__file__), "..")
wukong = os.path.join(tools, "..")
master = os.path.join(wukong, "master")
sys.path.append(os.path.abspath(master))
wkpf = os.path.join(master, "wkpf")
sys.path.append(os.path.abspath(wkpf))
# print sys.path
import mptnUtils as MPTN


to_del_node_id = sys.argv[1]
print "\n\n***************************************************"
print "Going to remove node with %s (%d)" % (MPTN.ID_TO_STRING(MPTN.ID_FROM_STRING(to_del_node_id)), MPTN.ID_FROM_STRING(to_del_node_id))
to_del_node_id = MPTN.ID_FROM_STRING(to_del_node_id)
is_id_gateway = False
print "***************************************************\n"


print "\n\n========================\nMaster DB info:\n========================"
print "\n1.Gateway DB"

db = MPTN.DBDict(os.path.join(os.path.abspath(master), "master_gateway_info.sqlite"))
if len(db) == 0: print "None"
else:
    # Gateway(id=167772166, tcp_address=('127.0.0.1', 9001), if_address=167772166, if_address_len=4, prefix=167772160, prefix_len=24,
    #         network=IPv4Network('10.0.0.0/24'), network_size=256, netmask=4294967040, hostmask=255,
    #         uuid='\xe5\xa8\xc3\xc6\xa8\xcdB;\x86\xfb\xc5O\\x\xad\xd4')
    for (gateway_id, gateway) in db.iteritems():
        print "Gateway ID:", MPTN.ID_TO_STRING(int(gateway_id)), "IF_ADDR:", MPTN.ID_TO_STRING(gateway.if_address), "IF_ADDR_LEN:", gateway.if_address_len, "PREFIX:", MPTN.ID_TO_STRING(gateway.prefix), "PREFIX_LEN:", gateway.prefix_len, "NETWORK:", str(gateway.network), "NETWORK_SIZE:", gateway.network_size, "NETMASK:", MPTN.ID_TO_STRING(gateway.netmask), "HOSTMASK:", MPTN.ID_TO_STRING(gateway.hostmask), "UUID:", map(ord, gateway.uuid), "TCP_ADDRESS", str(gateway.tcp_address)

    if to_del_node_id in db:
        is_id_gateway = True

print "\n2.Node DB"
db = MPTN.DBDict(os.path.join(os.path.abspath(master), "master_node_info.sqlite"))
if len(db) == 0: print "None"
else:
    if to_del_node_id in db:
        del db[to_del_node_id]
    to_be_deleted = []
    for (node_id, node) in db.iteritems():
        print "Node ID:", MPTN.ID_TO_STRING(int(node_id)), "IF_ADDR:", MPTN.ID_TO_STRING(node.if_address), "IS_GATEWAY:", node.is_gateway, "GATEWAY ID:", MPTN.ID_TO_STRING(node.gateway_id), "UUID:", map(ord, node.uuid)

        if (node.gateway_id == to_del_node_id):
            to_be_deleted.append(int(node_id))

    if is_id_gateway:
        for node_id in to_be_deleted:
            del db[node_id]

sys.path.remove(os.path.abspath(master))

if is_id_gateway:
    print "The ID is gateway. Done removing all nodes related to that gateway.\nPlease remove the *.sqlite, *.pkl within that gateway folder"
    exit()

import sys, os
tools = os.path.join(os.path.dirname(__file__), "..")
wukong = os.path.join(tools, "..")
gateway = os.path.join(wukong, "gateway")
sys.path.append(os.path.abspath(gateway))
import mptnUtils as MPTN
import gtwconfig as CONFIG
print "========================\nGateway DB info:\n========================"
print "1.Nexthop DB"
# _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
db = MPTN.DBDict(os.path.join(os.path.abspath(gateway), "gtw_nexthop_table.sqlite"))
if len(db) == 0: print "None"
else:
    for (network_string, tcp_address) in db.iteritems():
        print "TCP address of ID/prefix_len:", network_string, "is", tcp_address

print "\n2.Address DB"
# _addr_db: key = address, value = True or False
db = MPTN.DBDict(os.path.join(os.path.abspath(gateway), "gtw_addr_uuid_table.sqlite"))
if len(db) == 0: print "None"
else:
    for (address, uuid) in db.iteritems():
        if int(address) == to_del_node_id:
            del db[address]
            break
        else:
            print "Registered address and its UUID:", MPTN.ID_TO_STRING(int(address)), map(ord, uuid)

print "\n3.Settings DB"
db = MPTN.DBDict(os.path.join(os.path.abspath(gateway), "gtw_settings_db.sqlite"))
if len(db) == 0: print "None"
else:        
    for (key,value) in db.iteritems():
        print "The value of setting", key, "is", value

if CONFIG.TRANSPORT_INTERFACE_TYPE == 'udp':
    from transport_udp import UDPDevice
    import pickle
    print "\n4.UDP Pickle File devices.pkl"
    try:
        f = open(os.path.join(os.path.abspath(gateway), "devices.pkl"))
        devices = pickle.load(f)
        f.close()
        index = None
        for i, d in enumerate(devices):
            print d.host_id, d.ip, d.port
            if d.host_id == (to_del_node_id & 0xff):
                index = i
                break

        if index is not None:
            devices.pop(index)

        f = open(os.path.join(os.path.abspath(gateway), "devices.pkl"),'w')
        pickle.dump(devices,f)
        f.close()
    except Exception as e:
        print e


print "Done removeing %s\n\nPlease restart Master and/or Gateway..." % MPTN.ID_TO_STRING(to_del_node_id)