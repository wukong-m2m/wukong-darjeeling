#!/usr/bin/python
# vim: ts=2 sw=2

# Translator convert the flow of WuKong components and their definitions into one application Java file
#
# Author: B.L.Tsai
# Date: Jun 10, 2012

#It seems only generateJava() is used here --- Sen 12/18/2012
import os
import sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../python")))
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../../master")))
from wkpf import *
import pickle
from xml.dom.minidom import parse, parseString
from wkpf.mapper import *
from jinja2 import Template
from jinja2 import Environment, FileSystemLoader

#from inspector import readNodeInfo

from wkpf.wkpfcomm import Communication
from wkpf.inspector import *
from optparse import OptionParser

from configuration import *

## print flag for parsed files
print_wuClasses = False
print_Components = True
print_links = True

## output file descriptor
out_fd = None
out_xml_fd = None

## node list methods. Written by Niels
def loadNodeList(filename):
  with open(filename, "r") as f:
    node_list = pickle.load(f)
    return node_list

def saveNodeList(node_list, filename):
  with open(filename, "w") as f:
    pickle.dump(node_list, f)

def getNodeList():
  if (not options.do_discovery and not options.use_hardcoded_discovery and not options.discovery_file):
    print "Please specify either -D to do network discovery, -d to use a previously stored discovery result, or store the result of the discovery, or -H to use the hardcoded discovery result (DEBUG ONLY)"
    exit(1)

  if options.do_discovery:
    comm = Communication(0)
    node_list = comm.getAllNodeInfos();
  elif options.use_hardcoded_discovery:
    #generic, numeric_controller, light_sensor
    wuClasses=(WuClass(name='', id=0, properties={}, virtual=False, soft=True, node_id=4),
               WuClass(name='', id=3, properties={}, virtual=False, soft=True, node_id=4),
               WuClass(name='', id=5, properties={}, virtual=False, soft=True, node_id=4))
    wuObjects=[WuObject(portNumber=0, wuClass=wuClasses[0], instanceId='', instanceIndex=0, nodeId=4),
               WuObject(portNumber=1, wuClass=wuClasses[1], instanceId='', instanceIndex=1, nodeId=4),
               WuObject(portNumber=2, wuClass=wuClasses[2], instanceId='', instanceIndex=2, nodeId=4)]   
    node1 = NodeInfo(4, wuClasses, wuObjects) # numeric_controller at port 1, light sensor at port 2
    node3 = NodeInfo(6,
              wuClasses=(WuClass(name='', id=0, properties={}, virtual=False, soft=True, node_id=6),
              WuClass(name='', id=1, properties={}, virtual=False, soft=True, node_id=6),
              WuClass(name='', id=4, properties={}, virtual=False, soft=True, node_id=6)), # generic, threshold, light
              wuObjects=(WuObject(portNumber=0, wuClass=wuClasses[0], instanceId='', instanceIndex=0, nodeId=6),
              WuObject(portNumber=4, wuClass=wuClasses[2], instanceId='', instanceIndex=1, nodeId=6))) # light at port 4
    node_list = (node1, node3)

    #wuclasses = parseXML(os.path.join(ROOT_PATH, "ComponentDefinitions", "WuKongStandardLibrary.xml")).values()

    #node_infos = [NodeInfo(nodeId=3,
                    #wuClasses=wuclasses,
                    #wuObjects=[])]

    #node_list = node_infos
  else:
    node_list = loadNodeList(options.discovery_file)

  if (node_list == None):
    print "Unable to load node list"
    exit(1)

  if (options.do_discovery and options.discovery_file): # Save the discovery result for future use
    saveNodeList(node_list, options.discovery_file)
  print "Node list:"
  print node_list
  return node_list

## parser, mapper, and codegen. Written by B.L.Tsai
# function for indenting text line to make a code readable
def indentor(s, num):
  return "\n".join((num * 4 * " ") + line for line in s.splitlines() if line.strip() != '')


