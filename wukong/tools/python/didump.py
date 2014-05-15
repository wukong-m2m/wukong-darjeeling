#!/usr/bin/python
import os
import sys
import struct

def little_endian(l):
	x = 0
	while len(l) > 0:
		x *= 256
		x += l.pop()
	return x

def first_or_default(lst, p):
    return next((x for x in lst if p(x)), None)

def parse_count_forward_pointers_and_elements(rawdata, countsize, forwardpointersize, offset=0):
	# The forward pointers don't seem to work the same way for each element.
	# Sometimes they count from the element ID, sometimes from the first real byte
	count = little_endian(rawdata[0:countsize])
	forward_pointers = [little_endian(rawdata[countsize+forwardpointersize*i:countsize+forwardpointersize*i+forwardpointersize])
						for i in range(count)]
	elements = []
	for i in range(count):
		end = forward_pointers[i+1] if i<len(forward_pointers)-1 else len(rawdata)+1
		elements.append(rawdata[forward_pointers[i]+offset:end+offset])
	return (count, forward_pointers, elements)

def lid_to_string(infusion, id):
	return "(%d, %d)" % (infusion, id)

def print_header_element(rawdata):
	print "\t\tmajor version:", rawdata[1]
	print "\t\tminor version:", rawdata[2]
	print "\t\tentry point entity id:", rawdata[3], "(None)" if rawdata[3]==255 else ""
	print "\t\tinfusion name: '%s'" % (''.join(map(chr, rawdata[4:])))

def print_class_list_element(rawdata):
	count, forward_pointers, elements = \
		parse_count_forward_pointers_and_elements(rawdata[1:], 1, 2, offset=-1)
	print "\t\tnumber of classes:", count
	print "\t\tforward pointers:", forward_pointers
	for i in range(count):
		print "\t\t\tclass ", i, elements[i]
		classdata = elements[i]
		# print class definition (InternalClassDefinition)
		print "\t\t\treference field count:", classdata.pop(0)
		print "\t\t\tnon-reference field size:", classdata.pop(0)
		print "\t\t\tsuperclass LID:", lid_to_string(classdata.pop(0), classdata.pop(0))
		cinitid = classdata.pop(0)
		print "\t\t\tclass init method id: ", cinitid, "(None)" if cinitid==255 else ""
		print "\t\t\tname LID:", lid_to_string(classdata.pop(0), classdata.pop(0))
		number_of_interfaces = classdata.pop(0)
		print "\t\t\tnumber of interfaces:", number_of_interfaces
		for j in range(number_of_interfaces):
			print "\t\t\t\tinterface %d: %s" % (j, lid_to_string(classdata.pop(0), classdata.pop(0)))
		number_of_methods = classdata.pop(0)
		print "\t\t\tnumber of methods:", number_of_methods		
		for j in range(number_of_methods):
			print "\t\t\t\tmethod %d def: %s, impl: %s" % (j, lid_to_string(classdata.pop(0), classdata.pop(0)), lid_to_string(classdata.pop(0), classdata.pop(0)))
		if i != count-1:
			print ''

def print_method_impl_list(rawdata):
	count, forward_pointers, elements = \
		parse_count_forward_pointers_and_elements(rawdata[1:], 1, 2, offset=-1)
	print "\t\tnumber of methods:", count
	print "\t\tforward pointers:", forward_pointers
	for i in range(count):
		print "\t\t\tmethod ", i, elements[i]
		if methodimpls is not None:
			methodimpl = methodimpls[i]
			methoddef = methodimpl['methoddef']
			if methoddef is not None:
				print "\t\t\t%s: %s" % (methoddef['name'], methoddef['signature'])
			else:
				print "\t\t\tmethoddef %s in infusion %s" % (methodimpl['methoddef.entity_id'], methodimpl['methoddef.infusion'])
		else:
			print "\t\t\tNO INFUSION HEADER FOUND"
		methoddata = elements[i]
		print "\t\t\treference argument count:", methoddata.pop(0)
		print "\t\t\tinteger argument count:", methoddata.pop(0)
		print "\t\t\treflocvar-refarg-isstatic:", methoddata.pop(0)
		print "\t\t\tintlocvar-intarg:", methoddata.pop(0)
		print "\t\t\ttotal number of parameters:", methoddata.pop(0)
		print "\t\t\tmax stack:", methoddata.pop(0)
		flags = methoddata.pop(0)
		print "\t\t\tflags:", ("NATIVE" if flags & 1 != 0 else ""), ("STATIC" if flags & 2 != 0 else "")
		print "\t\t\treturn type:", methoddata.pop(0)
		code_length = little_endian(methoddata[0:2])
		print "\t\t\tcode length:", code_length
		print "\t\t\tcode:", methoddata[2:2+code_length]
		methoddata = methoddata[2+code_length:]
		number_of_exceptions = methoddata.pop(0)
		print "\t\t\tnumber of exceptions:", number_of_exceptions
		for j in range(number_of_exceptions):
			print "\t\t\t\texception", j
			print "\t\t\t\tcatch type infusion:", methoddata[0]
			print "\t\t\t\tcatch type local id:", methoddata[1]
			print "\t\t\t\texception start, end PC:", little_endian(methoddata[2:4]), ",", little_endian(methoddata[4:6])
			print "\t\t\t\thandler PC:", little_endian(methoddata[6:8])
			methoddata = methoddata[8:]
		if i != count-1:
			print ''

