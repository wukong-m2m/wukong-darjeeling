# components_dict[instanceName] = {
#   'cmpid': component's ID, 
#   'class': WuClass's Name, 
#   'classid': WuClasses's ID,
#   'defaults': a list of tuples [ ( property's name, property's default value(int or bool value) ) ]
# }
#
# wuClasses_dict [ wuClass' name or wuClass' ID] = {
#   'jclass': Java Class Name
#   'jgclass': Generated Java Class Name
#   'xml': WuClass name in XML
#   'id' : wuClass ID
#   'prop' : wuClass' properties
#            {'jconst': java constant name, 'id' : property's ID, 'type' : u'short' or u'boolean' or enum's dict[enum's value] or u'array' , 'access': 'rw' or 'ro' or 'wo'}
#   'vrtl' : true or false (whether wuClass is virtual or not)
#   'soft' : true (soft) or false (hard)
# }
#
# soft_dict [ hard wuClass' ID ] = [ ( nodeId, true or false (virtual or not) ) ]
#   
# hard_dict [ soft wuClass' ID ] = [ ( nodeId, wuObject's port number ) ]
#
# node_port_dict [ node's ID ] = [ wuObject's port number ]
# 

class Algorithm:
    def __init__(self, wuClasses_dict, components_list, soft_dict, hard_dict, node_port_dict):
        self.__components = components_list
        for component in components_list:
            wuClass = wuClasses_dict[component['class']]
            cmpId = component['cmpid']

            if wuClass['soft']: # if it is a soft component
                if wuClass['id'] in soft_dict:
                    # pick the first node that has the wuClass
                    nodeId, vrtlflag = soft_dict[wuClass['id']][0]
                    component['iscreated'] = True
                else:
                    # if not found, create a virtual wuClass and assign it to a node
                    nodeId = node_port_dict.iterkeys().next()
                    vrtlflag = True
                    component['iscreated'] = False
                component['nodeid'] = nodeId

                # find the smallest available port number
                port_list = node_port_dict[nodeId]
                port_num = None
                #print "------", component['class'], soft_component, port_list
                #print node_port_dict
                for i in range(256): # Port numbers are in the range 0-255 (short) 
                    if i not in port_list:
                        port_list += [i]
                        port_list.sort()
                        port_num = i
                        break
                assert port_num != None, "Error! cannot find an unique port number"
                component['port'] = port_num
            else: # if it is a hard component
                # pick the first node that has the object of the wuClass
                component['nodeid'], component['port'] = hard_dict[wuClass['id']][0]
    
    def __findComponent(self,cmpId):
        for c in self.__components:
            if c['cmpid'] == cmpId:
                return c
        return None

    def getPortNum(self,cmpId):
        c = self.__findComponent(cmpId)
        if c: return c['port']
        return None

    def getNodeId(self,cmpId):
        c = self.__findComponent(cmpId)
        if c: return c['nodeid']
        return None

    def isCreated(self,cmpId):
        c = self.__findComponent(cmpId)
        if c: return c['iscreated']
        return None
