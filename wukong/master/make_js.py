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
  paths = []
  for wuClass in dom.getElementsByTagName('WuClass'):
    name = wuClass.getAttribute('name')
    properties = wuClass.getElementsByTagName('property')
    path = prefix + name.lower() + postfix
    paths.append('__comp__'+name.lower()+postfix)
    if (not os.path.exists(path)):
        output = open(path, 'w')
        output.write(jinja2_env.get_template('component.js.tmpl').render(func_name=name, properties=properties))
        output.close()
    
  logging.info("make_js_complete")
  manifest_path = os.path.join(os.path.dirname(__file__),'static','js','components_list.txt')
  manifest_fd = open(manifest_path,'wb')
  manifest_fd.write("\n".join(paths))
  manifest_fd.close()
  logging.info("make manifest to "+manifest_path)
  