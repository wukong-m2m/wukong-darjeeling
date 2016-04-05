#!/usr/bin/env python
# vim: ts=4 sw=4

# Wukong middleware framework
# Codegen component frontend for component.xml
# plus backend for parsing and generating code for
# java component implementation
# and plugin for NiagaraAX tool
#
#
# Author: Penn Su
# Date: May 13, 2012

import sys, os, re
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import distutils.dir_util
import logging
from jinja2 import Template, Environment, FileSystemLoader
from optparse import OptionParser
from xml.dom.minidom import parse, parseString

from util import *
from configuration import *

# Those classes are temporaily moved here for compatibility reasons
class WuType:
    def __init__(self, name, dataType, values=()):
        self._name = name  # WuType's name
        self._dataType = dataType # a string describing data type
        self._allowed_values = values # a tuple of sequential unicode values of the specified type

    def __repr__(self):
        return "WuType %s (type=%s) val:%s" % (self._name, self._dataType, str(self._allowed_values))

    def __contains__(self, value):
        return value in self._allowed_values

    def getName(self):
        return self._name

    def getDataType(self):
        return self._dataType

    def isEnumTypedef(self):
        return self._allowed_values != ()

    def hasAllowedValues(self):
        return self._allowed_values != ()

    def getAllowedValues(self):
        return self._allowed_values

    def getValueInCConstant(self, value):
        return 'WKPF_' + '_'.join([Convert.to_constant(self._dataType), Convert.to_constant(self._name), Convert.to_constant(value)])

    def getValueInJavaConstant(self, value):
        return '_'.join([Convert.to_constant(self._dataType), Convert.to_constant(self._name), Convert.to_constant(value)])

    def getValueInJavaConstByValue(self, value):
        if self.hasAllowedValues():
            return 'GENERATEDWKPF.' + Convert.to_constant(self._dataType) + "_" + Convert.to_constant(self._name) + "_" + Convert.to_constant(value)
        else:
            return value

class WuProperty:
    def __init__(self, class_name, name, id, wutype, access, default=None, value=None, status=None):
        self._class_name = class_name
        self._name = name  # an unicode for property's name
        self._id = id      # an integer for property's id
        self._wutype = wutype # a WuType object
        self._access = access
        self._default = default
        self._current_value = value
        self._property_status = status

    def __repr__(self):
        return "WuProperty %s (id=%s, wutype=%s access=%s current_value=%s)" % (self._name, self._id, self._wutype, self._access, str(self._current_value))

    def getName(self):
        return self._name

    def getWuClassName(self):
        return self._class_name

    def getJavaName(self):
        return Convert.to_constant(self._name)

    def getCConstName(self):
        return "WKPF_" + self.getJavaConstName()

    def getJavaConstName(self):
        return "PROPERTY_" + Convert.to_constant(self._class_name) + "_" + Convert.to_constant(self._name)

    def getPropertyStatus(self):
        return self._property_status

    def setPropertyStatus(self, status):
        self._property_status = status

    def hasDefault(self):
        return self._default != None

    def getDefault(self):
        return self._default

    def setDefault(self, default):
        if self._wutype.hasAllowedValues():
            if default in self._wutype.getAllowedValues():
                self._default = default
        else:
            self._default = default

    def getId(self):
        return self._id

    def setId(self, id):
        self._id = id

    def getWuType(self):
        return self._wutype

    def getDataType(self):
        return self._wutype.getName()

    # deprecated
    def setDataType(self, typeName):
        print 'set datatype of name %s of property %s' % (typeName, self.getName())
        if typeName != self._wutype.getName():
            for wutype in WuType.all():
                if wutype.getName() == typeName:
                    self._wutype = copy.deepcopy(wutype)


    def getAccess(self):
        return self._access

    def getCurrentValue(self):
        if self._current_value == None:
            return self._default
        return self._current_value

    def setCurrentValue(self, value):
        if self._wutype.hasAllowedValues():
            if value in self._wutype:
                self._current_value = value
        else:
            self._current_value = value

