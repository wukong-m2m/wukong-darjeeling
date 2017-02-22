
class Progression(tornado.web.RequestHandler):
  def post(self):
    config = json.loads(self.request.body)
    comm = getComm()
    if WuSystem.hasMappingResult(str(config['applicationId'])):
      for entity in config['entities']:
        result = WuSystem.lookUpComponent(str(config['applicationId']), str(entity['componentId']))
        if result:
            comm.setProperty(int(result['nodeId']), int(result['portNumber']), int(result['classId']), 2, 'short', int(entity['value']))
    self.write(config)

