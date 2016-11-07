from twisted.web.client import FileBodyProducer
from twisted.protocols import basic
from twisted.internet import reactor
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

from udpwkpf import WuClass, Device
import sys
import json
import csv
import copy
from udpwkpf_io_interface import *
csv.register_dialect(
        'mydialect',
        delimiter = ',',
        quotechar = '"',
        doublequote = True,
        skipinitialspace = True,
        lineterminator = '\r\n',
        quoting = csv.QUOTE_MINIMAL)
HOUSELAYOUT = 'dollhouseMap.csv'

LED_GRID               = 4
LEFT                   = -1
RIGHT                  = 1
STATIC                 = 0

class Path_LED(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Path_LED')
        self.id = 0
        self.myMQTTClient = AWSIoTMQTTClient("")
        self.myMQTTClient.configureEndpoint("a1trumz0n7avwt.iot.us-west-2.amazonaws.com", 8883)
        self.myMQTTClient.configureCredentials("AWS/root.crt", "AWS/private.key", "AWS/cert.crt")
        self.myMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
        self.myMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
        self.myMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
        self.myMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec
        self.myMQTTClient.connect()
        print "aws init success"
        self.map = []
        self.dist = []
        self.stairs = []
        self.leftStairEntries = []
        self.rightStairEntries = []
        self.ledstairMap = []
        self.fires = []
        self.ledfloorMap = []
        self.lastPath = []
        self.index = 0
        self.count = 0
        self.humans = []
        self.paths = []
        self.counts = []
        self.direction = 0
        self.nFloor = 0
        self.initMap()
        self.myMQTTClient.subscribe('fireMessage', 1, self.Callback)

    def Callback(self, client, userdata, message):
        print("Received a new message: ")
        print(message.payload)
        print("from topic: ")
        print(message.topic)
        print("--------------\n\n")
        try:
            data = json.loads(message.payload)
            print data
            if 'fire' in data.keys():
                if data['fire']:
                    self.addFire(data['floor'], data['x'], data['y'])
                    self.recalMap()
                else:
                    self.removeFire(data['floor'], data['x'], data['y'])
                    self.recalMap()
                self.calPath()
            if 'human' in data.keys():
                if data['human']:
                    self.humans.append((data['floor'], data['x'], data['y']))
                    self.calPath()
                else:
                    i = self.humans.index((data['floor'], data['x'], data['y']))
                    self.humans.pop(i)
                    if len(self.paths) > i:
                        self.paths.pop(i)
                        self.counts.pop(i)
        except ValueError, e:
            print 'not JSON'

    def initMap(self):
        with open(HOUSELAYOUT, 'rb') as csvfile:
            csv.reader(csvfile, dialect='mydialect')
            nextline = next(csvfile).split(',')
            nFloor, nStair = int(nextline[1]), int(nextline[2])
            self.nFloor = nFloor
            for i in xrange(nFloor):
                floor = []
                nextline = next(csvfile).split(',')
                floorL, floorW = int(nextline[1]), int(nextline[2])
                stripIndex = []
                for j in xrange(floorL):
                    nextline = next(csvfile).split(',')
                    floor.append([int(x) for x in nextline[1:floorW+1]])
                    stripIndex.append([int(x) for x in nextline[11:floorW+11]])

                self.map.append(floor)
                self.dist.append(copy.deepcopy(floor))
                self.ledfloorMap.append(stripIndex)
            for i in xrange(nStair):
                nextline = next(csvfile).split(',')
                self.stairs.append([int(x) for x in nextline[1:7]]+[1])
                lastStair = self.stairs[-1]
                self.leftStairEntries.append((lastStair[0], lastStair[1], lastStair[2]))
                self.rightStairEntries.append((lastStair[3], lastStair[4], lastStair[5]))
                stripIndex = []
                for index in nextline[11:]:
                    if index != '' and index != '\n':
                        stripIndex.append(int(index))
                self.ledstairMap.append(stripIndex)

    def addFire(self, f, i, j):
        self.fires.append((f, i, j))

    def removeFire(self, f, i, j):
        self.fires.remove((f, i, j))

    def recalMap(self):
        for floor in xrange(len(self.map)):
            for i in xrange(len(self.map[floor])):
                for j in xrange(len(self.map[floor][i])):
                    if self.map[floor][i][j] != 101 and self.map[floor][i][j] != 0:
                        self.map[floor][i][j] = 1
        for stair in self.stairs:
            stair[6] = 1

        for fire in self.fires:
            floor = fire[0]
            x = fire[1]
            y = fire[2]
            self.spreadFire(floor , x, y, 100)

    def spreadFire(self, floor , x, y, power):
        for i in xrange(len(self.map[floor])):
            for j in xrange(len(self.map[floor][i])):
                val = power - 10 * abs(x - i) - 20 * abs(y - j)
                if self.map[floor][i][j] and val > 0 and self.map[floor][i][j] < val:
                    self.map[floor][i][j] = val

        for stair in self.stairs:
            if stair[0] == floor and stair[6] < self.map[floor][stair[1]][stair[2]] - 10:
                stair[6] = self.map[floor][stair[1]][stair[2]] - 10
                self.spreadFire(stair[3], stair[4], stair[5], stair[6] - 10)
            if stair[3] == floor and stair[6] < self.map[floor][stair[4]][stair[5]] - 10:
                stair[6] = self.map[floor][stair[4]][stair[5]] - 10
                self.spreadFire(stair[0], stair[1], stair[2], stair[6] - 10)

    def dfsDist(self, f, x, y):
        if self.map[f][x][y] == 0:
                return
        dir = [(1,0),(-1,0),(0,1),(0,-1)]
        for i, j in dir:
            if self.map[f][x+i][y+j] != 101:
                if self.dist[f][x+i][y+j] == -1 or self.dist[f][x+i][y+j] > self.dist[f][x][y] + self.map[f][x+i][y+j]:
                    self.dist[f][x+i][y+j] = self.dist[f][x][y] + self.map[f][x+i][y+j]
                    self.dfsDist(f, x+i, y+j)
        for stair in self.stairs:
            if [f, x, y] == stair[0:3]:
                (i, j, k) = stair[3:6]
                if self.dist[i][j][k] == -1 or self.dist[i][j][k] > self.dist[f][x][y] + stair[6] + self.map[i][j][k]:
                    self.dist[i][j][k] = self.dist[f][x][y] + stair[6] + self.map[i][j][k]
                    self.dfsDist(i, j, k)
            if [f, x, y] == stair[3:6]:
                (i, j, k) = stair[0:3]
                if self.dist[i][j][k] == -1 or self.dist[i][j][k] > self.dist[f][x][y] + stair[6] + self.map[i][j][k]:
                    self.dist[i][j][k] = self.dist[f][x][y] + stair[6] + self.map[i][j][k]
                    self.dfsDist(i, j, k)

    def reversePath(self, f, x, y, path):
        if self.dist[f][x][y] == 0:
            return
        dir = [(1,0),(-1,0),(0,1),(0,-1)]
        for i, j in dir:
            try:
                if self.dist[f][x+i][y+j] != -1 and self.dist[f][x+i][y+j] + self.map[f][x][y] == self.dist[f][x][y]:
                    path.append((f, x+i, y+j))
                    self.reversePath(f, x+i, y+j, path)
                    return
            except:
                pass
        for stair in self.stairs:
                if [f, x, y] == stair[0:3]:
                    (i, j, k) = stair[3:6]
                    if self.dist[i][j][k] != -1 and self.dist[i][j][k] + stair[6] + self.map[f][x][y] == self.dist[f][x][y]:
                        path.append((i, j, k))
                        self.reversePath(i, j, k, path)
                        return
                if [f, x, y] == stair[3:6]:
                    (i, j, k) = stair[0:3]
                    if self.dist[i][j][k] != -1 and self.dist[i][j][k] + stair[6] + self.map[f][x][y] == self.dist[f][x][y]:
                        path.append((i, j, k))
                        self.reversePath(i, j, k, path)
                        return

    def findPath(self, f, x, y):
        for floor in xrange(len(self.dist)):
            for i in xrange(len(self.dist[floor])):
                for j in xrange(len(self.dist[floor][i])):
                    self.dist[floor][i][j] = -1
        self.dist[f][x][y] = 0
        self.dfsDist(f, x, y)
        shortestDist = -1
        startPoint = (0,0,0)
        for floor in xrange(len(self.map)):
            for i in xrange(len(self.map[floor])):
                for j in xrange(len(self.map[floor][i])):
                    if self.map[floor][i][j] == 0 and self.dist[floor][i][j] != -1:
                        if shortestDist == -1 or shortestDist > self.dist[floor][i][j]:
                            shortestDist = self.dist[floor][i][j]
                            startPoint = (floor, i, j)
        path = [startPoint]
        self.reversePath(startPoint[0], startPoint[1], startPoint[2], path)
        return path[::-1]
    
    def calPath(self):
        if self.index < self.nFloor:
            self.paths = []
            self.counts = []
            for point in self.humans:
                path = self.findPath(point[0], point[1], point[2])
                newPath = []
                for point in path:
                    if point[0] == self.index:
                        newPath.append(point)
                self.paths.append(newPath)
                self.counts.append(0)
        else:
            self.direction = 0
            for point in self.humans:
                path = self.findPath(point[0], point[1], point[2])
                self.direction |= self.findDirection(path)

    def updateSafty(self):
        if self.index < self.nFloor:
            for x, row in enumerate(self.ledfloorMap[self.index]):
                for y, ledstripIndex in enumerate(row):
                    if ledstripIndex == 101: # 101 is used to indicate that there is no led placed on this coordinate.
                        continue
                    safty = self.map[self.index][x][y]   
                    level = (safty / 100.0)
                    led.set(ledstripIndex, Color(255, 0, 0, level))
            led.update()
        else:
            i = self.index - self.nFloor
            safty = self.stairs[i][-1]
            level = safty / 100.0
            ledstripLen = len(self.ledstairMap[i])
            for ledstripIndex in range(ledstripLen):
                led.set(ledstripIndex, Color(255, 0, 0, level))
            led.update()

    def findDirection(self, path):
        if self.index < self.nFloor:
            pass
        else:
            i = self.index - self.nFloor
            leftStairEntry = self.leftStairEntries[i]
            lf, lx, ly = leftStairEntry
            rightStairEntry = self.rightStairEntries[i]
            rf, rx, ry = rightStairEntry
            leftSafty = self.map[lf][lx][ly] 
            rightSafty = self.map[rf][rx][ry]
            if leftStairEntry in path and rightStairEntry in path:
                leftSequenceNumber = path.index(leftStairEntry)
                rightSequenceNumber = path.index(rightStairEntry)
                if leftSequenceNumber < rightSequenceNumber:
                    return RIGHT        
                else:
                    return LEFT
            else:
                return STATIC

    def setPathLED(self, point):
        x = point[1]
        y = point[2]
        ledstripIndex = self.ledfloorMap[self.index][x][y]
        level = 0.5
        led.set(ledstripIndex, Color(0, 255, 0, level))
        led.update()
        
    def updateEvacuation(self):
        if self.index < self.nFloor:
            self.updateSafty()
            if len(self.paths) == 0:
                return
            for x in xrange(len(self.paths)):
                self.counts[x] %= LED_GRID
                for i in xrange(self.counts[x], len(self.paths[x]), LED_GRID):
                    self.setPathLED(self.paths[x][i])
                self.counts[x] += 1
        else:
            i = self.index - self.nFloor
            self.updateSafty()
            if self.direction:
                tempMap = self.ledstairMap[i]
                tempLen = len(tempMap)
                self.count %= tempLen
                index = tempMap[self.count]
                level = 0.5
                led.set(index, Color(0, 255, 0, level))
                led.update()
                self.count += self.direction
            else:
                print 'direction is static'
                self.updateSafty()

    def update(self,obj,pID=None,val=None):
        if pID == 0:
            self.index = val
        if len(self.fires):
            self.updateEvacuation()
        else:
            pass
            #led.all_off()

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            self.m = Path_LED()
            self.addClass(self.m,0)
            self.addObject(self.m.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
