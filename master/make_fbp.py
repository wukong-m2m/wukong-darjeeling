#!/usr/bin/python
# -*- coding: utf-8 -*-

import glob
import os
import logging
from configuration import *

def createFBP2(aList):
    head_path = os.path.join(ROOT_PATH, "wukong/master/templates/fbp2.html")
    fd = open(head_path, "rb")
    c = fd.read()
    fd.close()
    
    start = '<!--make_fbp.py-insert-start-->'
    end = '<!--make_fbp.py-insert-end-->'
    pos = c.find(start)
    path = os.path.join(ROOT_PATH, "wukong/master/templates/fbp2.html")
    g = open(path, "wb")
    g.write(c[:pos+len(start)]+'\n')
    
    manifest_path = os.path.join('static','js','components_list.txt')
    manifest_fd = open(manifest_path,'rb')
    paths = manifest_fd.readlines()
    manifest_fd.close()
    
    for path in paths:
        g.write('        '+'<script type="text/javascript" src="/static/js/'+path.rstrip()+'"></script>'+'\n')
    #for list in aList:
    #    list = list.replace('./','/')
    #    g.write('\t\t'+'<script type="text/javascript" src="'+list+'"></script>'+'\n')

    pos = c.find(end,pos)
    g.write(c[pos:])

    g.close()

def addHead_OLD():
    head_path = os.path.join(ROOT_PATH, "wukong/master/templates/fbp2.html")
    g = open(head_path, "a")
    g.write('\n'+
            '<html>'+'\n'+
            '\t'+'<head>'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/smoothness/jquery-ui-1.9.2.custom.min.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/jquery.contextMenu.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/fbp.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/jquery.treeview.css" rel="stylesheet"/>\n'+
            '\t\t'+'<link type="text/css" href="/static/css/multiple-select.css" rel="stylesheet"/>\n'+

            '\t\t'+'<script type="text/javascript" src="/static/js/jquery-1.7.2.min.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jquery-ui-1.9.2.custom.min.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jquery.contextMenu.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jquery.treeview.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jquery.multiple.select.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jcanvas.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/json2.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/block.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/line.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/fbp.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/flowlist.js"></script>'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/ace/ace.js"></script>'+'\n'
            )
    g.close()

def addMiddle(aList):
    for list in aList:
        list = list.replace('./','/')
        g = open("./templates/fbp2.html","a")
        g.write('\t\t'+'<script type="text/javascript" src="'+list+'"></script>'+'\n')
        g.close()

def addEnd():
    end_path = os.path.join(ROOT_PATH, "wukong/master/templates/fbp2.html")
    g = open(end_path, "a")
    g.write(
                '\t'+'</head>'+'\n'+
                '\t'+'<body style="overflow:hidden">'+'\n'+
                '\t\t'+'<canvas id=canvas style="position:absolute; left:205; top:50;" height=600 width=2000></canvas>'+'\n'+
                '\t\t'+'<div id=client></div>'+'\n'+
                '\t\t'+'<canvas id=canvastop style="position:absolute; left:0; top:50;" height=600 width=2000></canvas>'+'\n'+
                '\t\t'+'<div id=connection>'+'\n'+
                '\t\t'+'<table>'+'\n'+
                '\t\t'+'<tr><td>From</td></tr>'+'\n'+
                '\t\t'+'<tr>'+'\n'+
                '\t\t'+'<td><select id=connection_src></select></td>'+'\n'+
                '\t\t'+'</tr>'+'\n'+
                '\t\t'+'<tr><td>To</td></tr>'+'\n'+
                '\t\t'+'<tr>'+'\n'+
                '\t\t'+'<td><select id=connection_act></select></td>'+'\n'+
                '\t\t'+'</tr>'+'\n'+
                '\t\t'+'</table>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id=msg style="position:absolute;background-color:#ffff00"></div>'+'\n'+
                '\t\t'+'<div id=fileloader>'+'\n'+
                '\t\t'+'<input type=text id=fileloader_file></input>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id=propertyeditor>'+'\n'+
                '\t\t'+'<div> Location Policy</div>'+'\n'+
                '\t\t'+'<input type=text id=propertyeditor_location></input>'+'\n'+
                '<div id=propertyeditor_loc>Location: <input type=text id=propertyeditor_location_hierarchy size=35></input>'+
                '<button class="chooseLocNode" for="propertyeditor_location_hierarchy">Choose Tree Node</button><br>'+
                'Function: <input type=text id=propertyeditor_location_function size=35></input></div>'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id="progress"><div id="compile_status"></div>'+'\n'+
                '\t\t'+'<div id="normal"></div><div id="critical_error"></div>'+'\n'+
                '\t\t'+'<div id="urgent_error"></div></div>'+'\n'+
                 '<div class="modal ui-widget-content" id="tree_dialog" role="dialog" style="display:none">'+'\n'+
                '\t\t'+'<div class="modal-header">'+'\n'+
                    '\t\t'+'<h3>Choose a node</h3>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div class="modal-body">'+'\n'+
                  '\t\t'+'<div id="treeInDialogDiv">'+'\n'+
                    '\t\t'+'<ul id="treeInDialog" class="treeview">'+'\n'+
                    '\t\t'+'</ul>'+'\n'+
                  '\t\t'+'</div>'+'\n'+
                  '\t\t'+'Selected TreeNode: <input id = "selectedTreeNode" type=text size="20"/>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div class="modal-footer">'+'\n'+
                    '\t\t'+'<button id="confirm_tree_dialog" data-dismiss="modal" class="dialogCloseButton">Select</button>'+'\n'+
                    '\t\t'+'<button data-dismiss="modal" class="dialogCloseButton">Close</button>'+'\n'+
                '\t\t'+'</div>'+'\n'+
            '\t\t'+'</div>'+'\n'+

                '\t'+'</body>'+'\n'+
                '</html>'
                )
    g.close()



def check(name):
    isFile = os.path.exists(name)
    return isFile

class fbp_main:
    def make(self):
        #existFile = os.path.exists("./templates/fbp2.html")
        #if existFile:
        #    os.remove("./templates/fbp2.html")

        #addHead()
#       _jsList = glob.glob("./static/js/*.js")
        _testList = glob.glob("./static/js/__comp__*.js")
#       addMiddle(_jsList)
        #addMiddle(_testList)
        #addEnd()

        createFBP2(_testList)
        logging.info("make_fbp_complete")
