import csv
import sys
import copy
import time
from bootstrap import *

FLOOR                  = 0
STAIR                  = 1

location = FLOOR # FLOOR or STAIR

#direction
LEFT                   = -1
RIGHT                  = 1
STATIC                 = None

#FLOOR
LEFTTOP                = 0
LEFTMIDDLE             = 1
LEFTBOTTOM             = 2
RIGHTTOP               = 3
RIGHTMIDDLE            = 4
RIGHTBOTTOM            = 5

#STAIR
LEFTTOP2MIDDLE         = 0
LEFTMIDDLE2BOTTOM      = 1
RIGHTTOP2MIDDLE        = 2
RIGHTMIDDLE2BOTTOM     = 3
LEFTMIDDLE2RIGHTTOP    = 4
LEFTBOTTOM2RIGHTMIDDLE = 5
LEFTTOP2RIGHTTOP       = 6

csv.register_dialect(
        'mydialect',
        delimiter = ',',
        quotechar = '"',
        doublequote = True,
        skipinitialspace = True,
        lineterminator = '\r\n',
        quoting = csv.QUOTE_MINIMAL)

class patternSuggestor(object):
    def __init__(self, index):
        self.map = []
        self.dist = []
        self.stairs = []
        self.leftStairEntries = []
        self.rightStairEntries = []
        self.ledstairMap = []
        self.fires = []
        self.ledfloorMap = []
        self.lastPath = []
        self.floorIndex = index
        self.stairIndex = index
        self.count = 0
        self.path = []
        self.paths = []
        self.counts = []
        self.direction = None
      
    def initMap(self, filename):
        with open(filename, 'rb') as csvfile:
            csv.reader(csvfile, dialect='mydialect')
            nextline = next(csvfile).split(',')
            nFloor, nStair = int(nextline[1]), int(nextline[2])

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
    
    def setPathList(self, paths):
        if location == FLOOR:
           self.paths = []
           self.counts = []
           for path in paths:
               newPath = []
               for point in path:
                   if point[0] == self.floorIndex:
                       newPath.append(point)
               if len(newPath) > 0:
                   self.paths.append(newPath)
                   self.counts.append(0)
        elif location == STAIR:
            self.direction = 0
            for path in paths:
                self.direction |= self.findDirection(path)

    def updateSafty(self):
        if location == FLOOR:
            for x, row in enumerate(self.ledfloorMap[self.floorIndex]):
                for y, ledstripIndex in enumerate(row):
                    if ledstripIndex == 101: # 101 is used to indicate that there is no led placed on this coordinate.
                        continue
                    safty = self.map[self.floorIndex][x][y]   
                    level = (safty / 100.0)
                    #print 'coordinate: ', x, y, 'index: ', ledstripIndex, 'safty: ', safty
                    led.set(ledstripIndex, Color(255, 0, 0, level))
            led.update()
        elif location == STAIR:
            safty = self.stairs[self.stairIndex][-1]
            level = safty / 100.0
            #print 'stairIndex: ', self.stairIndex, 'safty: ', safty
            ledstripLen = len(self.ledstairMap[self.stairIndex])
            for ledstripIndex in range(ledstripLen):
                led.set(ledstripIndex, Color(255, 0, 0, level))
            led.update()
        else:
            raise NotImplementedError

    def findDirection(self, path):
        if location == FLOOR:
            pass
        elif location == STAIR:
            leftStairEntry = self.leftStairEntries[self.stairIndex]
            lf, lx, ly = leftStairEntry
            rightStairEntry = self.rightStairEntries[self.stairIndex]
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
        else:
            raise NotImplementedError

    def setPathLED(self, point):
        x = point[1]
        y = point[2]
        ledstripIndex = self.ledfloorMap[self.floorIndex][x][y]
        level = 0.5
        led.set(ledstripIndex, Color(0, 255, 0, level))
        led.update()
        
    def updateEvacuation(self):
        if location == FLOOR:
            self.updateSafty()
            if len(self.paths) == 0:
                print "No Path!!"
                return
            for x in xrange(len(self.paths)):
                if self.counts[x] >= len(self.paths[x]):
                    self.counts[x] = 0
                print x, self.counts[x]
                self.setPathLED(self.paths[x][self.counts[x]])
                self.counts[x] += 1
        elif location == STAIR:
            self.updateSafty()
            if self.direction:
                tempMap = self.ledstairMap[self.stairIndex]
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
        else:
            raise NotImplementedError

p=patternSuggestor(LEFTTOP)
#p=patternSuggestor(LEFTTOP2RIGHTTOP)
p.initMap(sys.argv[1])
p.addFire(0,2,2)
p.recalMap()
#print 'map: ', p.map[1]
p.updateSafty()
path1 = p.findPath(0,5,2)
path2 = p.findPath(0,2,4)
p.setPathList([path1, path2])
print p.paths
while True:
    time.sleep(1)
    p.updateEvacuation()
#print 'path1: ', path1
#p.removeFire(1, 4, 2)
#p.recalMap()
#p.updateSafty()
