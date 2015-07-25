import mptnUtils as MPTN

print "Nexthop DB"
# _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
db = MPTN.DBDict("gtw_nexthop_table.sqlite")
if len(db) == 0: print "None"
for (network_string, tcp_address) in db.iteritems():
    print "Tcp address of ID/prefix_len:", network_string, "is", tcp_address

print "\nAddress DB"
# _addr_db: key = address, value = True or False
db = MPTN.DBDict("gtw_addr_table.sqlite")
if len(db) == 0: print "None"
for (address, value) in db.iteritems():
    print "Registered address:", MPTN.ID_TO_STRING(int(address))

print "\nUUID DB"
# _uuid_db: key = MPTN ID, value = UUID (such as MAC address)
db = MPTN.DBDict("gtw_uuid_table.sqlite")
if len(db) == 0: print "None"
for (mptn_id, uuid) in db.iteritems():
    print "UUID of ID:", MPTN.ID_TO_STRING(int(mptn_id)), "is", map(ord, uuid)

print "\nSettings DB"
db = MPTN.DBDict("gtw_settings_db.sqlite")
for (key,value) in db.iteritems():
    print "The value of setting", key, "is", value