# Now both WuClass and WuObject have node id attribute, because each could represent at different stages of mapping process
class WuClass:
    def __init__(self, name, id, properties, virtual, soft, privateCData, node_id=None):
        self._name = name  # WuClass's name
        self._id = id      # an integer for class' id
        self._properties = properties  # a dict of WuProperty objects accessed thru the prop's name
        self._virtual = virtual    # a boolean for virtual or native
        self._soft = soft  # a boolean for soft or hard
        self._privateCData = privateCData # an optional data type, used if the wuclass needs to store some private data in the C implementation (Java should use instance variables instead)
        self_node_id = node_id

    def __iter__(self):
        for property in self._properties.values():
            yield property

    def __contains__(self, propertyName):
        return propertyName in self._properties

    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self._name, self._id,str(self._virtual),str(self._soft),str(self._properties))

    def getPropertyByName(self, name):
        for property in self._properties:
            if property.getName() == name:
                return property
        return None

    def getJavaGenClassName(self):
        return "GENERATEDVirtual" + Convert.to_java(self._name) + "WuObject"

    def getJavaClassName(self):
        return "Virtual" + Convert.to_java(self._name) + "WuObject"

    def getCDefineName(self):
        return Convert.to_constant(self.getCName())

    def getCName(self):
        return 'wuclass_' + Convert.to_c(self._name)

    def getCFileName(self):
        return 'GENERATED' + self.getCName()

    def getCUpdateName(self):
        return self.getCName() + "_update"

    def getCSetupName(self):
        return self.getCName() + "_setup"

    def getCConstName(self):
        return "WKPF_" + self.getJavaConstName()

    def getJavaConstName(self):
        return "WUCLASS_" + Convert.to_constant(self._name)

    def getName(self):
        return self._name

    def getProperties(self):
        return self._properties

    def getId(self):
        return self._id

    def isVirtual(self):
        return self._virtual

    def isSoft(self):
        return self._soft

    def getNodeId(self):
        return self._node_id

    def setNodeId(self, id):
        self._node_id = id

    def getPrivateCData(self):
      return self._privateCData

    def hasPrivateCData(self):
      return self._privateCData != ''


    def getPrivateCDataGetMacro(self):
      return '''#define %s_getPrivateData(wuobject) ((%s *)wkpf_get_private_wuobject_data(wuobject))''' % (self.getCName(), self._privateCData)


# Now both WuClass and WuObject have node id attribute, because each could represent at different stages of mapping process
# The wuClass in WuObject is just for reference only, the node_id shouldn't be used
class WuObject:
    def __init__(self, wuClass, instanceId, instanceIndex, nodeId=None, portNumber=None, occupied = False, queries=[]):
        self._wuClass = wuClass
        self._instanceId = instanceId
        self._instanceIndex = instanceIndex
        self._nodeId = nodeId
        self._portNumber = portNumber
        self._hasWuClass = False
        self._occupied = occupied
         #queries are [locationQuery, group_sizeQuery], should be replaced by a query class when we have more policy requirements in the future.
        self._queries = queries

    def __repr__(self):
        return 'wuobject(node:'+ str(self._nodeId)+' port:'+ str(self._portNumber)+ ' wuclass id: '+ str(self.getWuClassId())+' queries:'+ str(self._queries) +')'

    def __contains__(self, propertyName):
        return propertyName in self._wuClass.getProperties()

    def __iter__(self):
        for property in self.getProperties().values():
            yield property

    def isOccupied(self):
        return self._occupied

    def setOccupied(self, va = False):
        self._occupied = va

    def hasWuClass(self):
        return self._hasWuClass

    def setHasWuClass(self, value):
        self._hasWuClass = value

    def addQueries(self, queries):
        for query in queries:
            self.addQuery(query)

    def removeQuery(self, query):
        self._queries.remove(query)

    def addQuery(self, query):
        self._queries.append(query)

    def getQueries(self):
        return self._queries

    def setQueries(self, queries):
        self._queries = queries

    def toJava(self):
        print 'wuobject toJava'
        return ', '.join([str(self.getNodeId()), str(self.getPortNumber())])

    def getWuClass(self):
        return self._wuClass

    def getWuClassName(self):
        return self._wuClass.getName()

    def getWuClassId(self):
        return self._wuClass.getId()

    def getInstanceId(self):
        return self._instanceId

    def getInstanceIndex(self):
        return self._instanceIndex

    def getNodeId(self):
        return self._nodeId

    def setNodeId(self, nodeId):
        self._nodeId = nodeId

    def getPortNumber(self):
        return self._portNumber

    def setPortNumber(self, portNumber):
        self._portNumber = portNumber

    def getPropertyByName(self, prop_name):
        return self._wuClass.getPropertyByName(prop_name)

    def getProperties(self):
        return self._wuClass.getProperties()



