# vim: ts=4 sw=4
#!/usr/bin/python

import os, sys
from xml.etree import ElementTree
import xml.dom.minidom
sys.path.append(os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), "../..")))
from jinja2 import Template, Environment, FileSystemLoader
from struct import pack

from wkpf.model.models import *

from configuration import *
from wkpf.util import *


class Generator:
    @staticmethod
    def generate(name, changesets):
        Generator.generateTempComponentID(name, changesets)
        print '[generator] generate Table XML'
        Generator.generateTablesXML(name, changesets)
        print '[generator] generate Java App'
        Generator.generateJavaApplication(name, changesets)

    @staticmethod
    def generateJavaApplication(name, changesets):
        # i is the number to be transform into byte array, n is the number of bytes to use (little endian)
        def bytestring(i, n):
            return ['(byte)' + str(ord(b)) for b in pack("H", i)][:n]

        def nodeinjava(node):
            return str(node.id)

        def wuobjectinjava(wuobject):
            return ', '.join([str(wuobject.wunode().id),
                            str(wuobject.port_number)])

        def linkinjava(link):
            return ', '.join(bytestring(link.from_component_index, 2)
                    + bytestring(link.from_property_id, 1)
                    + bytestring(link.to_component_index, 2)
                    + bytestring(link.to_property_id, 1)
                    + bytestring(link.to_wuclass_id, 2))

        def wuclassname(wuclass):
            return wuclass.name

        def wuclassvirtualclassname(wuclass):
            return "Virtual" + Convert.to_java(wuclass.name) + "WuObject"

        def wuclassid(wuclass):
            return str(wuclass.id)

        def wuclassgenclassname(wuclass):
            return "GENERATEDVirtual" + Convert.to_java(wuclass.name) + "WuObject"

        def propertyconstname(property):
            print 'propertyconstname'
            return "PROPERTY_" + Convert.to_constant(property.wuobject.wuclassdef.name) + "_" + Convert.to_constant(property.name)

        # doesn't really matter to check since basic types are being take care of in application.java
        def propertyconstantvalue(property):
            wutype = WuTypeDef.where(name=property.wutype)
            if wutype:
                return wutype[0].type.upper() + '_' + Convert.to_constant(property.wutype) + "_" + Convert.to_constant(property.value)
            else:
                return 'ENUM' + '_' + Convert.to_constant(property.wutype) + "_" + Convert.to_constant(property.value)

        def generateProperties(wuobject_properties, component_properties):
            properties = wuobject_properties
            for property in properties:
                if property.name in component_properties:
                    property.value = component_properties[property.name]
            return properties

        # Generate the Java code
        print '[generator] generating', os.path.join(JAVA_OUTPUT_DIR, "WKDeploy.java")
        jinja2_env = Environment(loader=FileSystemLoader([os.path.join(os.path.dirname(__file__), 'jinja_templates')]))
        jinja2_env.filters['nodeinjava'] = nodeinjava
        jinja2_env.filters['wuclassname'] = wuclassname
        jinja2_env.filters['wuclassvirtualclassname'] = wuclassvirtualclassname
        jinja2_env.filters['wuclassid'] = wuclassid
        jinja2_env.filters['wuclassgenclassname'] = wuclassgenclassname
        jinja2_env.filters['propertyconstname'] = propertyconstname
        jinja2_env.filters['propertyconstantvalue'] = propertyconstantvalue
        jinja2_env.filters['generateProperties'] = generateProperties
        jinja2_env.add_extension('jinja2.ext.do')
        output = open(os.path.join(JAVA_OUTPUT_DIR, "WKDeploy.java"), 'w')
        output.write(jinja2_env.get_template('application2.java').render(name=name, changesets=changesets))
        output.close()
    @staticmethod
    def generateTempComponentID(name, changesets):
        component_index = 0
        for component in changesets.components:
          component.deployid = component_index
          component_index = component_index + 1
    @staticmethod
    def isLinkDestination(componentId, propertyId, changesets):
        for link in changesets.links:
            if link.to_component.deployid == componentId and link.to_property.id == propertyId:
                return True
        return False
    @staticmethod
    def generateTablesXML(name, changesets):
        #assign init values to wuobjects from components
        def generateProperties(wuobject_properties, component):
            all_properties = wuobject_properties
            component_properties_with_values = component.properties
            for property in all_properties:
                if property.name in component_properties_with_values and len(component_properties_with_values[property.name].strip())>0:
                    #print "set", property.name, "from default", property.value, "to", component_properties_with_values[property.name]
                    property.value = component_properties_with_values[property.name]
                else:   #if no value given, use default value of property
                    default_val = WuObjectFactory.wuclassdefsbyname[component.type].properties[property.name].value
                    if len(default_val) >0:
                        property.value = default_val
            return [p for p in all_properties if p.access!='readonly']

        # TODO: this should be in a higher level place somewhere.
        # TODO: is 'int' really a datatype? It was used in application2.java so keeping it here for now. should check if it can go later.
        datatype_sizes = {'boolean': 1, 'int': 2, 'short': 2, 'refresh_rate': 2, 'array': 30, 'string':30 }

        # Generate the link table and component map xml
        root = ElementTree.Element('wkpftables')
        tree = ElementTree.ElementTree(root)
        appId = ElementTree.SubElement(root, 'appId')
        links = ElementTree.SubElement(root, 'links')
        components = ElementTree.SubElement(root, 'components')
        initvalues = ElementTree.SubElement(root, 'initvalues')
        appId.attrib['name'] = name

        for link in changesets.links:
            #print str(link.from_component.deployid) +"to"+str(link.to_component.deployid)
            #special handler for multiplexer
            if (link.to_component.type == 'Multiplexer' and link.to_property_name != 'selector'):
                #for multiplexers, ignore links to inputs and output, and set default values to mapped wucomponent id in FBP.
                #assumption: component id < 65535/100, property id < 100
                #so we can put component
                link.to_component.properties["id"] = str(link.to_component.deployid);
                link.to_component.properties[link.to_property.name] = str(link.from_component.deployid * 100 + link.from_property.id)
                #print "multiplexer input " + link.to_property.name+" value:"+link.to_component.properties[link.to_property.name]
                #if current_link is not selected yet, randomly pick the current one
                if "current_src" not in link.to_component.properties.keys():
                    link.to_component.properties["current_src"] = str(link.to_component.properties[link.to_property.name])
                #if both src and dest are filled, add an initial link for later change
                if  "current_dest" in link.from_component.properties.keys():
                    link_element = ElementTree.SubElement(links, 'link')
                    link_element.attrib['fromComponent'] = str(int(link.to_component.properties["current_src"])//100)
                    link_element.attrib['fromProperty'] = str(int(link.to_component.properties["current_src"])%100)
                    link_element.attrib['toComponent'] = str(int(link.to_component.properties["current_dest"])//100)
                    link_element.attrib['toProperty'] = str(int(link.to_component.properties["current_dest"])%100)
                continue        #ignore code for normal wuclasses other than "multiplexer"
            elif link.from_component.type == 'Multiplexer'  and link.from_property_name == 'output':
                link.from_component.properties[link.to_property.name] = str(link.to_component.deployid * 100 + link.to_property.id)
                link.from_component.properties["current_dest"] = str(link.from_component.properties[link.to_property.name])
                #if both src and dest are filled, add an initial link for later change(happen only once)
                if "current_src" in link.from_component.properties.keys():
                    link_element = ElementTree.SubElement(links, 'link')
                    link_element.attrib['fromComponent'] = str(int(link.from_component.properties["current_src"])//100)
                    link_element.attrib['fromProperty'] = str(int(link.from_component.properties["current_src"])%100)
                    link_element.attrib['toComponent'] = str(int(link.from_component.properties["current_dest"])//100)
                    link_element.attrib['toProperty'] = str(int(link.from_component.properties["current_dest"])%100)
                continue        #ignore code for normal wuclasses other than "multiplexer"
            #end of multiplexer handler

            link_element = ElementTree.SubElement(links, 'link')
            link_element.attrib['fromComponent'] = str(link.from_component.deployid)
            link_element.attrib['fromProperty'] = str(link.from_property.id)
            link_element.attrib['toComponent'] = str(link.to_component.deployid)
            link_element.attrib['toProperty'] = str(link.to_property.id)
        for component in changesets.components:
            component_element = ElementTree.SubElement(components, 'component')
            component_element.attrib['id'] = str(component.deployid)
            component_wuclass = WuObjectFactory.wuclassdefsbyname[component.type]
            component_element.attrib['wuclassId'] = str(component_wuclass.id)
            for endpoint in component.instances:
                endpoint_element = ElementTree.SubElement(component_element, 'endpoint')
                endpoint_element.attrib['node'] = str(endpoint.wunode.id)
                endpoint_element.attrib['port'] = str(endpoint.port_number)
        for component in changesets.components:
            if len(component.instances) == 0:
                raise IndexError('No instances for component of type ' + component.type)
            wuobject = component.instances[0]
            for property in [p for p in generateProperties(wuobject.properties.values(), component) if p.value != '']:
                if (Generator.isLinkDestination(component.deployid, property.id, changesets)):
                    if component.type != 'Multiplexer':
                        # This property is the destination for a link, so we shouldn't generate an entry in the init value table
                        # The framework will get the initial value from the source component instead.
                        continue

                initvalue = ElementTree.SubElement(initvalues, 'initvalue')
                initvalue.attrib['componentId'] = str(component.deployid)
                initvalue.attrib['propertyNumber'] = str(property.id)
                if property.wutype in datatype_sizes: # Basic type
                    initvalue.attrib['valueSize'] = str(datatype_sizes[property.wutype])
                else: # Enum
                    initvalue.attrib['valueSize'] = '2'
                if property.wutype.wutype == 'short' or property.wutype.wutype == 'int' or property.wutype.wutype == 'refresh_rate' or property.wutype.wutype == 'array' or property.wutype.wutype == 'string':
                    initvalue.attrib['value'] = str(property.value)
                elif property.wutype.wutype == 'boolean':
                    initvalue.attrib['value'] = '1' if property.value == 'true'else '0'
                else: # Enum
                    enumtype = property.wutype
                    enumvalues = [wuvalue.upper() for wuvalue in enumtype.values]
                    initvalue.attrib['value'] = str(enumvalues.index(property.value.upper())) # Translate the string representation to an integer
        #write to a well-formatted xml file
        rough_stri = ElementTree.tostring(root, 'utf-8')
        xml_content = xml.dom.minidom.parseString(rough_stri)
        pretty_stri = xml_content.toprettyxml()
        fileout = open (os.path.join(JAVA_OUTPUT_DIR, "WKDeploy.xml"), "w")
        fileout.write(pretty_stri)
        fileout.close()
