import urllib2
#from google.appengine.api import oauth


print "[CreateUser]"
print urllib2.urlopen('http://localhost:8888/createuser?id=a&type=b&pref=c&loc=d').read()

print "[CreateSystem]"
print urllib2.urlopen('http://localhost:8888/createsystem?id=a&holder=b&devices=c&fbps=d&gateways=e&wuclasses=f').read()

print "[CreateDevice]"
print urllib2.urlopen('http://localhost:8888/createdevice?id=a&wuobject=b&type=c&capacity=d&network=e&loc=lll').read()