class CodeGen:
    @staticmethod
    def getStandardLibrary(logger, filename):
        component_string = open(filename).read()

        # Parse ComponentLibrary XML
        dom = parseString(component_string)

        wuclasses_dom = dom.getElementsByTagName("WuClass")
        wutypedefs_dom = dom.getElementsByTagName("WuTypedef")

        logger.info("==================Begin TypeDefs=====================")
        wuTypedefs = {'short': WuType('short', 'short'), 'boolean': WuType('boolean', 'boolean'), 'refresh_rate': WuType('refresh_rate', 'refresh_rate')}
        for wutypedef in wutypedefs_dom:
          logger.info("Parsing wutype %s" % (wutypedef.getAttribute('name')))
          if wutypedef.getAttribute('type').lower() == 'enum':
            wuTypedefs[wutypedef.getAttribute('name')] = WuType(wutypedef.getAttribute('name'), wutypedef.getAttribute('type'), tuple([element.getAttribute('value') for element in wutypedef.getElementsByTagName('enum')]))
          else:
            wuTypedefs[wutypedef.getAttribute('name')] = WuType(wutypedef.getAttribute('name'), wutypedef.getAttribute('type'))
        logger.info("==================End of TypeDefs=====================")


        logger.info("==================Begin WuClasses=====================")
        wuClasses = []
        for wuclass in wuclasses_dom:
          logger.info("Parsing WuClass %s" % (wuclass.getAttribute('name')))
          wuclassName = wuclass.getAttribute('name')
          wuclassId = int(wuclass.getAttribute('id'),0)
          wuclassProperties = []
          #wuclassProperties = {}
          for i, prop in enumerate(wuclass.getElementsByTagName('property')):
              propType = prop.getAttribute('datatype')
              propName = prop.getAttribute('name')
              print propType
              print propName
              wuclassProperties.append(WuProperty(wuclassName, propName, i, wuTypedefs[propType], prop.getAttribute('access')) )
              #wuclassProperties[propName] = WuProperty(wuclassName, propName, i, wuTypedefs[propType], prop.getAttribute('access'))
          privateCData = wuclass.getAttribute('privateCData')
          wuClasses.append(WuClass(wuclassName, wuclassId, wuclassProperties, True if wuclass.getAttribute('virtual').lower() == 'true' else False, True if wuclass.getAttribute('type').lower() == 'soft' else False, privateCData))
        logger.info("==================End of WuClasses=====================")
        return (wuTypedefs, wuClasses)

    @staticmethod
    def generateEnabledWuclasses(logger, enabled_wuclasses_filename, wuclasses, c_dir_src):
        # By catlikethief 2013.04.11
        # Try to generate native_wuclasses.c by parsing another xml file
        native_wuclasses_path = os.path.join(c_dir_src, 'GENERATEDenabled_wuclasses.c')
        native_wuclasses = open(native_wuclasses_path, 'w')
        # For posix we need a function to process lines of another enabled_wuclasses file at startup
        native_wuclasses_path_posix = os.path.join(c_dir_src, 'GENERATEDposix_process_enabled_wuclass.c')
        native_wuclasses_posix = open(native_wuclasses_path_posix, 'w')

        header_lines = ['#include <debug.h>\n',
            '#include <string.h>\n',
            '#include "wkcomm.h"\n',
            '#include "wkpf_wuclasses.h"\n',
            '#include "wkpf_properties.h"\n',
            '#include "native_wuclasses.h"\n'
        ]
        header_lines_posix = ['#include <debug.h>\n',
            '#include <string.h>\n',
            '#include "wkcomm.h"\n',
            '#include "wkpf_wuclasses.h"\n',
            '#include "native_wuclasses.h"\n'
        ]
        init_function_lines = ['''
        uint8_t wkpf_native_wuclasses_init() {
          uint8_t retval = WKPF_OK;
          wuobject_t *wuobject;
          wuobject = NULL;
          wuobject++;
          wuobject--;
          wuobject = NULL;
          DEBUG_LOG(DBG_WKPF, "WKPF: (INIT) Running wkpf native init for node id: %x\\n", wkcomm_get_node_id());
        ''']

        init_function_lines_posix_register_class = ['''
        wuclass_t* wkpf_process_enabled_wuclasses_xml_register_class(char* wuclassname, bool appCanCreateInstances) {
            printf("[posix init] Registering wuclass %s", wuclassname);
        ''']

        portCnt = 1
        dom = parseString(open(enabled_wuclasses_filename).read())
        enabled_wuclasses = []
        for wuclass_element in dom.getElementsByTagName("WuClass"):
            wuclass_name = wuclass_element.getAttribute('name')
            tmp = [wuclass for wuclass in wuclasses if wuclass.getName() == wuclass_name]
            if len(tmp) == 0:
                print "Wuclass %s not found in standard library." % (wuclass_name)
                sys.exit(1)
            wuclass = tmp[0]
            enabled_wuclasses.append(wuclass)

            appCanCreateInstancesAtt = wuclass_element.getAttribute('appCanCreateInstances')
            appCanCreateInstances = True if (appCanCreateInstancesAtt.lower()=='true' or appCanCreateInstancesAtt=='1') else False
            # print appCanCreateInstances, appCanCreateInstancesAtt
            startup_instances = wuclass_element.getElementsByTagName('CreateInstance');

            header_lines.append('#include "%s.h"\n' % wuclass.getCFileName())

            # Register the wuclass
            init_function_lines.append('''
                        wkpf_register_wuclass(&%s);''' % wuclass.getCName())

            # Create as many instances as the XML specifies
            for instance_element in startup_instances:
                instance_properties = instance_element.getElementsByTagName('Property')
                init_function_lines.append('''
                        retval = wkpf_create_wuobject(%s.wuclass_id, %d, 0, true);
                        if (retval != WKPF_OK)
                            return retval;
                        wkpf_get_wuobject_by_port(%d, &wuobject);
                        ''' % (wuclass.getCName(), portCnt, portCnt))
                for instance_property_element in instance_properties:
                    print
                    instance_property = wuclass.getPropertyByName(instance_property_element.getAttribute('name'))
                    instance_property_type = instance_property.getWuType()
                    if instance_property_type.getDataType() != 'short' and not instance_property_type.isEnumTypedef():
                        print 'unsupported type' + instance_property_datatype
                        sys.exit()
                    if instance_property_type.isEnumTypedef():
                        instance_property_value = instance_property_type.getValueInCConstant(instance_property_element.getAttribute('value'))
                    else:
                        instance_property_value = instance_property_element.getAttribute('value')

                    init_function_lines.append('''retval = wkpf_internal_write_property_int16(wuobject, %s, %s);
                        if (retval != WKPF_OK)
                            return retval;
                        ''' % (instance_property.getCConstName(), instance_property_value))
                    assert portCnt < 256, 'number of wuobject exceeds 256'
                init_function_lines.append('''%s.setup(wuobject);
                        ''' % (wuclass.getCName()))
                portCnt += 1

            # Set the flag if the application is allowed to create instances
            if appCanCreateInstances:
                init_function_lines.append('''
                        %s.flags |= WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE;''' % wuclass.getCName())

            header_lines_posix.append('#include "%s.h"\n' % wuclass.getCFileName())
            init_function_lines_posix_register_class.append('''

            if (!strcmp(wuclassname, "%s")) {
                wkpf_register_wuclass(&%s);
                if (appCanCreateInstances)
                    %s.flags |= WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE;
                return &%s;
            }
            ''' % (wuclass.getName(), wuclass.getCName(), wuclass.getCName(), wuclass.getCName()))

        init_function_lines.append('''
            return retval;
        }''')
        init_function_lines_posix_register_class.append('''
            printf("Unknown wuclass %s\\n", wuclassname);
            return NULL;
        }''')

        native_wuclasses.writelines(header_lines)
        native_wuclasses.writelines(init_function_lines)
        native_wuclasses.close()
        native_wuclasses_posix.writelines(header_lines_posix)
        native_wuclasses_posix.writelines(init_function_lines_posix_register_class)
        native_wuclasses_posix.close()
        return enabled_wuclasses # Will be used to generate only the enabled C files (otherwise the Gradle build will fail unless there's a native implementation for everything in the std library)


    @staticmethod
    def generate(logger, wutypedefs, wuclasses, c_dir_src, c_dir_include, java_virtualclasses_dir, java_constants_dir, java_package, enabled_wuclasses):
        enumTypedefs = [x for x in wutypedefs.keys() if wutypedefs[x].isEnumTypedef()]

        # Lines
        global_vm_header_lines = []
        global_virtual_constants_lines = []

        global_arduinoIDE_header_lines = []
        global_arduinoIDE_wuclass_native_impl_lines = []

        # Boilerplate for Java global constants file
        global_virtual_constants_lines.append('''
        package javax.wukong.virtualwuclasses;

            public class GENERATEDWKPF {
        ''')

        # Parsing to WukongVM.h for Arduino IDE
        global_arduinoIDE_wuclass_native_impl_lines.append("wuclass_t classes[] = {")

        # Parsing to WuKong Profile Framework Component Library header
        for wutype in wutypedefs.values():
          # Generate global header typedef definition for VM
          for enumvalue, value in enumerate(wutype.getAllowedValues()):
            cline = "#define " + wutype.getValueInCConstant(value) + " %d\n" % (enumvalue)
            jline = "public static final short " + wutype.getValueInJavaConstant(value) + " = %d;\n" % (enumvalue)

            global_vm_header_lines.append(cline)
            global_virtual_constants_lines.append(jline)

        for wuClass in wuclasses:
          # Lines
          wuclass_native_header_lines = []
          wuclass_native_impl_lines = []
          wuclass_virtual_super_lines = []

          # Generate global header definition for VM
          global_vm_header_lines.append("#define " + wuClass.getCConstName() + " %s\n" % (wuClass.getId()))

          # Generate global constants definition for Java
          global_virtual_constants_lines.append("public static final short " + wuClass.getJavaConstName() + " = %s;\n" % (wuClass.getId()))

          for indprop, property in enumerate(wuClass.getProperties()):
            global_vm_header_lines.append("#define " + property.getCConstName() + " " + str(indprop) + "\n")

            global_virtual_constants_lines.append("public static final byte " + property.getJavaConstName() + " = " + str(indprop) + ";\n")

          # Parsing to WukongVM.h for Arduino IDE
          clip_wuclass_getCConstName = wuClass.getCConstName()[13:]
          global_arduinoIDE_header_lines.append( "static const int " + clip_wuclass_getCConstName + "=" + wuClass.getCConstName() + ";\n")

          for indprop, property in enumerate(wuClass.getProperties()):
            clip_property_getCConstName = property.getCConstName()[14:]
            global_arduinoIDE_header_lines.append( "static const int " + clip_property_getCConstName + "=" + property.getCConstName() + ";\n")

          global_vm_header_lines.append("\n")

          global_virtual_constants_lines.append("\n")

          global_arduinoIDE_header_lines.append("\n")

          # Parsing to WuKong Profile Framework Component Library header in Java
          if wuClass.isVirtual():
            package = 'package ' + java_package + ';' if java_package else ''

            wuclass_virtual_super_lines.append('''
            %s
            import javax.wukong.wkpf.VirtualWuObject;
            import javax.wukong.wkpf.WKPF;

            public abstract class %s extends VirtualWuObject {
              public static byte[] properties = new byte[] {
            ''' % (package, wuClass.getJavaGenClassName()))

            for ind, property in enumerate(wuClass.getProperties()):
              datatype = property.getDataType()
              access = property.getAccess()

              if datatype in enumTypedefs:
                datatype = "SHORT"

              line = "WKPF.PROPERTY_TYPE_" + datatype.upper() + "|WKPF.PROPERTY_ACCESS_" + access.upper()
              if ind < len(wuClass.getProperties())-1:
                line += ","

              line += "\n"
              wuclass_virtual_super_lines.append(line)

            wuclass_virtual_super_lines.append('''
              };
            ''')

            for propind, property in enumerate(wuClass.getProperties()):
              wuclass_virtual_super_lines.append("protected static final byte %s = %d;\n" % (property.getJavaName(), propind))

            wuclass_virtual_super_lines.append('''
            }
            ''')

          # Generate C header for each native component implementation
          wuclass_native_header_lines.append('''
          #include "native_wuclasses.h"
          #include "native_wuclasses_privatedatatypes.h"

          #ifndef %sH
          #define %sH

          extern wuclass_t %s;

          %s

          #endif
          ''' % (
                  wuClass.getCDefineName(),
                  wuClass.getCDefineName(),
                  wuClass.getCName(),
                  wuClass.getPrivateCDataGetMacro() if wuClass.hasPrivateCData() else ''
                ))

          # Generate C implementation for each native component implementation
          wuclass_native_impl_lines.append('''
          #include "native_wuclasses.h"
          #include "native_wuclasses_privatedatatypes.h"

          extern void %s(wuobject_t *wuobject);
          extern void %s(wuobject_t *wuobject);

          ''' % (
                  wuClass.getCSetupName(),
                  wuClass.getCUpdateName()
                ))

          wuclass_native_impl_properties_lines = '\t'
          for ind, property in enumerate(wuClass.getProperties()):
            datatype = property.getDataType()
            access = property.getAccess()

            if datatype in enumTypedefs:
              datatype = "SHORT"

            line = "WKPF_PROPERTY_TYPE_" + datatype.upper() + "+WKPF_PROPERTY_ACCESS_" + access.upper()
            if ind < len(wuClass.getProperties())-1:
              line += ","

            if ind < len(wuClass.getProperties())-1:
              line += "\n\t\t"

            wuclass_native_impl_properties_lines += line

          wuclass_native_impl_lines.append('''
          wuclass_t %s = {
            %s,
            %s,
            %s,
            %d,
            %s,
            0, // Initialise flags to 0, possibly set WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE from native_wuclasses_init
            NULL,
            {
            %s
            }
          };
          ''' % (wuClass.getCName(),
                wuClass.getCConstName(),
                wuClass.getCSetupName(),
                wuClass.getCUpdateName(),
                len(wuClass.getProperties()),
                "sizeof(%s)" % (wuClass.getPrivateCData()) if wuClass.hasPrivateCData() else "0",
                wuclass_native_impl_properties_lines))

          global_arduinoIDE_wuclass_native_impl_lines.append(
          '''
          {
            %s,
            wkpf_dump,
            NULL,
            %d,
            %s,
            0, // Initialise flags to 0, possibly set WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE from native_wuclasses_init
            NULL,
            {
            %s
            }
          },
          ''' % (wuClass.getCConstName(),
                len(wuClass.getProperties()),
                "sizeof(%s)" % (wuClass.getPrivateCData()) if wuClass.hasPrivateCData() else "0",
                wuclass_native_impl_properties_lines))

          #wuclass_native_impl_lines.append('''
          ##endif
          #''')

          if c_dir_src:
            wuclass_native_header_path = os.path.join(c_dir_include, wuClass.getCFileName() + '.h')
            wuclass_native_header = open(wuclass_native_header_path, 'w')
            wuclass_native_header.writelines(wuclass_native_header_lines)
            wuclass_native_header.close()

            if wuClass in enabled_wuclasses:
                wuclass_native_impl_path = os.path.join(c_dir_src, wuClass.getCFileName() + '.c')
                wuclass_native_impl = open(wuclass_native_impl_path, 'w')
                wuclass_native_impl.writelines(wuclass_native_impl_lines)
                wuclass_native_impl.close()

          if java_virtualclasses_dir and wuClass.isVirtual():
            wuclass_virtual_super_path = os.path.join(java_virtualclasses_dir, wuClass.getJavaGenClassName() + '.java')
            wuclass_virtual_super = open(wuclass_virtual_super_path, 'w')
            wuclass_virtual_super.writelines(wuclass_virtual_super_lines)
            wuclass_virtual_super.close()


        global_virtual_constants_lines.append('''
        }
        ''')

        global_arduinoIDE_wuclass_native_impl_lines.append("{0,0,0,0,0,0,NULL,{0}}" + "\n" + "};")
        if c_dir_src:
            global_vm_header_filename = 'GENERATEDwkpf_wuclass_library.h'
            global_vm_header_path = os.path.join(c_dir_include, global_vm_header_filename)
            global_vm_header = open(global_vm_header_path, 'w')
            global_vm_header.writelines(global_vm_header_lines)
            global_vm_header.close()

            global_arduinoIDE_header_filename = 'GENERATEDwkpf_wuclass_library_arduinoIDE.h'
            global_arduinoIDE_header_path = os.path.join(c_dir_include, global_arduinoIDE_header_filename)
            global_arduinoIDE_header = open(global_arduinoIDE_header_path, 'w')
            global_arduinoIDE_header.writelines(global_arduinoIDE_header_lines)
            global_arduinoIDE_header.close()

            global_arduinoIDE_wuclass_native_impl_filename = 'GENERATEDwuclass_arduinoIDE.h'
            global_arduinoIDE_wuclass_native_impl_path = os.path.join(c_dir_src, global_arduinoIDE_wuclass_native_impl_filename)
            global_arduinoIDE_wuclass_native_impl = open(global_arduinoIDE_wuclass_native_impl_path, 'w')
            global_arduinoIDE_wuclass_native_impl.writelines(global_arduinoIDE_wuclass_native_impl_lines)
            global_arduinoIDE_wuclass_native_impl.close()

        if java_constants_dir:
            global_virtual_constants_filename = 'GENERATEDWKPF.java'
            global_virtual_constants_path = os.path.join(java_constants_dir, global_virtual_constants_filename)
            global_virtual_constants = open(global_virtual_constants_path, 'w')
            global_virtual_constants.writelines(global_virtual_constants_lines)
            global_virtual_constants.close()



