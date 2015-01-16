"""
This file defines the Colocation Graph Node and Colocation Graph. Each Colocation Graph Node represents a set of component,
which are a strongly connected and are able to deployed onto the same device. Colocation Graph a layer based graph composed
by a set of Colocation Node. A link between any two nodes represent a contradiction, which means we can not choose the
neighbor colocation node at the same time. To minimize the total energy cost of a flow based process, the problem itself
falls to find the maximum independent set from the Colocation Graph.
"""

from wkpf.model.models import Edge

class FlowGraph:
    """ Define a connected sub graph of a FBP
    def __init__():
        self._wuComponentSet = set()
        self._links = []

    def addLink(link):
        if not link in self._links:
            self._links.append(link)
            self._wuComponentSet.add(link.from_component)
            self._wuComponentSet.add(link.to_component)

    def getLinks():
        return self._links

class ColocationGraphNode:
    _id = 0
    def __init__(self, wuComponentSet, edgeSet, energySaved):
        ColocationGraphNode._id += 1
        self._nid = ColocationGraphNode._id
        self._wuComponetSet = wuComponentSet
        self._edgeSet = edgeSet
        self._energySaved = energySaved
        self._layer = len(edgeSet)
        self._neighbors = []   # Neighbor Colocation Graph Node
        self._parents = []   # Colocation Graph Node connected with less number of components

    def addNeighbor(self, node):
        self._neighbors.append(node)

    def isNeighborExist(self, node):
        for neighbor in self._neighbors:
            if node == neighbor:
                return True
        return False

    def getNeighbors(self):
        return self._neighbors

    def removeNeighbor(self, node):
        self._neighbors.remove(node)

    def addParent(self, node):
        self._parents.append(node)

    def isParentExist(self, node):
        for parent in self._parents:
            if node == parent:
                return True
        return False

    def getParents(self):
        return self._parents

    def equals(self, node):
        return self._nid == node._nid

    def getWeight(self):
        return self._energySaved

class ColocationGraphEdge:
    def __init__(self, inNode, outNode):
        self._inNode = inNode
        self._outNode = outNode

    def getInNode(self):
        return self._inNode

    def getOutNode(self):
        return self._outNode

    def equals(self, edge):
        return self._inNode.equals(edge.getInNode()) and self._outNode.equals(edge.getOutNode())

    def isInNode(self, node):
        return this._inNode.equals(node)

    def isOutNode(self, node):
        return this._outNode.equals(node)

class AbstractColocationGraph:
    def __init__(flowgraph, system):
        this._flowgraph = flowgraph
        this._system = system
        this._nodes = []  # ColocationGraphNode
        this._edges = []  # ColocationGraphEdge

    def rawInitCollocationGraph(self, FlowGraph graph):
        for link in graph.getLinks():
            nodeset = set()
            nodeset.add(link.from_component.index)
            nodeset.add(link.to_component.index)
            edgeset = set()
            edgeset.add(link)
            addNode(ColocationGraphNode(nodeset, link.data_volume, edgeset))

    def addNode(self, node):
        self._nodes.append(node)

    def getInterSection(self, node1, node2):
        return node1._nodes.intersection(node2._nodes)

    def getUnion(self, node1, node2):
        return node1._nodes.union(node2._nodes)

    def getEdgeUnion(self, node1, node2):
        return node1._edges.union(node2._nodes)

    def getAllEdges(self):
        return self._edges

    def isEdgeExist(self, edge):
        if edge in self._edges:
            return true
        return false

    def addEdge(self, edge):
        if not isEdgeExist(edge):
            edge.getInNode().addNeighbors(edge.getOutNode())
            edge.getOutNode().addNeighbors(edge.getInNode())
            self._edges.append(edge)
            return true
        return false

    def deleteEdge(self, edge):
        edge.getInNode().getNeighbors().remove(edge.getOutNode())
        edge.getOutNode().getNeighbors().remove(edge.getInNode())
        self._edges.remove(edge)

    def getNeighbors(self, node):
        neighbors = []
        for edge in self._edges:
            if edge.isOutLink(node):
                neighbors.append(edge.getOutNode())
            else if edge.isInLink(node):
                neighbors.append(edge.getInNode())

        return return neighbors

    def getNeighborWeight(self, node):
        sum = 0
        for node in getNeighbors(node):
            sum += node.getWeight()
        return sum

def main():
    inNode = ColocationGraphNode(set(), set(), 0)
    outNode = ColocationGraphNode(set(), set(), 1)
    edge = ColocationGraphEdge(inNode, outNode)

if __name__ == "__main__":
    main()
