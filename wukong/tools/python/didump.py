#!/usr/bin/python
import sys
import struct

element_ids = {
	0 :'HEADER',
	1 :'CLASSLIST',
	2 :'METHODIMPLLIST',
	3 :'STATICFIELDLIST',
	4 :'METHODDEFLIST',
	5 :'INFUSIONLIST',
	6 :'STRINGTABLE',
	
	7 :'CLASSDEF',
	8 :'METHODIMPL',
	9 :'INFUSION',
	10 :'METHODDEF',
	11 :'METHOD',
	12 :'FIELDDEF'
}

def printElement(rawdata):
	id = rawdata[0]
	print "\ttype:", id, element_ids[id]
	print "\tdata:", rawdata[1:]


filename = sys.argv[1]
with open(filename, "rb") as f:
	rawdata = f.read()
	number_of_elements = struct.unpack('B', rawdata[0])[0]
	print "number of elements:", number_of_elements
	forward_pointers = [struct.unpack('<H', rawdata[1+2*i : 1+2*i+2])[0]
						for i in range(number_of_elements)]
	print "forward pointers:", forward_pointers
	for i in range(number_of_elements):
		begin = forward_pointers[i]
		end = forward_pointers[i+1] if i<len(forward_pointers)-1 else len(rawdata)+1
		element = [ord(x) for x in rawdata[begin:end]]
		printElement(element)