if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option('-i', '--input_xml', dest='component_file')
    parser.add_option('-e', '--enabled_xml', dest='enabled_file')
    parser.add_option('-c', '--c_dir', dest='c_dir')
    parser.add_option('-j', '--java_virtualclasses_dir', dest='java_virtualclasses_dir')
    parser.add_option('-x', '--java_constants_dir', dest='java_constants_dir')
    parser.add_option('-p', '--java_package', dest='java_package')
    (options, args) = parser.parse_args()

    # print options, args

    wuTypedefs, wuClasses = CodeGen.getStandardLibrary(logging.getLogger(), options.component_file)


    c_dir_src = None
    c_dir_include = None
    if options.c_dir:
        c_dir_src = options.c_dir + '/c'
        c_dir_include = options.c_dir + '/include'
        if not os.path.exists(c_dir_src):
            os.makedirs(c_dir_src)
        if not os.path.exists(c_dir_include):
            os.makedirs(c_dir_include)

    enabled_wuclasses = []
    if options.enabled_file and os.path.exists(options.enabled_file) and options.c_dir:
        enabled_wuclasses = CodeGen.generateEnabledWuclasses(logging.getLogger(), options.enabled_file, wuClasses, c_dir_src)

    if os.path.exists(options.component_file):
        CodeGen.generate(logging.getLogger(),
                         wuTypedefs,
                         wuClasses,
                         c_dir_src,
                         c_dir_include,
                         options.java_virtualclasses_dir,
                         options.java_constants_dir,
                         options.java_package,
                         enabled_wuclasses)
    else:
        print "path to component library doesn't exist: ", options.component_file
