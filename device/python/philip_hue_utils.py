from pprint import pformat
from twisted.internet.defer import Deferred
from twisted.web.client import Agent
from twisted.web.http_headers import Headers
from twisted.internet import reactor, protocol
import math
import json

class point2d():
    def __init__(self):
        self.x = -1
        self.y = -1

class BeginningPrinter(protocol.Protocol):
    def __init__(self, finished, gamma):
        self.finished = finished
        self.remaining = 1024 * 10
        self.gamma = gamma

    def dataReceived(self, bytes):
        if self.remaining:
            display = bytes[:self.remaining]
            print 'Some data received:'
            print display
            try: 
                display_parse = json.loads(display)
                self.gamma.x = display_parse['state']['xy'][0]
                self.gamma.y = display_parse['state']['xy'][1]
                self.gamma.bri = display_parse['state']['bri']
                modelid = display_parse['modelid']
                strcmp = lambda modelid_list: modelid in modelid_list
                modelid_list_1 = ['LCT001', 'LCT002', 'LCT003', 'LCT007', 'LLM001']
                modelid_list_2 = ['LLC006', 'LLC007', 'LLC010', 'LLC011', 'LLC012', 'LLC013', 'LST001']  
                modelid_list_3 = ['LLC020', 'LST002']
                if strcmp(modelid_list_1):
                    self.gamma.gamma = 1
                elif strcmp(modelid_list_2):
                    self.gamma.gamma = 0
                elif strcmp(modelid_list_3):
                    self.gamma.gamma = 2
                else:
                    self.gamma.gamma = -99
            except Exception as e:
                self.gamma.message = e.message
                self.gamma.gamma = -101

            self.remaining -= len(display)

    def connectionLost(self, reason):
        print 'Finished receiving body:', reason.getErrorMessage()
        self.finished.callback(None)

class hue_web_client():
    def __init__(self, ip, user, index, obj):
        self.http_get_path = 'http://'+ip+'/api/'+user+'/lights/'+str(index)
        self.http_put_path = 'http://'+ip+'/api/'+user+'/lights/'+str(index)+'/state/'
        self.gamma = obj
 
    def get_gamma(self):
        agent = Agent(reactor)
        defer = agent.request(
            'GET', self.http_get_path,
            Headers({'User-Agent': ['Twisted Web Client Example'],
                     'Content-Type': ['text/x-greeting']}),
            None)
        defer.addCallback(self.cbRequest)
        # defer.addBoth(self.cbShutdown)

    def put_command(self, body):
        agent = Agent(reactor)
        defer = agent.request(
            'PUT', self.http_put_path,
            Headers({'User-Agent': ['Twisted Web Client Example'],
                     'Content-Type': ['text/x-greeting']}),
            body)
        # defer.addBoth(self.cbShutdown)

    def cbRequest(self, response):
        #print 'Response version:', response.version
        #print 'Response code:', response.code
        #print 'Response phrase:', response.phrase
        #print 'Response headers:'
        #print pformat(list(response.headers.getAllRawHeaders()))
        finished = Deferred()
        response.deliverBody(BeginningPrinter(finished, self.gamma))
        return finished

    def cbShutdown(self, ignored):
        reactor.stop()

