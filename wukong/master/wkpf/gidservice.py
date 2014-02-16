import os, os.path
import UserDict
import pickle
import sqlite3
import pynvc
import gevent
import gevent.queue
import gevent.server

def to_db_type(value):
    """
    If value's type is supported natively in SQLite, return value.
    Otherwise, return a pickled representation.
    """
    if value is None or isinstance(value, (int, long, float, basestring)):
        return value
    else:
        return buffer(pickle.dumps(value))

def from_db_type(value):
    """
    Converts a value from the database to a Python object.
    """
    if isinstance(value, buffer):
        return pickle.loads(value)
    else:
        return value

class DBDict(UserDict.DictMixin):
    """
    Shelf implementation using an SQLite3 database.
    """
    def __init__(self, filename):
        if not os.path.isfile(filename):
            self._database = sqlite3.connect(filename)
            self._database.execute("CREATE TABLE IF NOT EXISTS Shelf "
                               "(Key TEXT PRIMARY KEY NOT NULL, Value BLOB)")
        else:
            self._database = sqlite3.connect(filename)
        self._open = True

    def __del__(self):
        self.close()

    def __getitem__(self, key):
        row = self._database.execute("SELECT Value FROM Shelf WHERE Key=?",[key]).fetchone()
        if row:
            return from_db_type(row[0])
        else:
            raise KeyError(key)

    def __setitem__(self, key, value):
        self._database.execute("INSERT OR REPLACE INTO Shelf VALUES (?, ?)",[key, to_db_type(value)])
        self._database.commit()

    def __delitem__(self, key):
        if not self._database.execute("SELECT Key FROM Shelf WHERE Key=?",[key]).fetchone():
            raise KeyError(key)
        self._database.execute("DELETE FROM Shelf WHERE Key=?", [key])
        self._database.commit()

    def keys(self):
        """Return a list of keys in the shelf."""
        return [row[0] for row in self._database.execute("SELECT Key FROM Shelf")]

    def close(self):
        """Commit changes and close the file."""
        if self._database is not None:
            self._database.commit()
            self._database.close()
            self._database = None

NULL_GID = 0x0000
MASTER_GID = 0x0001
GID_BITS = 16
GID_MAX_NUM = 2 ** GID_BITS