#making dictionaries, generate [Application].class
def xmlparser():
  # get discovery result
    node_list = getNodeList()

    # parse out java class name from flow.xml dom, using xml.dom.minidom.parse 
    f_dom = parse(options.pathf)
    java_class_name = f_dom.getElementsByTagName('application')[0].getAttribute('name')

    # set output descriptors
    out_dir = options.out
    if not out_dir: out_dir = os.getcwd()
    assert os.path.exists(out_dir), "Error! the specified output directory does not exist."
    global out_fd
    global out_xml_fd
    out_fd = open(os.path.join(out_dir, java_class_name+".java"), 'w')
    out_xml_fd = open(os.path.join(out_dir, java_class_name+"Mapping.xml"), 'w')

  # About various names : Niagara Name = (xml name) for now: And_Gate(in StandardLibrary), jgclass: GENERATEDVirtualAndGateWuObject, jclass: VirtualAndGateWuObject
    # wuClasses_dict [ wuClass' name or wuClass' ID] = {
    #   'jclass': Java Class Name
    #   'jgclass': Generated Java Class Name
    #   'xml': WuClass name in XML
    #   'id' : wuClass ID
    #   'prop' : wuClass' properties
    #            {'jconst': java constant name, 'id' : property's ID, 'type' : u'short' or u'boolean' or enum's dict[enum's value] or u'array', 'access': 'rw' or 'ro' or 'wo'}
    #   'vrtl' : true or false (whether wuClass is virtual or not)
    #   'soft' : true (soft) or false (hard)
    # }

    ## Second, from Flow XML, parse out components (wuClass instances) and links btwn components
    def findProperty(prop_name, class_dict, class_name, path):
        assert class_name in class_dict, 'Error! illegal component type (wuClass name) %s in xml %s' % (wuClassName, path) #TODO should be in outer loop
        tmp_prop_dict = class_dict[class_name]['prop']
        assert prop_name in tmp_prop_dict, 'Error! property %s is not in wuClass %s in xml %s' % (prop_name,class_name,path)
        return tmp_prop_dict[prop_name]

    if print_Components: 
        print>>out_fd, "//========== Components Definitions =========="
    components_dict = {}
    truefalse_dict = {u'true':True, u'false':False}
    links_list = []
    path = options.pathf		#used for debug path of file
    for i, component in enumerate(f_dom.getElementsByTagName('component')):
        wuClassName = component.getAttribute('type')
        instanceName = component.getAttribute('instanceId')
        tmp_dflt_list = []
    ##retrieving propoerty and default value
        for prop in component.getElementsByTagName('property'):
            prop_found = findProperty(prop.getAttribute('name'), wuClasses_dict, wuClassName, path)
            prop_name = prop_found['jconst']
            prop_type = prop_found['type']
            prop_dflt_value = prop.getAttribute('default')
      #TODO change type definiation tuple(type, value)
            if type(prop_type) is dict:
                prop_dflt_value = prop_type[prop_dflt_value]    # it becomes type str
            elif prop_type == u'short':
                prop_dflt_value = int(prop_dflt_value,0)    # it becomes type int 
            elif prop_type == u'boolean':
                prop_dflt_value = truefalse_dict[prop_dflt_value.lower()] # it becomes type boolean
            elif prop_type == u'refresh_rate':
                prop_dflt_value = ('r', int(prop_dflt_value,0)) # it becomes type tuple
            elif prop_type == u'array':    #it becomes type list
                prop_dflt_value = map(int,prop_dflt_value)
            elif prop_type == u'string':
                prop_dflt_value = map(ord,prop_dflt_value)
            else:
                assert False, 'Error! property %s of unknown type %s in xml %s' % (prop_name, prop_type, path)
            tmp_dflt_list += [(prop_name, prop_dflt_value)]
        print "instanceName" + str(wuClasses_dict.keys())
        components_dict[instanceName] = {'cmpid':i, 'class':wuClassName, 'classid':wuClasses_dict[wuClassName]['id'], 'defaults':tmp_dflt_list, 'cmpname':instanceName }
        if print_Components:
            print>>out_fd, "//", i, instanceName, components_dict[instanceName]

    #create a list of links
        for link in component.getElementsByTagName('link'):
            toInstanceName = link.getAttribute('toInstanceId')
            links_list += [ (instanceName, link.getAttribute('fromProperty'), toInstanceName, link.getAttribute('toProperty')) ]
    if print_Components: print>>out_fd, "//"

    from struct import pack
    def short2byte(i): return [ord(b) for b in pack("H", i)]

    ## Finally, check the validity of links and output as a string
    if print_links: 
        print>>out_fd, "//========== Links Definitions =========="
        print>>out_fd, "// fromCompInstanceId(2 bytes), fromPropertyId(1 byte), toCompInstanceId(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)"
    link_table_str = ''
  #establishing link table (refer to HAScenario2.java)
  #eg. (byte)0,(byte)0, (byte)0,  (byte)2,(byte)0, (byte)1,  (byte)1,(byte)0,
  # from     Compo. 0      prop. 0  to Comp. 2 prop.0,  (why WuClass 1 ????? ---Sen) 
    for link in links_list:
        assert link[2] in components_dict, 'Error! cannot find target component %s linked from component %s in xml %s' % (link[2], link[0],path)

        fromInstanceId = short2byte(components_dict[ link[0] ]['cmpid'])
        fromPropertyId = findProperty(link[1], wuClasses_dict, components_dict[ link[0] ]['class'], path)['id']
        link_table_str += "(byte)%d,(byte)%d, " % (fromInstanceId[0], fromInstanceId[1])
        link_table_str += "(byte)%d,  " % fromPropertyId

        toInstanceId = short2byte(components_dict[ link[2] ]['cmpid'])
        toPropertyId = findProperty(link[3], wuClasses_dict, components_dict[ link[2] ]['class'], path)['id']
        link_table_str += "(byte)%d,(byte)%d, " % (toInstanceId[0], toInstanceId[1])
        link_table_str += "(byte)%d,  " % toPropertyId

        wuClassId = short2byte(components_dict[ link[2] ]['classid'])
        link_table_str += "(byte)%d,(byte)%d,\n" % (wuClassId[0], wuClassId[1])
        if print_links: 
            print>>out_fd, "//", link
            print>>out_fd, "//", (fromInstanceId[0],fromInstanceId[1]), fromPropertyId, (toInstanceId[0], toInstanceId[1]), toPropertyId, (wuClassId[0], wuClassId[1])
    if print_links: 
        print>>out_fd, "//"

    return java_class_name, wuClasses_dict, components_dict, indentor(link_table_str,1), out_dir, node_list

  # components_dict[instanceName] = {
    #   'cmpid': component's ID, 
    #   'class': WuClass's Name, 
    #   'classid': WuClasses's ID,
    #   'defaults': a list of tuples [ ( property's name, property's default value(int or bool value or list) ) ]
    # }

