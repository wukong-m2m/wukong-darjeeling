
def delete_application(i):
    shutil.rmtree(wkpf.globals.applications[i].dir)
    wkpf.globals.applications.pop(i)
    return True

class list_applications(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/index.html', applications=wkpf.globals.applications)

  def post(self):
    update_applications()
    apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

class new_application(tornado.web.RequestHandler):
  def post(self):
    try:
      try:
        app_name = self.get_argument('app_name')
      except:
        app_name = 'application' + str(len(wkpf.globals.applications))

      app_id = hashlib.md5(app_name).hexdigest()

      if getAppIndex(app_id) is not None:
        ## assign a serial number to the application name
        count = 1
        while True:
            new_app_name = app_name +'(%s)' % count
            app_id = hashlib.md5(new_app_name).hexdigest()
            if getAppIndex(app_id) is None:
                app_name = new_app_name
                break
            count += 1

      # copy base for the new application
      logging.info('creating application... "%s"' % (app_name))
      copyAnything(BASE_DIR, os.path.join(APP_DIR, app_id))

      # default to be disabled
      app = WuApplication(id=app_id, app_name=app_name, dir=os.path.join(APP_DIR, app_id),disabled=True)
      logging.info('app constructor')
      logging.info(app.app_name)

      wkpf.globals.applications.append(app)

      #
      # HY: save the content (when user "upload")
      #
      app_ind = getAppIndex(app_id)
      try:
        xml = self.get_argument('xml',default=None)
        # HY:
        # rewrite the app_id in xml
        #
        if xml:
            keyword = 'application name="'
            start_pos = xml.find(keyword)
            end_pos = xml.find('"',start_pos+len(keyword)+1)
            if start_pos != -1:
                start_xml = xml[:start_pos+len(keyword)]
                end_xml = xml[end_pos:]
                xml = start_xml+app_id+end_xml
            wkpf.globals.applications[app_ind].updateXML(xml)
      except:
        #
        # HY:
        # do cleanup (delete newly created app)
        #
        delete_application(app_ind)
        raise

      # dump config file to app
      logging.info('saving application configuration...')
      app.saveConfig()

      self.content_type = 'application/json'
      self.write({'status':0, 'app': app.config()})
    except Exception as e:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      traceback.print_exception(exc_type, exc_value, exc_traceback,
                                  limit=2, file=sys.stdout)
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg':'Cannot create application:%s,%s' % (exc_value,exc_traceback)})

class rename_application(tornado.web.RequestHandler):
  def put(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      try:
        wkpf.globals.applications[app_ind].app_name = self.get_argument('value', '')
        wkpf.globals.applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0,'app_name':self.get_argument('value', '')})
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.set_status(400)
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot save application'})

class disable_application(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      disabled = self.get_argument('disabled', '') == '1'
      try:
        wkpf.globals.applications[app_ind].disabled = disabled
        wkpf.globals.applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0,'disabled':disabled})
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.set_status(400)
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot '+('disable' if disabled else 'enable')+' application'})

class reset_application(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)

    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      wkpf.globals.set_wukong_status("close")
      wkpf.globals.applications[app_ind].status = "close"
      self.content_type = 'application/json'
      self.write({'status':0, 'version': wkpf.globals.applications[app_ind].version})

