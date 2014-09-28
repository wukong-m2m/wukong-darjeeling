#!/usr/bin/python

import os, sys, traceback
import xml.dom.minidom
from model.models import *

class Parser:
  @staticmethod
  def parseLibraryXMLString(xml_string):
    dom = xml.dom.minidom.parseString(xml_string)

    print 'Constructing basic types'
    WuTypeDef.create('short', 'short')
    WuTypeDef.create('boolean', 'boolean')
    WuTypeDef.create('refresh_rate', 'refresh_rate')

    print 'Scanning types'
    for wuType in dom.getElementsByTagName('WuTypedef'):
      name = wuType.getAttribute('name')
      type = wuType.getAttribute('type')
      wutype = WuTypeDef.create(name, type)
      if wuType.getAttribute('type').lower() == 'enum':
        for element in wuType.getElementsByTagName('enum'):
          value = element.getAttribute('value').lower()
          wuvalue = WuValueDef.create(value, wutype.identity)

    print 'Scanning classes & properties'
    for wuClass in dom.getElementsByTagName('WuClass'):
      name = wuClass.getAttribute('name')
      id = int(wuClass.getAttribute('id'))
      #TODO: we should remove virtual from xml
      virtual = wuClass.getAttribute('virtual').lower() == 'true'
      type = wuClass.getAttribute('type')
      wuclassdef = WuClassDef.create(id, name, virtual, type)

      for property_id, prop_tag in enumerate(wuClass.getElementsByTagName('property')):
        name = prop_tag.getAttribute('name')
        datatype = prop_tag.getAttribute('datatype')
        default = prop_tag.getAttribute('default').lower()
        access = prop_tag.getAttribute('access')
        wuproperty = WuPropertyDef.create(property_id, 
            name, datatype, default, access, wuclassdef)

  @staticmethod
  def parseLibrary(xml_path):
    print 'Scanning', xml_path
    xml = open(xml_path)
    return Parser.parseLibraryXMLString(xml.read())