def mapper(wuClasses_dict, components_dict, node_list, locationTree, queries):
  # find nodes
    hard_dict = {}
    soft_dict = {}
    node_port_dict = {}
    for node in node_list:
        if node.isResponding == False:
            print "node "+str(node.nodeId) +"is not responding"
            continue
        for wuObject in node.nativeWuObjects:
            assert wuObject.wuClassId in node.nativeWuClasses, 'Error! the wuClass of wuObject %s does not exist on node %d' % (wuObject, node.nodeId)
            node_port_dict.setdefault(node.nodeId, []).append(wuObject.portNumber)
            print "appending nodeId to hard_dict: "+ str(node.nodeId)
            hard_dict.setdefault(wuObject.wuClassId, []).append( (node.nodeId, wuObject.portNumber) )

        print node.nativeWuClasses
        for wuClassId in node.nativeWuClasses:
            if wuClasses_dict[wuClassId]['soft']:
                soft_dict.setdefault(wuClassId, []).append((node.nodeId, False)) # (nodeId, virtual?) for now, assumed all to be native

    from operator import itemgetter
    components_list = sorted(components_dict.values(), key=itemgetter('cmpid'))

    map_table_str = ""
    map_xml_str = ""

    ## generate WKPF initialization statements
    reg_func_call = "WKPF.registerWuClass(WKPF.%s, %s.properties);\n"
    create_obj_call = "WKPF.createWuObject((short)WKPF.%s, WKPF.getPortNumberForComponent((short)%d), %s);\n" 
    set_prop_call = "WKPF.setProperty%s((short)%d, WKPF.%s, %s);\n" 
    reg_stmts = ''
    init_stmts = ''

    from mapalgo import Algorithm
    algo = Algorithm(wuClasses_dict, components_list, soft_dict, hard_dict, node_port_dict)

    for component in components_list:
        wuClass = wuClasses_dict[component['class']]
        wuClassName = wuClass['jclass']
        wuGenClassName = wuClass['jgclass']
        wuClassConstName = wuClass['jconst']
        cmpId = component['cmpid']
        if_stmts = ''

        if wuClass['soft']: # it is a soft componen
            port_num = algo.getPortNum(cmpId)
            map_table_str += "(byte)%d, (byte)%d, \n" % (algo.getNodeId(cmpId), port_num)
            map_xml_str += '\t<entry componentid="%s" componentname="%s" wuclassid="%s" wuclassname="%s" nodeid="%s" portnumber="%s" />\n' % (component['cmpid'], component['cmpname'], component['class'], component['classid'],algo.getNodeId(cmpId), port_num)
            if not algo.isCreated(cmpId): # if it is virtual?
                reg_stmts += reg_func_call % (wuClassConstName, wuGenClassName)
                wuClassInstVar = "wuclassInstance%s" % wuClass['xml']
                if_stmts += "VirtualWuObject %s = new %s();\n" % (wuClassInstVar, wuClassName)
                if_stmts += create_obj_call % (wuClassConstName, cmpId, wuClassInstVar)
            else:
                if_stmts += create_obj_call % (wuClassConstName, cmpId, "null")
        else: # it is a hard component
            map_table_str += "(byte)%d, (byte)%d, \n" % (algo.getNodeId(cmpId), algo.getPortNum(cmpId))
            map_xml_str += '\t<entry componentid="%s" componentname="%s" wuclassid="%s" wuclassname="%s" nodeid="%s" portnumber="%s" />\n' % (component['cmpid'],component['cmpname'], component['class'],component['classid'],algo.getNodeId(cmpId),algo.getPortNum(cmpId))

        # set default values
        for prop_name, prop_value in component['defaults']:
            prop_type = type(prop_value)
            if prop_type is int:
                if_stmts += set_prop_call % ("Short", cmpId, prop_name, "(short)"+str(prop_value))
            elif prop_type is bool:
                if_stmts += set_prop_call % ("Boolean", cmpId, prop_name, str(prop_value).lower()) 
            elif prop_type is unicode:
                if_stmts += set_prop_call % ("Short", cmpId, prop_name, prop_value)
            elif prop_type is tuple and prop_value[0] == 'r':
                if_stmts += set_prop_call % ("RefreshRate", cmpId, prop_name, "(short)"+str(prop_value[1]))
            elif prop_type is list:
                tmp_str = ""
                for value in prop_value:
                  tmp_str += "%s," %str(value)
                if_stmts += set_prop_call % ("Array", cmpId, prop_name, "(list)"+tmp_str) 
            else:
                assert False, 'Error! property %s of unknown type %s' % (prop_name, prop_type)

        init_stmts += "if (WKPF.isLocalComponent((short)%d)) {\n%s\n}\n" % (cmpId, indentor(if_stmts,1)) if if_stmts != '' else '// no need to init component %d\n' % cmpId

    return indentor(map_table_str, 1), reg_stmts + init_stmts, map_xml_str