def print_static_field_list_element(rawdata):
	print "\t\tnumber of refs:", rawdata[1]
	print "\t\tnumber of bytes:", rawdata[2]
	print "\t\tnumber of shorts:", rawdata[3]
	print "\t\tnumber of ints:", rawdata[4]
	print "\t\tnumber of longs:", rawdata[5]

def print_method_def_list_element(rawdata):
	number_of_methoddefs = rawdata[1]
	print "\t\tnumber of methoddefs:", number_of_methoddefs
	for i in range(number_of_methoddefs):
		print "\t\tmethod %d parameter count: %d" % (i, rawdata[i+2])

def print_infusion_list_element(rawdata):
	count, forward_pointers, elements = \
		parse_count_forward_pointers_and_elements(rawdata[1:], 1, 2)
	print "\t\tnumber of infusions:", count
	print "\t\tforward pointers:", forward_pointers
	for i in range(count):
		s = ''.join(map(chr, elements[i]))
		print "\t\tinfusion %d: '%s'" % (i, s)


def print_string_table_element(rawdata):
	count, forward_pointers, elements = \
		parse_count_forward_pointers_and_elements(rawdata[1:], 2, 2, offset=-1)
	count = little_endian(rawdata[1:3])
	print "\t\tnumber of strings:", count
	print "\t\tforward pointers:", forward_pointers
	for i in range(count):
		length = little_endian(elements[i][0:2])
		s = ''.join(map(chr, elements[i][2:]))
		print "\t\tlen: %d, '%s'" % (length, s)

element_types = {
	0 :('HEADER',print_header_element),
	1 :('CLASSLIST',print_class_list_element),
	2 :('METHODIMPLLIST',print_method_impl_list),
	3 :('STATICFIELDLIST',print_static_field_list_element),
	4 :('METHODDEFLIST',print_method_def_list_element),
	5 :('INFUSIONLIST',print_infusion_list_element),
	6 :('STRINGTABLE',print_string_table_element),	
	7 :('CLASSDEF',None),
	8 :('METHODIMPL',None),
	9 :('INFUSION',None),
	10 :('METHODDEF',None),
	11 :('METHOD',None),
	12 :('FIELDDEF',None)
}

def print_element(rawdata):
	id = rawdata[0]
	print "\ttype:", id, element_types[id][0]
	print "\tdata:", rawdata
	if element_types[id][1] != None:
		element_types[id][1](rawdata)
	print

methodimpls = None
methoddefs = None
# If we have a dih, parse it.
infusionfilename = sys.argv[1]
headerfilename = infusionfilename + 'h'
if os.path.isfile(headerfilename):
	from xml.etree import ElementTree
	doc = ElementTree.parse(headerfilename)
	dih = doc.getroot()
	infusion = dih.find('infusion')
	infusionname = infusion.find('header').attrib['name']
	methoddeflist = infusion.find('methoddeflist')
	methoddefs = [{'entity_id': int(x.attrib['entity_id']), 'name': x.attrib['name'], 'signature': x.attrib['signature']}
				   for x in list(methoddeflist.iter())
				   if x is not methoddeflist]
	methodimpllist = infusion.find('methodimpllist')
	methodimpls = [{'entity_id': int(x.attrib['entity_id']),
					'methoddef.entity_id': x.attrib['methoddef.entity_id'],
					'methoddef.infusion': x.attrib['methoddef.infusion'],
					'methoddef': first_or_default(methoddefs, lambda(d): d['entity_id'] == int(x.attrib['methoddef.entity_id']))
									if x.attrib['methoddef.infusion'] == infusionname
									else None}
				   for x in list(methodimpllist.iter())
				   if x is not methodimpllist]

with open(infusionfilename, "rb") as f:
	rawdata = [ord(x) for x in f.read()]
	count, forward_pointers, elements = \
		parse_count_forward_pointers_and_elements(rawdata, 1, 2)
	print "number of elements:", count
	print "forward pointers:", forward_pointers
	for i in range(count):
		print_element(elements[i])
