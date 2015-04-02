#Sato Katsunori made Oct.16 2012 
#Penn Su rewrite Aug.22 2013

import os
import logging
from configuration import *
import xml.dom.minidom
from jinja2 import Template, Environment, FileSystemLoader

prefix = "./static/js/__comp__"
postfix = ".js"

jinja2_env = Environment(loader=FileSystemLoader([os.path.join(os.path.dirname(__file__), 'jinja_templates')]))

def make_main(path=COMPONENTXML_PATH):
  dom = xml.dom.minidom.parseString(open(path, "r").read())
  for wuClass in dom.getElementsByTagName('WuClass'):
    name = wuClass.getAttribute('name')
    properties = wuClass.getElementsByTagName('property')
    if (not os.path.exists(prefix + name.lower() + postfix)):
        output = open(prefix + name.lower() + postfix, 'w')
        output.write(jinja2_env.get_template('component.js.tmpl').render(func_name=name, properties=properties))
        output.close()

  logging.info("make_js_complete")