class application(tornado.web.RequestHandler):
  # topbar info
  def get(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      title = ""
      if self.get_argument('title'):
        title = self.get_argument('title')
      app = wkpf.globals.applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=wkpf.globals.applications[app_ind], title=title, default_location=LOCATION_ROOT)
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Display a specific application
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      # active application
      wkpf.globals.set_active_application_index(app_ind)
      app = wkpf.globals.applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=wkpf.globals.applications[app_ind], title="Flow Based Programming")
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Update a specific application
  def put(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      try:
        wkpf.globals.applications[app_ind].app_name = self.get_argument('name', '')
        wkpf.globals.applications[app_ind].desc = self.get_argument('desc', '')
        wkpf.globals.applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0})
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot save application'})

  # Destroy a specific application
  def delete(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      if delete_application(app_ind):
        self.content_type = 'application/json'
        self.write({'status':0})
      else:
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot delete application'})

# Never let go
class poll(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      application = wkpf.globals.applications[app_ind]

      self.content_type = 'application/json'
      self.write({
        'status':0,
        'ops': application.deploy_ops,
        'version': application.version,
        'deploy_status': application.deploy_status,
        'mapping_status': application.mapping_status,
        'wukong_status': wkpf.globals.get_wukong_status(),
        'application_status': application.status,
        'returnCode': application.returnCode})

      # TODO: log should not be requested in polling, should be in a separate page
      # dedicated for it
      # because logs could go up to 10k+ entries
      #'logs': wkpf.globals.applications[app_ind].logs()

class save_fbp(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      xml = self.get_argument('xml')
      wkpf.globals.applications[app_ind].updateXML(xml)
      #applications[app_ind] = load_app_from_dir(applications[app_ind].dir)
      #applications[app_ind].xml = xml
      # TODO: need platforms from fbp
      #platforms = self.get_argument('platforms')
      platforms = ['avr_mega2560']

      self.content_type = 'application/json'
      self.write({'status':0, 'version': wkpf.globals.applications[app_ind].version})

class load_fbp(tornado.web.RequestHandler):
  def get(self, app_id):
    vh = self.get_argument('vh')
    vw = self.get_argument('vw')
    fbp2 = template.Loader(os.getcwd()).load('templates/fbp2.html').generate(
          vh=vh,vw=vw,app_id=app_id
        )
    self.content_type = 'text/html'
    self.write(fbp2)

  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'xml': wkpf.globals.applications[app_ind].xml})

class download_fbp(tornado.web.RequestHandler):
  def get(self, app_id):
    app_ind = getAppIndex(app_id)
    app = wkpf.globals.applications[app_ind]
    file_name = app_id
    xml = ''
    if app:
        file_name = app.app_name
        xml = app.xml
    self.set_header('Content-Type', 'application/octet-stream')
    self.set_header('Content-Disposition', 'attachment; filename=' + file_name+'.xml')

    #
    # Add the app_name into the xml
    #
    insert_pos = xml.find('>')+1
    xml = xml[:insert_pos]+'<app_name>'+app.app_name+'</app_name>'+xml[insert_pos:]

    self.write(xml)
    self.finish()

class Submit2AppStore(tornado.web.RequestHandler):
  def post(self,app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      app = wkpf.globals.applications[app_ind]
      name = self.get_argument('name', default=None, strip=False)
      static_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "static"))
      appstore_path = os.path.join(static_path,'appstore')
      ## save application to xml
      illegal_chars = list(' "\'\/\\,.;$&=*:|[]')
      xmlname = name
      for char in illegal_chars:
          xmlname = xmlname.replace(char,'')

      count = 0
      xmlpath = os.path.join(appstore_path,xmlname+'.xml')
      while os.path.exists(xmlpath):
        count += 1
        xmlpath = os.path.join(appstore_path,'%s(%s).xml' % (xmlname,count))
      if count > 0:
        xmlname = '%s(%s)' % (xmlname,count)

      #
      # Add the app_name into the xml
      #
      xml = app.xml
      xmlfd = open(xmlpath,'wb')
      insert_pos = xml.find('>')+1
      xml = xml[:insert_pos]+'<app_name>'+name+'</app_name>'+xml[insert_pos:]
      xmlfd.write(xml)
      xmlfd.close()

      #
      # save icon to the same name
      #
      try:
          fileinfo = self.request.files['icon'][0]
          iconpath = os.path.join(appstore_path,xmlname+'.png')
          iconfd = open(iconpath,'wb')
          iconfd.write(fileinfo['body'])
          iconfd.close()
      except KeyError:
          pass

      self.content_type = 'application/json'
      try:
          desc = self.get_argument('desc', default='No description', strip=False)
          author = self.get_argument('author', default='Guest', strip=False)
          csv_path = os.path.join(appstore_path,'application_xmls.js')
          if os.path.exists(csv_path):
             fd = open(csv_path,'ab')
          else:
             fd = open(csv_path,'wb')
          line = '\t'.join([name,xmlname,author,desc])
          fd.write('\n'+line)
          fd.close()
          self.write({'status':0})
      except e:
          ## delete the created xml file of this app
          print 'Error !',e
          os.unlink(xmlpath)
          os.unlink(iconpath)
          self.write({'status':1,'mesg':'%s' % e})
      self.finish()

class RemoveAppFromStore(tornado.web.RequestHandler):
  def post(self):
      nameprefix = self.get_argument('nameprefix', default=None, strip=False)
      static_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "static"))
      appstore_path = os.path.join(static_path,'appstore')

      self.content_type = 'application/json'
      try:
          csv_path = os.path.join(appstore_path,'application_xmls.js')
          if os.path.exists(csv_path):
             fd = open(csv_path,'rb')
             lines = fd.readlines()
             fd.close()

             appname = xmlname = author = desc = None
             for i in range(len(lines)):
                if lines[i].find('\t'+nameprefix+'\t') > 0:
                    appname,xmlname,author,desc = lines[i].split('\t')
                    del lines[i]
                    break

             if appname:
                 ## the last line should not end with new-line
                 ## because the new-line will be added when new app submitted
                 lines[-1] = lines[-1].rstrip()
                 fd = open(csv_path,'wb')
                 fd.write(''.join(lines))
                 fd.close()

                 xmlpath = os.path.join(appstore_path,xmlname+'.xml')
                 if os.path.exists(xmlpath):
                     os.unlink(xmlpath)
                 iconpath = os.path.join(appstore_path,xmlname+'.png')
                 if os.path.exists(iconpath):
                     os.unlink(iconpath)
                 self.write({'status':'0','msg':'ok'})
             else:
                 self.write({'status':'1','msg':name+' Not Found'})
          else:
              self.write({'status':'1','msg':'App List Not Found'})
      except:
          self.write({'status':'1','msg':sys.exc_info()})
      self.finish()

