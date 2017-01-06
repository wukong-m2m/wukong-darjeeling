#! /usr/bin/python
from TOSSIM import *
import sys

t = Tossim([])
r = t.radio()
f = open("topo.txt", "r")

lines = f.readlines()
for line in lines:
  s = line.split()
  if (len(s) > 0):
#    print " ", s[0], " ", s[1], " ", s[2];
    r.add(int(s[0]), int(s[1]), float(s[2]))

t.addChannel("OUTPUT", sys.stdout)
#t.addChannel("DEBUG", sys.stdout)

noise = open("meyer-heavy.txt", "r")
lines = noise.readlines()
for line in lines:
  str = line.strip()
  if (str != ""):
    val = int(str)
    for i in range(1, 6):
      t.getNode(i).addNoiseTraceReading(val)

for i in range(1, 6):
  print "Creating noise model for ",i;
  t.getNode(i).createNoiseModel()

print "Starting the nodes"
t.getNode(1).bootAtTime(1000001);
t.getNode(2).bootAtTime(1000008);
t.getNode(3).bootAtTime(2000009);
t.getNode(4).bootAtTime(2800009);
t.getNode(5).bootAtTime(2100009);

while(t.runNextEvent() != 0):
	True

