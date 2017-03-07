from configuration import *

# It controls the status of all the devices, wuobjects, locations, applications that are managed by the master
# Any operations on models or location tree should be done within it.
class SystemManager:

  def __init__():
    self.active_ind = None
    self.applications = []
    self.location_tree = None


  def init_monitoring():
    if(MONITORING == 'true'):

      # check availability of mongo driver
      try:
        from pymongo import MongoClient
      except:
        print "Please install python mongoDB driver pymongo by using"
        print "easy_install pymongo"
        sys.exit(-1)

      # check correctness of mongoDB URL
      try:
        wkpf.globals.mongoDBClient = MongoClient(MONGODB_URL)
      except:
        print "MongoDB instance " + MONGODB_URL + " can't be connected."
        print "Please install the mongDB, pymongo module."
        sys.exit(-1)

  def active_application():
    try:
      return applications[active_ind]
    except:
      return None

  def set_active_application_index(new_index):
    self.active_ind = new_index

  # using cloned nodes
  def rebuildTree(nodes):
    nodes_clone = copy.deepcopy(nodes)
    location_tree = LocationTree(LOCATION_ROOT)
    location_tree.buildTree(nodes_clone)
    flag = os.path.exists("../../LocalData/landmarks.txt")
    if(flag):
      location_tree.loadTree()
    location_tree.printTree()


  def getAppIndex(app_id):
    # make sure it is not unicode
    app_id = app_id.encode('ascii','ignore')
    for index, app in enumerate(applications):
      if app.id == app_id:
        return index
    return None

  def delete_application(i):
    try:
      shutil.rmtree(wkpf.globals.applications[i].dir)
      wkpf.globals.applications.pop(i)
      return True
    except Exception as e:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      print traceback.print_exception(exc_type, exc_value, exc_traceback, limit=2, file=sys.stdout)
    return False

  def load_app_from_dir(dir):
    app = WuApplication(dir=dir)
    app.loadConfig()
    return app

  def update_applications():
    logging.info('updating applications:')
    application_basenames = [os.path.basename(app.dir) for app in wkpf.globals.applications]

    for dirname in os.listdir(APP_DIR):
      app_dir = os.path.join(APP_DIR, dirname)
      if dirname.lower() == 'base': continue
      if not os.path.isdir(app_dir): continue

      logging.info('scanning %s:' % (dirname))
      if dirname not in application_basenames:
        logging.info('%s' % (dirname))
        wkpf.globals.applications.append(load_app_from_dir(app_dir))
        application_basenames = [os.path.basename(app.dir) for app in wkpf.globals.applications]
