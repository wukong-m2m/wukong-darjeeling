#!/usr/bin/python

    # <property name="djarchive_type_lib_infusion" value="0"/>
    # <property name="djarchive_type_app_infusion" value="1"/>
    # <property name="djarchive_type_wkpf_link_table" value="2"/>
    # <property name="djarchive_type_wkpf_component_map" value="3"/>
    # <property name="djarchive_type_wkpf_initvalues" value="4"/>

import sys

def filetype2string(type):
    return [
        'library infusion',
        'application infusion',
        'wkpf link table',
        'wkpf component map',
        'wkpf initvalues'
    ][type]

def parseLinkTable(filedata):
    number_of_links = filedata[0]+256*filedata[1]
    print "\t%s: \t\t\t%d links" % (str(filedata[0:2]), number_of_links)
    for i in range(number_of_links):
        link = filedata[2+i*6:2+i*6+6]
        fromcomponent = link[0]+link[1]*256
        fromport = link[2]
        tocomponent = link[3]+link[4]*256
        toport = link[5]
        print "\t%s: \t\tlink from (%d,%d) to (%d,%d)" % (str(link),
                                                          fromcomponent,
                                                          fromport,
                                                          tocomponent,
                                                          toport)

def parseComponentMap(filedata):
    number_of_components = filedata[0]+256*filedata[1]
    print "\t%s: \t\t\t%d components" % (str(filedata[0:2]), number_of_components)

    offsettable = filedata[2:2+(number_of_components)*2]
    print "\t\t\t\t\toffset table:%s" % (str(offsettable))
    for i in range(number_of_components):
        offset = offsettable[2*i]+offsettable[2*i+1]*256
        print "\t%s: \t\t\t\tcomponent %d at offset %d" % (str(offsettable[2*i:2*i+2]), i, offset)

    componenttable = filedata[2+(number_of_components)*2:]
    pos = 0
    print "\t\t\t\t\tcomponents:"
    for i in range(number_of_components):
        number_of_endpoints = componenttable[pos]
        wuclass = componenttable[pos+1]+componenttable[pos+2]*256
        print "\t%s: \t\t\t\tcomponent %d, wuclass %d, %d endpoint(s):" % (str(componenttable[pos:pos+3]), i,  wuclass, number_of_endpoints)
        pos += 3
        for j in range(number_of_endpoints):
            node = componenttable[pos]+componenttable[pos+1]*256
            port = componenttable[pos+2]
            print "\t%s: \t\t\t\t\tnode %d, port %d" % (str(componenttable[pos:pos+3]), node, port)
            pos += 3

def parseInitvalues(filedata):
    number_of_initvalues = filedata[0]+256*filedata[1]
    print "\t%s: \t\t\t%d initvalues" % (str(filedata[0:2]), number_of_initvalues)

    initvalues = filedata[2:]
    pos = 0
    for i in range(number_of_initvalues):
        component_id = initvalues[pos]+initvalues[pos+1]*256
        property_number = initvalues[pos+2]
        value_size = initvalues[pos+3]
        print "\t%s: \t\t\tcomponent %d, property %d, size %d" % (str(initvalues[pos:pos+4]),
                                                                  component_id,
                                                                  property_number,
                                                                  value_size)
        value = initvalues[pos+4:pos+4+value_size]
        if value_size == 1:
            valuestr = str(value[0])
        elif value_size == 2:
            valuestr = str(value[0]+value[1]*256)
        else:
            valuestr = str(value)
        print "\t%s: \t\t\t\tvalue %s" % (str(value), valuestr)
        pos += 4+value_size


filename = sys.argv[1]
with open(filename, "rb") as f:
    while True:
        filelength = ord(f.read(1)) + ord(f.read(1))*256
        if filelength == 0:
            break
        filetype = ord(f.read(1))
        print "FILE length %d, type '%s'" % (filelength, filetype2string(filetype))
        filedata = [ord(x) for x in f.read(filelength)]

        if filetype == 0:
            print "\t Java archive"
        elif filetype == 1:
            print "\t Java archive"
        elif filetype == 2:
            parseLinkTable(filedata)
        elif filetype == 3:
            parseComponentMap(filedata)
        elif filetype == 4:
            parseInitvalues(filedata)

        print ""

    # file should be empty here
    remaining = f.read()
    if len(remaining) == 0:
        print "END OF ARCHIVE"
    else:
        print "UNEXPECTED DATA AFTER END OF ARCHIVE:"
        print " ".join([str(x) for x in remaining])