class GIDService(gevent.server.DatagramServer()):
    _gid_server = None
    @classmethod
    def init(cls):
        if not cls._gid_server:
            cls._gid_server = GIDService(":9000")
        return cls._gid_server

    def __init__(self, *args, **kwargs):
        self.database = dbdict.DBDict("gid.db")

        self.avail_num = GID_MAX_NUM
        byte_num = 1 if GID_BITS <= 3 else (GID_BITS - 3)
        self.avail_bitset = '\xFF' * (byte_num)
        self.loadAvailableGIDs()

        self.reserve_list = []

        print 'GIDService init [OK]'
        gevent.spawn(self.serve_forever)

    def handle(self, data, address):
        payload = None
        client_lid = None

        if self.isGIDRequestPacket(packet):
            client_lid = packet.destination
            client_gid = self.getGIDSourceAddressFromPacket(packet)
            print "[GIDService] got GID REQUEST from %d" % client_lid

            gid = self.reserveGID(client_lid, client_gid)
            if not gid: 
                no_error = False
            tmp_msg = [pynvc.MULT_PROTO_MSG_SUBTYPE_GID_OFFER] + self.getTwoBytesListFromInt16(gid)
            payload = self.getGIDPayload(
                self.getTwoBytesListFromInt16(NULL_GID), 
                self.getTwoBytesListFromInt16(MASTER_GID), 
                pynvc.MULT_PROTO_MSG_TYPE, tmp_msg
            )
            print "[GIDService] reply with GID OFFER", payload
            self.socket.sendto('Received %s bytes' % len(data), address)

        elif self.isGIDACKPacket(packet):
            client_lid = packet.destination
            client_gid = self.getGIDSourceAddressFromPacket(packet)
            print "[GIDService] got GID ACK from %d GID = %d" % (client_lid, client_gid)
            self.allocateGID(client_lid, client_gid)
            no_error = False

        else:
            print "[GIDService] Error: invalid GID packet", packet
            no_error = False

        '''
        if no_error: 
            #getZwaveAgent().deferSend(client_lid, payload[0], payload[1:], [], cb, error_cb)
            defer = new_defer(cb, 
                error_cb,
                None, 
                [], 
                new_message(client_lid, payload[0], payload[1:]), int(round(time.time() * 1000)) + 10000)
            tasks.put_nowait(defer)
        '''

    def getTwoBytesListFromInt16(self, integer):
        return [(integer>>8 & 0xFF), (integer & 0xFF)]

    '''
    Bit manipulation functions
    used for managing available gids (max 2**16) with minimum memory and high performance
    '''
    def getByteAndBitIndexes(self, index):
        byte_ind = int(index / 8)
        bit_ind = index - byte_ind * 8
        return byte_ind, bit_ind

    def testBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        return (ord(self.avail_bitset[byte_ind]) >> bit_ind) & 0x1

    def setBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        self.avail_bitset = self.avail_bitset[0:byte_ind] + chr(ord(self.avail_bitset[byte_ind]) | (0x1 << bit_ind)) + self.avail_bitset[byte_ind+1:]
        self.avail_num += 1

    def clearBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        self.avail_bitset = self.avail_bitset[0:byte_ind] + chr(ord(self.avail_bitset[byte_ind]) & (~(0x1 << bit_ind))) + self.avail_bitset[byte_ind+1:]
        self.avail_num -= 1

    def popSetBit(self):
        for i in range(len(self.avail_bitset)):
            byte = self.avail_bitset[i]
            if byte is not "\x00":
                byte = ord(byte)
                for j in range(8):
                    if byte & 1:
                        ret = i*8 + j
                        self.clearBit(ret)
                        return ret
                    byte >>= 1
        return None

    def loadAvailableGIDs(self):
        gids = map(lambda x: int(x), self.database.keys())
        # exclude GID = 1 (Master) and 0 (NULL)
        #return [i for i in xrange(GID_MAX_NUM-1,1,-1) if i not in gids]
        self.clearBit(0)
        self.clearBit(1)
        for i in xrange(2, GID_MAX_NUM):
            if i in gids:
                self.clearBit(i)
    '''
    GID Packet handling sub-functions
    '''
    def reserveGID(self, lid, gid):
        if self.avail_num == 0:
            if len(self.reserve_list) == 0:
                print "GIDService Warning: No Available GID"
                return None
            print "GIDService Warning: Canceling reserved GID"
            for rlid, rgid in self.reserve_list:
                self.setBit(gid)
            self.reserve_list = []
        
        if (not self.testBit(gid)) and self.database[gid] == lid:
            return gid

        gid = self.popSetBit()
        gevent.sleep(0)
        self.reserve_list.append((lid,gid))
        #print self.reserve_list
        return gid

    def allocateGID(self, lid, gid):
        #print self.reserve_list
        if (lid, gid) not in self.reserve_list:
            print "GIDService Error: client lid = %d, gid = %d not reserved" % (lid,gid)
            return None
        elif gid >= GID_MAX_NUM or gid <= 1:
            print "GIDService Error: client gid = %d exceeds range [2, %d]" % (lid,gid,2**GID_BITS)
            return None
        self.reserve_list.remove((lid, gid))
        self.database[gid] = lid
        print "GIDService: Successfully assign GID %d to LID %d" % (gid, lid)

    def getGIDMessageTypeFromPacket(self, packet):
        return packet.payload[4]

    def getGIDSourceAddressFromPacket(self, packet):
        hbyte = packet.payload[1] & 0xFF
        lbyte = packet.payload[2] & 0xFF
        return (((hbyte) << 8) | lbyte)

    def getGIDPayload(self, destination, source, gid_message_type, extra):
        return destination + source + [gid_message_type] + extra

    def isGIDdeliver(self, deliver):
        return deliver.payload[3] == pynvc.MULT_PROTO_MSG_TYPE

    def isGIDRequestPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.MULT_PROTO_MSG_SUBTYPE_GID_REQ

    def isGIDACKPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.MULT_PROTO_MSG_SUBTYPE_GID_ACK