def javacodegen(link_table, map_table, comp_init, java_class_name):

    import_stmt = indentor("""
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;
    """, 0)

    linkDefinitions = indentor("""
private final static byte[] linkDefinitions = {
%s
};
    """ % link_table, 1)

    componentInstanceToWuObjectAddrMap = indentor("""
private final static byte[] componentInstanceToWuObjectAddrMap = {
%s
};
    """ % map_table, 1)

    wkpf_init = indentor("""
System.out.println("%s");
WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
WKPF.loadLinkDefinitions(linkDefinitions);
    """ % java_class_name
    , 2)

    comp_init_func_name = "initialiseLocalWuObjects"
    comp_init_stmts = indentor(comp_init, 2)

    main_loop = indentor("""
VirtualWuObject wuclass = WKPF.select();
if (wuclass != null) {
    wuclass.update();
}
    """, 3)

    from jinja2 import Template
    tpl = Template("""{{ IMPORT_STATEMENTS }}

public class {{ CLASS_NAME }} {
    public static void main (String[] args) {
{{ WKPF_INIT_STATEMENTS }}
        {{ COMPONENT_INIT_FUNC_NAME }}();

        while(true){
{{ MAIN_LOOP_STATEMENTS }}
        }
    }

{{ LINK_DEFINITIONS }}

{{ MAPPING_DEFINITIONS }}

    private static void {{ COMPONENT_INIT_FUNC_NAME }}() {
{{ COMPONENT_INIT }}
    }
}
    """)

    rendered_tpl = tpl.render(IMPORT_STATEMENTS=import_stmt,
        CLASS_NAME=java_class_name,
        LINK_DEFINITIONS=linkDefinitions,
        MAPPING_DEFINITIONS=componentInstanceToWuObjectAddrMap,
        WKPF_INIT_STATEMENTS=wkpf_init,
        MAIN_LOOP_STATEMENTS=main_loop,
        COMPONENT_INIT_FUNC_NAME=comp_init_func_name,
        COMPONENT_INIT=comp_init_stmts
        )

    print>>out_fd, "//========== Code =========="
    print>>out_fd, rendered_tpl


