class Monitoring_Chart(tornado.web.RequestHandler):
  def get(self, nodeID, port):
      comm = getComm()
      node_infos = comm.getAllNodeInfos(False)
      # print node_infos
      list_name=[]
      list_id=[]
      list_port=[]
      list_loc=[]

      obj1 = Test_array('Light Sensor',int(nodeID),int(port),"room")#location tree
      self.render('templates/index3.html', applications=[obj1])

  def post(self):
    apps=wkpf.globals.mongoDBClient.wukong.readings.find().sort('timestamp',-1).limit(1)[2]
    #apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

class Monitoring(tornado.web.RequestHandler):
  def get(self):
      comm = getComm()
      node_infos = comm.getAllNodeInfos(False)
      # print node_infos
      list_name=[]
      list_id=[]
      list_port=[]
      list_loc=[]
      for node in node_infos:
        print node.id
        print node.location
        for port_number in node.wuobjects.keys():
          wuobject = node.wuobjects[port_number]
          print 'port:', port_number
          print wuobject.wuclassdef.name
          list_name.append(wuobject.wuclassdef.name)
          list_id.append(node.id)
          list_port.append(port_number)
          list_loc.append(node.location)

      obj=[]
      #obj1 = Test('Light Sensor',23,2,comm.getLocation(23))#location tree
      #obj2 = Test('Slider',23,3,'BL-7F ')
      for i in range(MONITORING_COUNT):
        obj.append( Test('Light Sensor',MONITORING_NODE[i],MONITORING_PORT[i],"room") );

      self.render('templates/index2.html', applications=obj)

  def post(self):
    apps=wkpf.globals.mongoDBClient.wukong.readings.find().sort('timestamp',-1).limit(1)[2]
    #apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

class Monitoring_Planar(tornado.web.RequestHandler):
  def get(self):
      comm = getComm()
      node_infos = comm.getAllNodeInfos(False)
      # print node_infos
      list_name=[]
      list_id=[]
      list_port=[]
      list_loc=[]
      for node in node_infos:
        print node.id
        print node.location
        for port_number in node.wuobjects.keys():
          wuobject = node.wuobjects[port_number]
          print 'port:', port_number
          print wuobject.wuclassdef.name
          list_name.append(wuobject.wuclassdef.name)
          list_id.append(node.id)
          list_port.append(port_number)
          list_loc.append(node.location)

      obj=[]
      for i in range(MONITORING_COUNT):
        obj.append( Test('Light Sensor',MONITORING_NODE[i],MONITORING_PORT[i],"room") );


      self.render('templates/index5.html', applications=obj)

  def post(self):
    apps=wkpf.globals.mongoDBClient.wukong.readings.find().sort('timestamp',-1).limit(1)[2]
    #apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

