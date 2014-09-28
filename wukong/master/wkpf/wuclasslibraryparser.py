#!/usr/bin/python

import os, sys, traceback
import xml.dom.minidom
from model.models import *

class WuClassLibraryParser:
  @staticmethod
  def parseLibraryXMLString(xml_string):
    dom = xml.dom.minidom.parseString(xml_string)

    print ('creating wutypes')
    WuObjectFactory.createWuTypeDef('short', 'short')
    WuObjectFactory.createWuTypeDef('boolean', 'boolean')
    WuObjectFactory.createWuTypeDef('refresh_rate', 'refresh_rate')

    for wuType in dom.getElementsByTagName('WuTypedef'):
      name = wuType.getAttribute('name')
      type = wuType.getAttribute('type')
      wutype = WuObjectFactory.createWuTypeDef(name, type)
      if wuType.getAttribute('type').lower() == 'enum':
        for element in wuType.getElementsByTagName('enum'):
          value = element.getAttribute('value').lower()
          wutype.values.append(value)

    print 'Scanning classes & properties'
    for wuClass in dom.getElementsByTagName('WuClass'):
      name = wuClass.getAttribute('name')
      id = int(wuClass.getAttribute('id'))
      #TODO: we should remove virtual from xml
      virtual = wuClass.getAttribute('virtual').lower() == 'true'
      type = wuClass.getAttribute('type')
      wuclassdef = WuObjectFactory.createWuClassDef(id, name, virtual, type)

      for property_id, prop_tag in enumerate(wuClass.getElementsByTagName('property')):
        name = prop_tag.getAttribute('name')
        datatype = prop_tag.getAttribute('datatype')
        default = prop_tag.getAttribute('default').lower()
        access = prop_tag.getAttribute('access')
        wuproperty = WuObjectFactory.createWuPropertyDef(property_id,
            name, datatype, default, access, wuclassdef)

  @staticmethod
  def read(xml_path):
    print 'Scanning', xml_path
    xml = open(xml_path)
    return WuClassLibraryParser.parseLibraryXMLString(xml.read())
