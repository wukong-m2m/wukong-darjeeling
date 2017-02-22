
class poll_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    status = comm.currentStatus()
    if status != None:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': status.split('\n')})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': []})

class stop_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    node_infos = comm.updateAllNodeInfos()
    rebuildTree(node_infos)
    if comm.onStopMode():
      self.content_type = 'application/json'
      self.write({'status':0,'logs':[]})
    else:
      self.content_type = 'application/json'
      self.write({'status':1,'logs':[]})

class exclude_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    if comm.onDeleteMode():
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': ['Going into exclude mode']})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'logs': ['There is an error going into exclude mode']})

class include_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    if comm.onAddMode():
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': ['Going into include mode']})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'logs': ['There is an error going into include mode']})

class testrtt(tornado.web.RequestHandler):
  def get(self):
    global node_infos

    comm = getComm()
    node_infos = comm.getAllNodeInfos(False)

class nodes(tornado.web.RequestHandler):
  def get(self):
    pass

  def post(self, nodeId):
    info = None
    comm = getComm()
    info = comm.getNodeInfo(nodeId)

    self.content_type = 'application/json'
    self.write({'status':0, 'node_info': info})

  def put(self, nodeId):
    global node_infos
    location = self.get_argument('location')
    print node_infos
    print 'in nodes: simulation:'+SIMULATION
    if SIMULATION == "true":
      for info in node_infos:
        if info.id == int(nodeId):
          info.location = location
          senNd = SensorNode(info)
          WuNode.saveNodes()
          wkpf.globals.location_tree.addSensor(senNd)
      wkpf.globals.location_tree.printTree()
      self.content_type = 'application/json'
      self.write({'status':0})
      return
    comm = getComm()
    if location:
       #print "nodeId=",nodeId
       info = comm.getNodeInfo(int(nodeId))
       #print "device type=",info.type
       if info.type == 'native':
         info.location = location
         WuNode.saveNodes()
         senNd = SensorNode(info)
         wkpf.globals.location_tree.addSensor(senNd)
         #wkpf.globals.location_tree.printTree()
         self.content_type = 'application/json'
         self.write({'status':0})
       else:
         if comm.setLocation(int(nodeId), location):
          # update our knowledge too
            for info in comm.getActiveNodeInfos():
              if info.id == int(nodeId):
                info.location = location
                senNd = SensorNode(info)
                print (info.location)
            wkpf.globals.location_tree.addSensor(senNd)
