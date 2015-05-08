#!/usr/bin/python
# vim: ts=2 sw=2 expandtab

# author: Penn Su
import dateutil.parser

from gevent import monkey; monkey.patch_all()
import gevent
import serial
import platform
import os, sys, zipfile, re, time
import tornado.ioloop, tornado.web
import tornado.template as template
import simplejson as json
from jinja2 import Template
import logging
import hashlib
from threading import Thread
import traceback
import StringIO
import shutil, errno
import datetime
from datetime import date, timedelta
import glob
import copy
import fcntl, termios, struct
from types import *

import tornado.options
tornado.options.define("appdir", type=str, help="Directory that contains the applications")
tornado.options.parse_command_line()
from configuration_db import *

import pymongo


import tornado.options

print "DATA STORE API CLIENT"

import urllib
import urllib2

response = urllib2.urlopen('http://localhost:8888/get?app=df40477e9a75456ed53400041634f94b&com=Light_Sensor&loc=/WuKong/Door%23&wuc=1001')
html = response.read()
print html