def generateJava(application):
    application.setOutputDir(JAVA_OUTPUT_DIR)
    jinja2_env = Environment(loader=FileSystemLoader([os.path.join(os.path.dirname(__file__), 'jinja_templates')]))
    output = open(os.path.join(application.destinationDir, "WKDeploy.java"), 'w')
    wuObjects = sorted(application.wuObjects.values(), key=lambda obj: obj[0].getInstanceIndex())
        
    print 'generating', os.path.join(application.destinationDir, "WKDeploy.java")
    print wuObjects[0][0].getProperties()

    output.write(jinja2_env.get_template('application.java').render(applicationName=application.applicationName, wuObjects=wuObjects, wuLinks=application.wuLinks,
          heartbeatGroups=application.heartbeatGroups))
    output.close()


if __name__ == "__main__":

  #Sen Zhou 12.8.14 Move arg parser from parser() to main() here
   # program argument handler by OptionParser module
  parser = OptionParser("usage: %prog [options]")
  parser.add_option("-c", "--component", 
      action="store", type="string", dest="pathc", help="WuKong Component XML file path")
  parser.add_option("-f", "--flow", 
      action="store", type="string", dest="pathf", help="WuKong Flow XML file path")
  parser.add_option("-o", "--output-path", 
      action="store", type="string", dest="out", help="the dest. folder path of output java file")
  parser.add_option("-d", "--discovery-result", 
      action="store", type="string", dest="discovery_file", help="The file containing the discovery result. Will be read if -D isn't specified, or created/overwritten if -D is specified. Either -D, -H, -d or both must be used.")
  parser.add_option("-D", "--do-discovery", 
      action="store_true", dest="do_discovery", help="Do a new discovery of the network nodes. If a discovery file is specified using -d, the result will be stored there for future use. Either -D, -H, -d or both must be used.")
  parser.add_option("-H", "--use-hardcoded-discovery", 
      action="store_true", dest="use_hardcoded_discovery", help="Use hardcoded discovery result (DEBUG ONLY).")
  (options, args) = parser.parse_args()

  if not options.pathc and \
      os.path.exists(os.path.join(ROOT_PATH, "ComponentDefinitions", "WuKongStandardLibrary.xml")):
      print "Component.xml file not specified, default WuKongStandardLibrary.xml used"
      options.pathc = os.path.join(ROOT_PATH, "ComponentDefinitions", "WuKongStandardLibrary.xml")
  if not options.pathf and \
      os.path.exists(os.path.join(ROOT_PATH, "Applications", "HAScenario1.xml")):
      print "flow.xml file not specified, default HAScenario1.xml used"
      options.pathf = os.path.join(ROOT_PATH, "Applications", "HAScenario1.xml")
  if not options.pathc or not options.pathf: 
      parser.error("invalid component and flow xml, please refer to -h for help")


#Sen Zhou 12.8.14 Add location tree and node discovery here
  locTree = LocationTree("Boli_Building")
  componentDefinitions = getComponentDefinitions(options.pathc)
  loc = "Boli_Building/3F/South_Corridor/Room319"
#	loc1 = "Boli_Building/3F/East_Corridor/Room318"
#	loc2 = "Boli_Building/3F/East_Corridor/Room318"
#	loc3 = "Boli_Building/3F/East_Corridor/Room318"
#	nodeIds = comm.getNodeIds()
#	nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  nodeInfos = getNodeList()
  loc_args = [[0,1,2],[0,5,3],[3,3,2],[2,1,2]]
  sensorNodes = []
  for i in range(len(nodeInfos)):
    if nodeInfos[i].isResponding == True:
      sensorNodes.append(SensorNode(nodeInfos[i]))
      locTree.addSensor(sensorNodes[-1])


#  locTree.printTree(0)
  queries = ["Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
              "Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
              None,
              None]

  application = WuApplication(flowDom=parse(options.pathf), outputDir=options.out, templateDir=os.path.join(ROOT_PATH, 'tools', 'xml2java'), componentXml=open(options.pathc).read())
  application.parseComponents()
  application.parseApplicationXML()
  application.mappingWithNodeList(locTree, queries)


  #old stuff below
  #java_class_name, wuClasses_dict, components_dict, links_table, out_dir, node_list = xmlparser()
  #map_table, comp_init, map_table_xml = mapper(wuClasses_dict, components_dict, node_list)
  #javacodegen(links_table, map_table, comp_init, java_class_name)

#	print>>out_xml_fd, "<ComponentToNodeMapping>\n%s</ComponentToNodeMapping>" % (map_table_xml)
#	print "Translator msg: the file %s.java generated is on the path %s" % (java_class_name, out_dir)
