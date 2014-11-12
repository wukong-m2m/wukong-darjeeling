"""
This FlowBasedProcess contains data structure for mapping algorithm. Actually, these data structure can be built
during parseApplication. But to make it clean and easy to upgrade, when we have progression server connected, we separate
them in the new class. To parallel serve the mapping request, such an information container helps to store
the intermediate results.
"""
from models import WuObjectFactory

class FlowBasedProcess:
    def __init__(changeset):
        self._edges = changeset.links
        self._component_dic = {}  # map component id to component
        self._component_class_dic = {} # map class id to a list of component in the fbp
        self._from_edge_dic = {} # map component id to a list of from edge list
        self._to_edge_dic = {} # map component id to a list of to edge list
        self._location_constraints_dic = {} # unused for now

        for component in changeset.components:
            self._component_dic[component.id] = component
            wuclassdef = WuObjectFactory.wuclassdefsbyname(component.type)
            if wuclassdef.id not in self._component_class_dic.keys():
                self._component_class_dic[wuclassdef.id] = []

            self._component_class_dic[wuclassdef.id].append(component)


        for link in changeset.links:
            # add to edge for from component
            if link.from_component.id not in self._to_edge_dic.keys():
                self._to_edge_dic[link.from_component.id] = []
            self._to_edge_dic[link.from_component.id].append(link)

            # add from edge for to component
            if link.to_component.id not in self._from_edge_dic.keys():
                self._from_edge_dic[link.to_component.id] = []
            self._from_edge_dic[link.to_component.id].append(link)
            