class hue_calculation():
    def gammaCorrection(self, rgb):
        if rgb > 0.04045:
            rgb = ((rgb + 0.055) / (1.0 + 0.055) ** 2.4)
        else:
            rgb = (rgb / 12.92)
        return rgb

    def crossProduct(self, p1, p2):
        return (p1.x * p2.y) - (p1.y * p2.x)

    def getClosestPointToLine(self, A, B, P, ret):
        AP = point2d()
        AB = point2d()
        AP.x=(P.x - A.x)
        AP.y=(P.y - A.y)
        AB.x=(B.x - A.x)
        AB.y=(B.y - A.y)
        ab2 = AB.x * AB.x + AB.y * AB.y
        ap_ab = AP.x * AB.x + AP.y * AB.y
        t = ap_ab / ab2
        if (t < 0.0):
            t = 0.0
        elif (t > 1.0):
            t = 1.0
        ret.x = (A.x + AB.x * t)
        ret.y = (A.y + AB.y * t)

    def getDistance(self, a, b):
        dx = a.x - b.x
        dy = a.y - b.y
        return math.sqrt(dx * dx + dy * dy)

    def getClosestPointToPoint(self, gamut_r, gamut_g, gamut_b, x, y):
        xy = point2d()
        pAB = point2d()
        pAC = point2d()
        pBC = point2d()
        xy.x = x
        xy.y = y
        self.getClosestPointToLine(gamut_r, gamut_g, xy, pAB)
        self.getClosestPointToLine(gamut_b, gamut_r, xy, pAC)
        self.getClosestPointToLine(gamut_g, gamut_b, xy, pBC)
        dAB = self.getDistance(xy, pAB)
        dAC = self.getDistance(xy, pAC)
        dBC = self.getDistance(xy, pBC)
        closetPoint = pAB
        lowest = dAB
        if (dAC < lowest):
            lowest = dAC
            closetPoint = pAC
        if (dBC < lowest):
            lowest = dBC
            closetPoint = pBC
        x = closetPoint.x
        y = closetPoint.y

    def checkPointInLampsReach(self, gamut_r, gamut_g, gamut_b, x, y):
        v1 = point2d()
        v2 = point2d()
        q = point2d()
        v1.x = gamut_g.x - gamut_r.x
        v1.y = gamut_g.y - gamut_r.y
        v2.x = gamut_b.x - gamut_r.x
        v2.y = gamut_b.y - gamut_r.y
        q.x = x - gamut_r.x
        q.y = y - gamut_r.y
        v1Xv2 = self.crossProduct(v1, v2) 
        v1Xv2 = v1Xv2 if v1Xv2 != 0 else -1
        s = self.crossProduct(q, v2) / v1Xv2
        t = self.crossProduct(v1, q) / v1Xv2
        if ((s >= 0.0) and (t >= 0.0) and (s+t <= 1.0)):
            return True
        else:
            return False

    def getGamut(self, gamma, gamut_r, gamut_g, gamut_b):
        if gamma == 0:
            gamut_r.x = 0.7; gamut_r.y = 0.2986;
            gamut_g.x = 0.214; gamut_g.y = 0.709;
            gamut_b.x = 0.139; gamut_b.y = 0.081;
        elif gamma == 1:
            gamut_r.x = 0.674; gamut_r.y = 0.322;
            gamut_g.x = 0.408; gamut_g.y = 0.517;
            gamut_b.x = 0.168; gamut_b.y = 0.041;
        elif gamma == 2:
            gamut_r.x = 0.692; gamut_r.y = 0.308;
            gamut_g.x = 0.17; gamut_g.y = 0.7;
            gamut_b.x = 0.153; gamut_b.y = 0.048;

    def RGBtoXY(self, obj, red, green, blue):
        r = red/255.0
        g = green/255.0
        b = blue/255.0
        r = self.gammaCorrection(r)
        g = self.gammaCorrection(g)
        b = self.gammaCorrection(b)
        X = r * 0.664511 + g * 0.154324 + b * 0.162028
        Y = r * 0.283881 + g * 0.668433 + b * 0.047685
        Z = r * 0.000088 + g * 0.072310 + b * 0.986039
        if ((X + Y + Z) == 0.0):
            cx = cy = 0.0;
        else:
            cx = X / (X + Y + Z)
            cy = Y / (X + Y + Z)
        gamut_r = point2d()
        gamut_g = point2d()
        gamut_b = point2d()
        self.getGamut(obj.gamma, gamut_r, gamut_g, gamut_b)
        if (not self.checkPointInLampsReach(gamut_r, gamut_g, gamut_b, cx, cy)):
            self.getClosestPointToPoint(gamut_r, gamut_g, gamut_b, cx, cy)
        obj.x = cx
        obj.y = cy
        obj.bri = Y

    def XYbtoRGB(self, gamma, x, y, bri, red, green, blue):
        gamut_r = point2d()
        gamut_g = point2d()
        gamut_b = point2d()
        self.getGamut(gamma, gamut_r, gamut_g, gamut_b)
        if (not self.checkPointInLampsReach(gamut_r, gamut_g, gamut_b, x, y)):
            self.getClosestPointToPoint(gamut_r, gamut_g, gamut_b, x, y)
      
        z = (1.0-x-y)
        Y = bri
        X = (Y/y)*x
        Z = (Y/y)*z
        r =  X * 1.656492 - Y * 0.354851 - Z * 0.255038
        g = -X * 0.707196 + Y * 1.655397 + Z * 0.036152
        b =  X * 0.051713 - Y * 0.121364 + Z * 1.011530

        r = 12.92 * r if (r <= 0.0031308) else (1.0 + 0.055) * (r**(1.0 / 2.4)) - 0.055
        g = 12.92 * g if (g <= 0.0031308) else (1.0 + 0.055) * (g**(1.0 / 2.4)) - 0.055
        b = 12.92 * b if (b <= 0.0031308) else (1.0 + 0.055) * (b**(1.0 / 2.4)) - 0.055

        if (r < 0.0): r = 0.0
        if (g < 0.0): g = 0.0
        if (b < 0.0): b = 0.0

        if (r > b and r > g):
          #red is biggest
          if (r > 1.0):
              g = g / r
              b = b / r
              r = 1.0
        elif (g > b and g > r):
          #green is biggest
          if (g > 1.0):
              r = r / g
              b = b / g
              g = 1.0
        elif (b > r and b > g):
          #blue is biggest
          if (b > 1.0):
              r = r / b
              g = g / b
              b = 1.0
        red = (r * 255.0 + 0.5)
        green = (g * 255.0 + 0.5)
        blue = (b * 255.0 + 0.5)