class deploy_application(tornado.web.RequestHandler):
  def get(self, app_id):
    global node_infos
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      # deployment.js will call refresh_node eventually, rebuild location tree there
      vh = self.get_argument('vh')
      vw = self.get_argument('vw')
      deployment = template.Loader(os.getcwd()).load('templates/deployment2.html').generate(
              app=wkpf.globals.applications[app_ind],
              app_id=app_id, node_infos=node_infos,
              logs=wkpf.globals.applications[app_ind].logs(),
              changesets=wkpf.globals.applications[app_ind].changesets,
              set_location=False,
              default_location=LOCATION_ROOT,
              vh=vh,
              vw=vw)

      app = wkpf.globals.applications[app_ind]
      """
      # see wuapplication.py
      appmeta = {
              'app_id':app_id,
              'node_infos':node_infos,
              'logs':app.logs(),
              #'changesets':app.changesets,
              'set_location':False,
              'default_location':LOCATION_ROOT
            }
      attrs = ['status','name']
      for attr in attrs:
        appmeta[attr] = getattr(app,attr)
      """
      self.content_type = 'application/json'
      self.write({'status':0,'page': deployment})

  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    wkpf.globals.set_wukong_status("Deploying")
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # signal deploy in other greenlet task
      wusignal.signal_deploy(platforms)
      wkpf.globals.set_active_application_index(app_ind)

      self.content_type = 'application/json'
      self.write({
        'status':0,
        'version': wkpf.globals.applications[app_ind].version})

class map_application(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp
      #node_infos = getComm().getActiveNodeInfos()
      rebuildTree(node_infos)

      # Map with location tree info (discovery), this will produce mapping_results
      #mapping_result = wkpf.globals.applications[app_ind].map(wkpf.globals.location_tree, getComm().getRoutingInformation())
      mapping_result = wkpf.globals.applications[app_ind].map(wkpf.globals.location_tree, [])
      ret = []
      mapping_result = {}
      for component in wkpf.globals.applications[app_ind].changesets.components:
        obj_hash = {
          'instanceId': component.index,
          'location': component.location,
          'group_size': component.group_size,
          'replica' : component.replica,
          'name': component.type,
          'msg' : component.message,
          'instances': []
        }

        for wuobj in component.instances:
          wuobj_hash = {
            'instanceId': component.index,
            'name': component.type,
            'nodeId': wuobj.wunode.id,
            'portNumber': wuobj.port_number,
            'virtual': wuobj.virtual
          }

          # We have one instance for each component for now
          component_result = {
            'nodeId': wuobj.wunode.id,
            'portNumber': wuobj.port_number,
            'classId' : wuobj.wuclassdef.id
          }

          obj_hash['instances'].append(wuobj_hash)
          mapping_result[component.index] = component_result

        ret.append(obj_hash)
        WuSystem.addMappingResult(app_id, mapping_result)

      self.content_type = 'application/json'
      self.write({
        'status':0,
        'mapping_result': mapping_result, # True or False
        'mapping_results': ret,
        'version': wkpf.globals.applications[app_ind].version,
        'mapping_status': wkpf.globals.applications[app_ind].mapping_status})

