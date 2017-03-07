import sys
from TOSSIM import *

t = Tossim([])
t.runNextEvent()
m = t.getNode(32)
m.turnOn()
t.addChannel("DEBUG", sys.stdout)
while (m.isOn()==1):
    t.runNextEvent()

