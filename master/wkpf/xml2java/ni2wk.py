#!/usr/bin/python

# Niagara2WuKongFlow is a temporary solution that converts the flow of Niagara FBP XML to the WuKong Flow XML
#
# Author: B.L.Tsai
# Date: May 27, 2012

import os
import xml.etree.ElementTree as ET
from optparse import OptionParser

def parser():
    parser = OptionParser("usage: %prog [options]")
    parser.add_option("-i", "--input", action="store", type="string", dest="i", metavar="INPUT", help="Input: Niagara FBP XML file path")
    parser.add_option("-n", "--name", action="append", type="string", dest="nl", metavar="NAMELIST", help='Name(s) of the scenario(s) "-n A -n B ..."', default=[])
    parser.add_option("-o", "--output-path", action="store", type="string", dest="out", metavar="OUTPUT_DIR", help="the dest. folder path of output java file")
    
    (options, args) = parser.parse_args()
    if not options.i: parser.error("invalid Nigara XML, please refer to -h for help")
    out_dir = options.out
    if not out_dir: out_dir = os.getcwd()
    assert os.path.exists(out_dir), 'Error! the specified output directory "-o %s" does not exist.' % out_dir
    if len(options.nl) == 0: parser.error("invalid Scenario name(s), please refer to -h for help")

    root = ET.parse(options.i).getroot()
    
    plist = root.findall(".//p[@t='b:Folder']")
    slist = [p for p in plist if p.get('n') in options.nl]
    olist = []

    for scenario in slist:
        clist = scenario.findall("p")
        id2comp = {
            c.get('h'):{'type':c.get('t'), 'iname':c.get('n'), 'link':{},
                        'val':{ i.get('n'):i.get('v') for i in list(set(c.findall("p"))-set(c.findall("p[@t]"))) }
            } for c in clist 
        } 
        # id2comp format
        # id: { 'type': type (wuClass),
        #       'iname': instancdId, 
        #       'val': default_value_dict {prop_name: prop_default}
        #       'link': links_dict {fromProperty: (toInstanceId, toProperty) 
        #     }
        ## rename the type, add links to source component in the view of target component
        for component in clist:
            dest = id2comp[component.get('h')]
            dest['type'] = dest['type'][dest['type'].find(':')+1:]
            dname = dest['iname']
            dval = dest['val']
            for i in component.findall("p[@t='b:Link']"):
                srcid = i.find("p[@n='sourceOrd']").get('v')[2:]
                assert srcid in id2comp, "Error! srcid %s is not found under scenario %s" % (srcid, component.get('n'))
                src = id2comp[srcid]
                sprop = i.find("p[@n='sourceSlotName']").get('v')
                dprop = i.find("p[@n='targetSlotName']").get('v')
                sname = src['iname']
                sval = src['val']
                src['link'][sprop] = (dname, dprop)
                # only destination property cannot be set with default value
                if dprop in dval: del dval[dprop]
        olist += [(scenario.get('n'), id2comp)]
    return out_dir, olist

def codegen(out_dir, slist):
    from xml.dom.minidom import parseString
    for scenario in slist:
        root = ET.Element('application', attrib={'name':scenario[0]})
        out_filename = scenario[0] + ".xml"
        for c in scenario[1].values():
            component = ET.SubElement(root, 'component', attrib={'type':c['type'], 'instancdId':c['iname']})
            for k, v in c['val'].iteritems():
                ET.SubElement(component, 'property', attrib={'name':k, 'default':v})
            for k, v in c['link'].iteritems():
                ET.SubElement(component, 'link', attrib={'fromProperty':k, 'toInstanceId':v[0], 'toProperty':v[1]})
        fd = open(os.path.join(out_dir, out_filename), 'w')
        parseString(ET.tostring(root)).writexml(fd, addindent=" "*4, newl='\n')
        fd.close()
        del root
        print "Niagara2WuKongFlow msg: the file %s is generated in the folder %s" % (out_filename, out_dir)

if __name__ == "__main__":
    out_dir, slist = parser()
    codegen(out_dir, slist)
