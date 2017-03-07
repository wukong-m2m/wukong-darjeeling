import urllib2
#from google.appengine.api import oauth


print "==================Create System and device==================================="

print "[CreateSystem]"
print urllib2.urlopen('http://localhost:8888/createsystem?id=a&holder=b&devices=c&fbps=d&gateways=e&wuclasses=f').read()

print "[CreateDevice]"
print urllib2.urlopen('http://localhost:8888/createdevice?id=DEVICEa&wuobject=b&type=c&capacity=d&network=e&loc=lll').read()

print "[FindDevice]"
a=urllib2.urlopen('http://localhost:8888/finddevice').read()
print a


print "==================Delete one device==================================="
variable = raw_input("Press Enter to continue...")

print len(a)
print "[DeleteDevice]"
print urllib2.urlopen('http://localhost:8888/deletedevice?Did=55521cdc491d4312a60ec845').read()


print "[FindDevice]"
print urllib2.urlopen('http://localhost:8888/finddevice').read()